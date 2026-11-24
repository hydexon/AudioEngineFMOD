#include "AudioSystemImpl_FMOD.h"
#include "AudioSystemImplCVars.h"
#include "ATLEntities_FMOD.h"
#include "FMOD_FileSystemIO.h"
#include "Common_FMOD.h"
#include "ConfigFMOD.h"
#include "IAudioSystem.h"
#include "fmod_errors.h"

#include <AzCore/IO/FileIO.h>
#include <ATLEntityData.h>
#include <AzCore/Debug/Profiler.h>
#include <AzCore/std/algorithm.h>
#include <fmod.hpp>

using namespace Audio;

namespace AudioEngineFMOD
{
#if defined(AZ_DEBUG_BUILD)
    namespace Debug {
        FMOD_RESULT F_CALL AzDebugHook(FMOD_DEBUG_FLAGS flags, const char* file, int line, const char *func, const char* message) {
            if(flags & FMOD_DEBUG_LEVEL_ERROR)
            {
                AZ_Error("FMOD", false, "[%s:%s#%d]: %s", file, func, line, message);
            }
            else if(flags & FMOD_DEBUG_LEVEL_WARNING)
            {
                AZ_Warning("FMOD", false, "[%s:%s#%d]: %s", file, func, line, message);
            }
            else
            {
                AZ_Info("FMOD", "[%s:%s#%d]: %s", file, func, line, message);
            }
            return FMOD_OK;
        }
    }
#endif

    namespace MemCallbacks
    {
        void* F_CALL Malloc(unsigned int size, [[maybe_unused]] FMOD_MEMORY_TYPE type, [[maybe_unused]] const char* srcstr)
        {
            return AZ::AllocatorInstance<Audio::AudioImplAllocator>::Get().Allocate(size, 0, 0, nullptr);
        }

        void* F_CALL Realloc(void* ptr, unsigned int size, [[maybe_unused]] FMOD_MEMORY_TYPE type, [[maybe_unused]] const char* srcstr)
        {
            return AZ::AllocatorInstance<Audio::AudioImplAllocator>::Get().ReAllocate(ptr, size, 0);
        }

        void F_CALL Free(void* ptr, [[maybe_unused]] FMOD_MEMORY_TYPE type, [[maybe_unused]] const char* srcstr)
        {
            return AZ::AllocatorInstance<Audio::AudioImplAllocator>::Get().DeAllocate(ptr);
        }
    }



AudioSystemImpl_FMOD::AudioSystemImpl_FMOD(const char* assetPlatformName)
    : m_studioSystem(nullptr)
    , m_masterBank(nullptr)
    , m_masterStringsBank(nullptr)
    , m_globalAudioObjectId(INVALID_AUDIO_OBJECT_ID)
{
    AudioSystemImplementationRequestBus::Handler::BusConnect();
    AudioSystemImplementationNotificationBus::Handler::BusConnect();

    if(assetPlatformName && assetPlatformName[0] != '\0')
    {
        m_assetPlatform = assetPlatformName;
    }
}

AudioSystemImpl_FMOD::~AudioSystemImpl_FMOD()
{
    AudioSystemImplementationRequestBus::Handler::BusDisconnect();
    AudioSystemImplementationNotificationBus::Handler::BusDisconnect();
}

void AudioSystemImpl_FMOD::Update([[maybe_unused]] float updateIntervalMS) {
    AZ_PROFILE_FUNCTION(Audio);
    m_studioSystem->update();
}

EAudioRequestStatus AudioSystemImpl_FMOD::Initialize() {
    AZ_Info("FMODAudioSystem", "Initializing FMOD Studio...");
    FMOD::Memory_Initialize(
                nullptr,
                0,
                MemCallbacks::Malloc,
                MemCallbacks::Realloc,
                MemCallbacks::Free
                );

#if defined(AZ_DEBUG_BUILD)
    FMOD::Debug_Initialize(
                FMOD_DEBUG_LEVEL_ERROR | FMOD_DEBUG_LEVEL_WARNING | FMOD_DEBUG_LEVEL_LOG,
                FMOD_DEBUG_MODE_CALLBACK, Debug::AzDebugHook);
#endif

    FMOD_RESULT result = FMOD::Studio::System::create(&m_studioSystem);

    if(result != FMOD_OK)
    {
        AZ_Error("FMODAudioSystem", false, "Unable to create FMOD Studio System.");
        return EAudioRequestStatus::Failure;
    }

    FMOD_STUDIO_ADVANCEDSETTINGS studioSettings = { .cbsize = sizeof(FMOD_STUDIO_ADVANCEDSETTINGS) };
    m_studioSystem->getAdvancedSettings(&studioSettings);

    studioSettings.commandqueuesize  = CVars::s_FMODStudio_CommandQueueSize;
    studioSettings.studioupdateperiod = CVars::s_FMODStudio_StudioUpdatePeriod;
    studioSettings.idlesampledatapoolsize = CVars::s_FMODStudio_IdleSampleDataPoolSize;
    studioSettings.streamingscheduledelay = CVars::s_FMODStudio_StreamingScheduleDelay;
    studioSettings.cbsize = sizeof(FMOD_STUDIO_ADVANCEDSETTINGS);

    m_studioSystem->setAdvancedSettings(&studioSettings);

    m_studioSystem->getCoreSystem(&m_lowlvlSystem);

    FMOD_ADVANCEDSETTINGS coreSettings;
    coreSettings.profilePort = CVars::s_FMODCore_ProfilePort;
    coreSettings.cbSize = sizeof(FMOD_ADVANCEDSETTINGS);

    m_lowlvlSystem->setAdvancedSettings(&coreSettings);

    m_lowlvlSystem->setFileSystem(
                SyncIO::AzFileOpen,
                SyncIO::AzFileClose,
                SyncIO::AzFileRead,
                SyncIO::AzFileSeek,
            #if 0
                AsyncIO::AzAsyncFileRead,
                AsyncIO::AzAsyncFileCancel,
            #else
                nullptr,
                nullptr,
            #endif
                -1
                );

    SetBankPaths();

    result = m_studioSystem->initialize(CVars::s_FMODStudio_MaxChannels,
                                      CVars::s_FMODStudio_EnableProfiling ? FMOD_STUDIO_INIT_LIVEUPDATE : FMOD_STUDIO_INIT_NORMAL,
                                      FMOD_INIT_NORMAL,
                                      nullptr);



    if(result != FMOD_OK)
    {
        AZ_Error("FMODAudioSystem", false, "Unable to Initialize FMOD Studio");
        return EAudioRequestStatus::Failure;
    }

    //Load the Master Banks:
    FMOD_RESULT bankResult = m_studioSystem->loadBankFile(Constants::MasterBank, FMOD_STUDIO_LOAD_BANK_NORMAL, &m_masterBank);
    if(bankResult != FMOD_OK)
    {
        AZ_Error("FMODAudioSystem", false, "FMOD Studio Failed to load Master.bank: %s", FMOD_ErrorString(bankResult));
        m_masterBank = nullptr;
        return EAudioRequestStatus::Failure;
    }

    bankResult = m_studioSystem->loadBankFile(Constants::MasterStringsBank, FMOD_STUDIO_LOAD_BANK_NORMAL, &m_masterStringsBank);
    if(bankResult != FMOD_OK)
    {
        AZ_Error("FMODAudioSystem", false, "FMOD Studio Failed to load Master.strings.bank: %s", FMOD_ErrorString(bankResult));
        m_masterStringsBank = nullptr;
        return EAudioRequestStatus::Failure;
    }

    m_studioSystem->setNumListeners(CVars::s_FMOD_MaxListeners); //@TODO: Should be left as a Cvar?

    return EAudioRequestStatus::Success;
}

EAudioRequestStatus AudioSystemImpl_FMOD::ShutDown() {
    m_studioSystem->release();
    m_studioSystem = nullptr;
    return EAudioRequestStatus::Success;
}

EAudioRequestStatus AudioSystemImpl_FMOD::Release() {
    return EAudioRequestStatus::Success;
}

EAudioRequestStatus AudioSystemImpl_FMOD::StopAllSounds() {
    FMOD::Studio::Bus* masterBus = nullptr;
    m_studioSystem->getBus("bus:/", &masterBus);
    if(masterBus)
    {
        masterBus->stopAllEvents(FMOD_STUDIO_STOP_IMMEDIATE);
        return EAudioRequestStatus::Success;
    }

    return EAudioRequestStatus::Success;
}

EAudioRequestStatus AudioSystemImpl_FMOD::RegisterAudioObject(IATLAudioObjectData * objectData, const char *objectName)
{
    SATLAudioObjectData_FMOD* obj = static_cast<SATLAudioObjectData_FMOD*>(objectData);
    if(!obj)
    {
        return EAudioRequestStatus::Failure;
    }

    auto it = std::find(m_registeredAudioObjects.begin(), m_registeredAudioObjects.end(), obj);
    if(it == m_registeredAudioObjects.end())
    {
        //obj->m_name = objectName;
        m_registeredAudioObjects.push_back(obj);
        return EAudioRequestStatus::Success;
    }

    AZ_Error("FMODAudioSystem", false, "Attempt to register an object twice!");
    return EAudioRequestStatus::Failure;
}

EAudioRequestStatus AudioSystemImpl_FMOD::UnregisterAudioObject(IATLAudioObjectData *objectData)
{
    SATLAudioObjectData_FMOD* obj = static_cast<SATLAudioObjectData_FMOD*>(objectData);
    if(!obj)
    {
        return EAudioRequestStatus::Failure;
    }

    auto it = std::find(m_registeredAudioObjects.begin(), m_registeredAudioObjects.end(), obj);
    if(it != m_registeredAudioObjects.end())
    {
        m_registeredAudioObjects.erase(it);
        return EAudioRequestStatus::Success;
    }

    AZ_Error("FMODAudioSystem", false, "Entry not found trying to find object to unregister");
    return EAudioRequestStatus::Failure;
}

EAudioRequestStatus AudioSystemImpl_FMOD::ResetAudioObject(IATLAudioObjectData *objectData)
{
    SATLAudioObjectData_FMOD* obj = static_cast<SATLAudioObjectData_FMOD*>(objectData);
    if(obj)
    {
        //obj->Reset();
        return EAudioRequestStatus::Success;
    }

    return EAudioRequestStatus::Failure;
}

EAudioRequestStatus AudioSystemImpl_FMOD::UpdateAudioObject([[maybe_unused]]IATLAudioObjectData *objectData)
{
    return EAudioRequestStatus::Success;
}

EAudioRequestStatus AudioSystemImpl_FMOD::PrepareTriggerSync([[maybe_unused]] IATLAudioObjectData *audioObjectData, [[maybe_unused]] const IATLTriggerImplData *triggerData) {
    AZ_Info("FMODAudioSystem", "Preparing Trigger!");
    return EAudioRequestStatus::Success;
}

EAudioRequestStatus AudioSystemImpl_FMOD::UnprepareTriggerSync([[maybe_unused]] IATLAudioObjectData *objectData, [[maybe_unused]] const IATLTriggerImplData *triggerData) {

    return EAudioRequestStatus::Success;
}

EAudioRequestStatus AudioSystemImpl_FMOD::PrepareTriggerAsync([[maybe_unused]] IATLAudioObjectData *objectData, [[maybe_unused]] const IATLTriggerImplData *triggerData, [[maybe_unused]] IATLEventData *eventData) {
    AZ_Info("FMODAudioSystem", "Preparing Trigger (Async)!");
    return EAudioRequestStatus::Success;
}

EAudioRequestStatus AudioSystemImpl_FMOD::UnprepareTriggerAsync([[maybe_unused]] IATLAudioObjectData *pAudioObjectData, [[maybe_unused]] const IATLTriggerImplData *pTriggerData, [[maybe_unused]] IATLEventData *pEventData) {
    return EAudioRequestStatus::Success;
}

EAudioRequestStatus AudioSystemImpl_FMOD::ActivateTrigger(IATLAudioObjectData *objectData, const IATLTriggerImplData *triggerData, IATLEventData *eventData, const SATLSourceData *sourceData) {
    auto result = EAudioRequestStatus::Success;


    return result;
}

EAudioRequestStatus AudioSystemImpl_FMOD::StopEvent(IATLAudioObjectData *objectData, const IATLEventData *eventData)
{
    EAudioRequestStatus result = EAudioRequestStatus::Failure;
    auto implEvtData = static_cast<const SATLEventData_FMOD*>(eventData);
    if(implEvtData)
    {
        /*
        FMOD_RESULT res = implEvtData->m_evtInstance->stop(FMOD_STUDIO_STOP_IMMEDIATE);
        if(res == FMOD_OK)
        {
            result = EAudioRequestStatus::Success;
        }*/
    }
    return result;
}

EAudioRequestStatus AudioSystemImpl_FMOD::StopAllEvents(IATLAudioObjectData *objectData)
{
    SATLAudioObjectData_FMOD* obj = static_cast<SATLAudioObjectData_FMOD*>(objectData);
    if(!obj)
    {
        return EAudioRequestStatus::Failure;
    }

    //obj->StopAllEvents();
    return EAudioRequestStatus::Success;
}

EAudioRequestStatus AudioSystemImpl_FMOD::SetPosition(IATLAudioObjectData *objectData, const SATLWorldPosition &worldPosition) {
    auto* fmodObj = static_cast<SATLAudioObjectData_FMOD*>(objectData);
    if(!fmodObj)
    {
        return EAudioRequestStatus::FailureInvalidObjectId;
    }

    //fmodObj->Set3DAttributes(worldPosition);
    return EAudioRequestStatus::Success;
}

EAudioRequestStatus AudioSystemImpl_FMOD::SetMultiplePositions(IATLAudioObjectData *, const MultiPositionParams &) {
    AZ_Error("FMODAudioSystem", false, "Set Multiple Positions in FMOD are not supported!");
    return EAudioRequestStatus::FailureInvalidRequest;
}

EAudioRequestStatus AudioSystemImpl_FMOD::SetRtpc(IATLAudioObjectData *objectData, const IATLRtpcImplData *rtpcData, float value) {
    return EAudioRequestStatus::Failure;
}

EAudioRequestStatus AudioSystemImpl_FMOD::SetSwitchState(IATLAudioObjectData *objectData, const IATLSwitchStateImplData *switchStateData) {
    EAudioRequestStatus result = EAudioRequestStatus::Failure;

    auto* fmodObj = static_cast<SATLAudioObjectData_FMOD*>(objectData);
    const auto* fmodSwitchState = static_cast<const SATLSwitchStateImplData_FMOD*>(switchStateData);

    if(fmodObj && fmodSwitchState)
    {
    }

    return result;
}

EAudioRequestStatus AudioSystemImpl_FMOD::SetObstructionOcclusion(IATLAudioObjectData *objectData, float obstruction, float occlusion) {
    auto fmodObj = static_cast<SATLAudioObjectData_FMOD*>(objectData);

    if(fmodObj)
    {
    }

    return EAudioRequestStatus::Success;
}

EAudioRequestStatus AudioSystemImpl_FMOD::SetEnvironment(IATLAudioObjectData *objectData, const IATLEnvironmentImplData *environmentData, float amount) {

    auto fmodObj = static_cast<SATLAudioObjectData_FMOD*>(objectData);
    const auto fmodEnv = static_cast<const SATLEnvironmentImplData_FMOD*>(environmentData);

    if(fmodObj && fmodEnv)
    {
    }
    return EAudioRequestStatus::Failure;
}

EAudioRequestStatus AudioSystemImpl_FMOD::SetListenerPosition(IATLListenerData *listenerData, const SATLWorldPosition &newPosition) {
    auto ldata = static_cast<SATLListenerData_FMOD*>(listenerData);
    if(!ldata)
    {
        return Audio::EAudioRequestStatus::Failure;
    }

    ldata->m_attributes = Utils::CreateFMOD3DAttributes(newPosition.GetPositionVec(), newPosition.GetForwardVec(), newPosition.GetUpVec());
    FMOD_RESULT res = m_studioSystem->setListenerAttributes(ldata->m_id, &ldata->m_attributes);

    return (res == FMOD_OK) ? EAudioRequestStatus::Success : EAudioRequestStatus::Failure;
}

EAudioRequestStatus AudioSystemImpl_FMOD::ResetRtpc(IATLAudioObjectData *objectData, const IATLRtpcImplData *rtpcData) {

    auto fmodObj  = static_cast<SATLAudioObjectData_FMOD*>(objectData);
    auto fmodRtpc = static_cast<const SATLParameterRtpc_FMOD*>(rtpcData);

    if(fmodObj && fmodRtpc)
    {
    }

    return EAudioRequestStatus::Success;
}

EAudioRequestStatus AudioSystemImpl_FMOD::RegisterInMemoryFile(SATLAudioFileEntryInfo *audioFileEntry) {

    EAudioRequestStatus result = EAudioRequestStatus::Success;
    if(audioFileEntry)
    {
        auto const implFileEntryData = static_cast<SATLAudioFileEntryData_FMOD*>(audioFileEntry->pImplData);

        if(implFileEntryData)
        {
            FMOD_RESULT bankResult = m_studioSystem->loadBankMemory(static_cast<const char*>(audioFileEntry->pFileData),
                                                                    aznumeric_cast<int>(audioFileEntry->nSize),
                                                                    FMOD_STUDIO_LOAD_MEMORY_POINT, //Match Wwise's AK::SoundEngine::LoadBankMemoryView behavior.
                                                                    FMOD_STUDIO_LOAD_BANK_NORMAL,
                                                                    &implFileEntryData->m_FMODBank);

            if(bankResult != FMOD_OK)
            {
                AZ_Warning("FMODAudioSystem", false,
                           "Failed to load bank from memory: %s, FMOD Error: %s",
                           audioFileEntry->sFileName, FMOD_ErrorString(bankResult));

                implFileEntryData->m_FMODBank = nullptr;
                result = EAudioRequestStatus::Failure;
            }
            else
            {
                //Cache event parameters:
                int evtCount = 0;
                implFileEntryData->m_FMODBank->getEventCount(&evtCount);
                if(evtCount > 0)
                {
                    std::vector<FMOD::Studio::EventDescription*> events(evtCount);
                    implFileEntryData->m_FMODBank->getEventList(events.data(), evtCount, &evtCount);

                    for(auto evtDesc : events)
                    {
                        int paramCount = 0;
                        evtDesc->getParameterDescriptionCount(&paramCount);

                        for(int i = 0; i < paramCount; i++)
                        {
                            FMOD_STUDIO_PARAMETER_DESCRIPTION paramDesc;
                            if(evtDesc->getParameterDescriptionByIndex(i, &paramDesc) == FMOD_OK)
                            {
                                m_evtParametersCache.emplace(paramDesc.name, ParameterInfo{ paramDesc.id, paramDesc.defaultvalue });
                            }
                        }
                    }
                }
            }
        }
    }
    return result;
}

EAudioRequestStatus AudioSystemImpl_FMOD::UnregisterInMemoryFile(SATLAudioFileEntryInfo *audioFileEntry) {

    EAudioRequestStatus result = EAudioRequestStatus::Success;

    if(audioFileEntry)
    {
        if(auto const implFileEntryData = static_cast<SATLAudioFileEntryData_FMOD*>(audioFileEntry->pImplData))
        {
            //Remove the cache entries from the parameters cache before unloading the bank.
            int evtCount = 0;
            implFileEntryData->m_FMODBank->getEventCount(&evtCount);
            if(evtCount > 0)
            {
                std::vector<FMOD::Studio::EventDescription*> events(evtCount);
                implFileEntryData->m_FMODBank->getEventList(events.data(), evtCount, &evtCount);
                for(auto evtDesc : events)
                {
                    int paramCount = 0;
                    evtDesc->getParameterDescriptionCount(&paramCount);

                    FMOD_STUDIO_PARAMETER_DESCRIPTION paramDesc;
                    for(int i = 0; i < paramCount; i++)
                    {
                        if(evtDesc->getParameterDescriptionByIndex(i, &paramDesc) == FMOD_OK)
                        {
                            auto entry = m_evtParametersCache.find(paramDesc.name);
                            if(entry != m_evtParametersCache.end())
                            {
                                m_evtParametersCache.erase(entry);
                            }
                        }
                    }
                }
            }
            if(FMOD_RESULT bankResult = implFileEntryData->m_FMODBank->unload(); bankResult != FMOD_OK)
            {
                AZ_Warning("FMODAudioSystem", false, "FMOD::Studio::Bank::Unload returned %s", FMOD_ErrorString(bankResult));
                result = EAudioRequestStatus::Failure;
            }


        }
    }

    return result;
}

EAudioRequestStatus AudioSystemImpl_FMOD::ParseAudioFileEntry(const AZ::rapidxml::xml_node<char> *audioFileEntryNode, SATLAudioFileEntryInfo *fileEntryInfo) {

    EAudioRequestStatus result = EAudioRequestStatus::Failure;
    if(audioFileEntryNode && azstricmp(audioFileEntryNode->name(), XMLTags::BankTag) == 0 && fileEntryInfo)
    {
        const char* audioFileEntryName = nullptr;
        if(auto bankPathAttr = audioFileEntryNode->first_attribute(XMLTags::NameAttr, 0, false))
        {
            audioFileEntryName = bankPathAttr->value();
        }

        bool isLocalized = false;
        if(auto localizedAttr = audioFileEntryNode->first_attribute(XMLTags::LocalizedAttr, 0, false))
        {
            if(azstricmp(localizedAttr->value(), "true") == 0)
            {
                isLocalized = true;
            }
        }

        if(audioFileEntryName && audioFileEntryName[0] != '\0')
        {
            AZStd::string bankBaseName = audioFileEntryName;
            AZ::StringFunc::Path::ReplaceExtension(bankBaseName, "");

            fileEntryInfo->bLocalized = isLocalized;
            fileEntryInfo->nMemoryBlockAlignment = FMOD_STUDIO_LOAD_MEMORY_ALIGNMENT;
            fileEntryInfo->sFileName = audioFileEntryName;
            fileEntryInfo->pImplData = azcreate(SATLAudioFileEntryData_FMOD, (bankBaseName.c_str()), Audio::AudioImplAllocator);
            result = EAudioRequestStatus::Success;
        }
        else
        {
            fileEntryInfo->sFileName = nullptr;
            fileEntryInfo->nMemoryBlockAlignment = 0;
            fileEntryInfo->pImplData = nullptr;
        }
    }
    return result;
}

void AudioSystemImpl_FMOD::DeleteAudioFileEntryData(IATLAudioFileEntryData *oldAudioFileEntryData)
{
    azdestroy(oldAudioFileEntryData, Audio::AudioImplAllocator, SATLAudioFileEntryData_FMOD);
}

const char * const AudioSystemImpl_FMOD::GetAudioFileLocation(SATLAudioFileEntryInfo *fileEntryInfo)
{

    if(fileEntryInfo)
    {
        return fileEntryInfo->bLocalized ? m_localizedFMODBankPath.c_str() : m_fmodBankPath.c_str();
    }

    return nullptr;
}

AZStd::string GetATLControlName(const AZ::rapidxml::xml_node<char>* node)
{
    //Get the ATL Control Trigger name for debug purposes:
    AZStd::string atlName;
    if(node->parent())
    {
        const char* parentName = node->parent()->name();
        if(azstricmp(parentName, "ATLTrigger") == 0)
        {
            auto atlnameAttr = node->parent()->first_attribute("atl_name");
            if(atlnameAttr)
            {
                atlName = atlnameAttr->value();
            }
        }
    }

    return atlName;
}

IATLTriggerImplData *AudioSystemImpl_FMOD::NewAudioTriggerImplData(const AZ::rapidxml::xml_node<char> *audioTriggerNode)
{
    SATLTriggerImplData_FMOD* newTriggerImpl = nullptr;
    if(!audioTriggerNode)
    {
        return nullptr;
    }

    const char* nodeName = audioTriggerNode->name();
    if(AZ::StringFunc::Equal(nodeName, XMLTags::EventTag))
    {
        auto pathAttr = audioTriggerNode->first_attribute(XMLTags::PathAttr);
        if(pathAttr)
        {
            const char* path = pathAttr->value();
            FMOD_GUID guid = { 0 };
            if(m_studioSystem->lookupID(path, &guid) == FMOD_OK)
            {
                auto eventType = FMODEventType::Play;
                auto evtTypeAttr = audioTriggerNode->first_attribute(XMLTags::EventTypeAttr);
                if(evtTypeAttr)
                {
                    eventType = Utils::GetActionFromXmlStr(evtTypeAttr->value());
                }
                newTriggerImpl = azcreate(SATLTriggerImplData_FMOD, (AudioStringToID<Audio::TAudioTriggerImplID>(path), eventType, nullptr, guid), Audio::AudioImplAllocator);
            }
        }
    }
    else if(AZ::StringFunc::Equal(nodeName, XMLTags::SnapshotTag))
    {
        auto pathAttr = audioTriggerNode->first_attribute(XMLTags::PathAttr);
        if(pathAttr)
        {
            const char* path = pathAttr->value();
            FMOD_GUID guid = {0};
            if(m_studioSystem->lookupID(path, &guid) == FMOD_OK)
            {
                FMOD::Studio::EventDescription* evtDescription = nullptr;
                m_studioSystem->getEventByID(&guid, &evtDescription);
                newTriggerImpl = azcreate(SATLTriggerImplData_FMOD, (AudioStringToID<Audio::TAudioTriggerImplID>(path), FMODEventType::Play, evtDescription, guid), Audio::AudioImplAllocator);
            }
        }
    }

    return newTriggerImpl;
}

void AudioSystemImpl_FMOD::DeleteAudioTriggerImplData(IATLTriggerImplData *oldTriggerData)
{
    azdestroy(oldTriggerData, Audio::AudioImplAllocator, SATLTriggerImplData_FMOD);
}

IATLRtpcImplData *AudioSystemImpl_FMOD::NewAudioRtpcImplData(const AZ::rapidxml::xml_node<char> *audioRtpcNode)
{
    return nullptr;
}

void AudioSystemImpl_FMOD::DeleteAudioRtpcImplData(IATLRtpcImplData *oldRtpcData) {
    azdestroy(oldRtpcData, Audio::AudioImplAllocator, SATLParameterRtpc_FMOD);
}

IATLSwitchStateImplData *AudioSystemImpl_FMOD::NewAudioSwitchStateImplData(const AZ::rapidxml::xml_node<char> *audioSwitchStateNode)
{

    return nullptr;

}

void AudioSystemImpl_FMOD::DeleteAudioSwitchStateImplData([[maybe_unused]] IATLSwitchStateImplData *oldAudioSwitchStateData) {
    azdestroy(oldAudioSwitchStateData, Audio::AudioImplAllocator, SATLSwitchStateImplData_FMOD);
}

IATLEnvironmentImplData *AudioSystemImpl_FMOD::NewAudioEnvironmentImplData(const AZ::rapidxml::xml_node<char> *audioEnvironmentNode) {

    return nullptr;
}

void AudioSystemImpl_FMOD::DeleteAudioEnvironmentImplData(IATLEnvironmentImplData *oldEnvironmentData) {
    azdestroy(oldEnvironmentData, Audio::AudioImplAllocator, SATLEnvironmentImplData_FMOD);
}

IATLAudioObjectData *AudioSystemImpl_FMOD::NewGlobalAudioObjectData(TAudioObjectID objectId)
{
    auto obj = azcreate(SATLAudioObjectData_FMOD, (), Audio::AudioImplAllocator);
    m_globalAudioObjectId = objectId;
    return obj;
}

IATLAudioObjectData *AudioSystemImpl_FMOD::NewAudioObjectData(TAudioObjectID objectId)
{
    return azcreate(SATLAudioObjectData_FMOD, (), Audio::AudioImplAllocator);
}

void AudioSystemImpl_FMOD::DeleteAudioObjectData(IATLAudioObjectData *oldObjectData) {

    StopAllAndClearInstancesFromAudioObject(oldObjectData);
    azdestroy(oldObjectData, Audio::AudioImplAllocator, SATLAudioObjectData_FMOD);
}

IATLListenerData *AudioSystemImpl_FMOD::NewDefaultAudioListenerObjectData(TATLIDType objectId) {
    return azcreate(SATLListenerData_FMOD, (0), Audio::AudioImplAllocator);
}

IATLListenerData *AudioSystemImpl_FMOD::NewAudioListenerObjectData(TATLIDType objectId) {
    static int id = 0;
    return azcreate(SATLListenerData_FMOD, (++id), Audio::AudioImplAllocator);
}

void AudioSystemImpl_FMOD::DeleteAudioListenerObjectData(IATLListenerData *oldListenerData) {
    auto listener = static_cast<SATLListenerData_FMOD*>(oldListenerData);
    azdestroy(listener, Audio::AudioImplAllocator, SATLListenerData_FMOD);
}

IATLEventData *AudioSystemImpl_FMOD::NewAudioEventData(TAudioEventID eventID) {
    auto audioEventData = azcreate(SATLEventData_FMOD, (), Audio::AudioImplAllocator);
    return audioEventData;
}

void AudioSystemImpl_FMOD::DeleteAudioEventData(IATLEventData *oldEventData) {
    azdestroy(oldEventData, Audio::AudioImplAllocator, SATLAudioObjectData_FMOD);
}

void AudioSystemImpl_FMOD::ResetAudioEventData(IATLEventData *eventData) {
    auto event = static_cast<SATLEventData_FMOD*>(eventData);
    if(event)
    {
    }
}

void AudioSystemImpl_FMOD::SetLanguage(const char *language) {
    if(!language)
    {
        return;
    }

    AZStd::string languageSubFolder(language);
    languageSubFolder += "/";

    m_localizedFMODBankPath = m_fmodBankPath;
    m_localizedFMODBankPath.append(languageSubFolder);
}

const char * const AudioSystemImpl_FMOD::GetImplSubPath() const {
    return "fmod/";
}

const char * const AudioSystemImpl_FMOD::GetImplementationNameString() const {
    return "FMODStudio";

}

void AudioSystemImpl_FMOD::GetMemoryInfo(SAudioImplMemoryInfo &memoryInfo) const {
    memoryInfo.nPrimaryPoolSize = AZ::AllocatorInstance<Audio::AudioImplAllocator>::Get().Capacity();
    memoryInfo.nPrimaryPoolUsedSize =
            memoryInfo.nPrimaryPoolSize - AZ::AllocatorInstance<Audio::AudioImplAllocator>::Get().NumAllocatedBytes();
    memoryInfo.nPrimaryPoolAllocations = 0;
    memoryInfo.nSecondaryPoolSize = 0;
    memoryInfo.nSecondaryPoolUsedSize = 0;
    memoryInfo.nSecondaryPoolAllocations = 0;
}

AZStd::vector<AudioImplMemoryPoolInfo> AudioSystemImpl_FMOD::GetMemoryPoolInfo() {
    return AZStd::vector<AudioImplMemoryPoolInfo>();
}

bool AudioSystemImpl_FMOD::CreateAudioSource(const SAudioInputConfig &sourceConfig) {
    AZ_UNUSED(sourceConfig);
    return false;
}

void AudioSystemImpl_FMOD::DestroyAudioSource(TAudioSourceId sourceId) {
    AZ_UNUSED(sourceId);
}

void AudioSystemImpl_FMOD::SetPanningMode(PanningMode) {
    // Not Implemented.
}

void AudioSystemImpl_FMOD::OnAudioSystemLoseFocus() {
    if(CVars::s_FMODStudio_PauseAudioOnFocusLost)
    {
        FMOD::System* sys = nullptr;
        m_studioSystem->getCoreSystem(&sys);
        if(sys)
        {
            sys->mixerSuspend();
        }
    }
}

void AudioSystemImpl_FMOD::OnAudioSystemGetFocus() {
    if(CVars::s_FMODStudio_PauseAudioOnFocusLost)
    {
        FMOD::System* sys = nullptr;
        m_studioSystem->getCoreSystem(&sys);
        if(sys)
        {
            sys->mixerResume();
        }
    }
}

void AudioSystemImpl_FMOD::OnAudioSystemMuteAll() {
    // FMOD has no direct way to mute everything, you might achieve this with events instead.
    // The only hack i might know is get every bus from every loaded bank, and retrieve every bus.
    // iterate each one and call FMOD::Studio::Bus::Mute() then Unmute().
    // This was proposed as a solution by an Firelight FMOD engineer!:
    // https://qa.fmod.com/t/how-to-make-an-mute-unmute-button/18840/5

    MuteFMODMasterBus(true);
}

void AudioSystemImpl_FMOD::OnAudioSystemUnmuteAll() {
    //See comments from OnAudioSystemMuteAll().
    MuteFMODMasterBus(false);
}

void AudioSystemImpl_FMOD::OnAudioSystemRefresh() {

    //Unload the Banks.
    if(m_masterBank)
    {
        FMOD_RESULT r = m_masterBank->unload();
        if(r != FMOD_OK)
        {
            AZ_Error("FMODAudioSystem", false, "Error trying to unload Master.bank for AudioSystemRefresh!: %s", FMOD_ErrorString(r));
        }
    }

    if(m_masterStringsBank)
    {
        FMOD_RESULT r = m_masterStringsBank->unload();
        if(r != FMOD_OK)
        {
            AZ_Error("FMODAudioSystem", false, "Error trying to unload Master.strings.bank for AudioSystemRefresh!: %s", FMOD_ErrorString(r));
        }
    }

    //Re-load the banks again.

    //Load the Master Banks:
    FMOD_RESULT bankResult = m_studioSystem->loadBankFile(Constants::MasterBank, FMOD_STUDIO_LOAD_BANK_NORMAL, &m_masterBank);
    if(bankResult != FMOD_OK)
    {
        AZ_Error("FMODAudioSystem", false, "FMOD Studio Failed to load Master.bank: %s", FMOD_ErrorString(bankResult));
        m_masterBank = nullptr;
    }

    bankResult = m_studioSystem->loadBankFile(Constants::MasterStringsBank, FMOD_STUDIO_LOAD_BANK_NORMAL, &m_masterStringsBank);
    if(bankResult != FMOD_OK)
    {
        AZ_Error("FMODAudioSystem", false, "FMOD Studio Failed to load Master.strings.bank: %s", FMOD_ErrorString(bankResult));
        m_masterStringsBank = nullptr;
    }
}

void AudioSystemImpl_FMOD::SetBankPaths()
{
    //Default:
    // "Assets/Audio/FMOD/Banks"
    AZStd::string bankPath = Constants::DefaultFMODBanksPath;
    // --                                 FMOD Banks Folder
    // assetPlatform -> windows   --|
    //                  linux     --|---- Desktop
    //                  mac       --|
    //
    //                  Android   --|
    //                              |---- Mobile
    //                  iOS/tvOS  --|
    //
    //                  (And soon consoles)

    AZStd::string fmodPlatform = "";
    if(AZ::StringFunc::Equal(m_assetPlatform, "pc") ||
            AZ::StringFunc::Equal(m_assetPlatform, "linux") ||
            AZ::StringFunc::Equal(m_assetPlatform, "mac"))
    {
        fmodPlatform = "Desktop";
    }

    if(AZ::StringFunc::Equal(m_assetPlatform, "ios") ||
            AZ::StringFunc::Equal(m_assetPlatform, "android"))
    {
        fmodPlatform = "Mobile";
    }

    AZ_Error("FMODAudioSystem", !fmodPlatform.empty(), "Unable to determine platform (Desktop or Mobile) for FMOD Bank directory."); //NDA Consoles or niche platform?

    AZStd::string platformPath;

    //Assets/Audio/FMOD/Banks/Desktop
    AZ::StringFunc::AssetDatabasePath::Join(bankPath.c_str(), fmodPlatform.c_str(), platformPath);
    //Some validation:
    AZStd::string masterBankPath;
    AZ::StringFunc::AssetDatabasePath::Join(platformPath.c_str(), Constants::MasterBank, masterBankPath);
    if(AZ::IO::FileIOBase::GetInstance()->Exists(masterBankPath.c_str()))
    {
        if(!platformPath.ends_with(AZ_CORRECT_DATABASE_SEPARATOR))
        {
            platformPath.push_back(AZ_CORRECT_DATABASE_SEPARATOR);
        }
        bankPath = AZStd::move(platformPath);
    }

    m_fmodBankPath = bankPath;
    m_localizedFMODBankPath = bankPath;

    SetBanksRootPath(m_fmodBankPath);
}

EAudioRequestStatus AudioSystemImpl_FMOD::MuteFMODMasterBus(bool mute)
{
    FMOD::Studio::Bus* mBus = nullptr;
    FMOD_RESULT res = m_studioSystem->getBus("bus:/", &mBus);

    if(mBus)
    {
        mBus->setMute(mute);
    }

    return (res == FMOD_OK) ? EAudioRequestStatus::Success : EAudioRequestStatus::Failure;
}

void AudioSystemImpl_FMOD::StopAllAndClearInstancesFromAudioObject(Audio::IATLAudioObjectData *sndObj)
{
}

void AudioSystemImpl_FMOD::ClearStoppedEventInstances(Audio::IATLAudioObjectData *sndObj)
{

}


}