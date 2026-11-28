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
    FMOD_RESULT F_CALL FMODEventCallback(FMOD_STUDIO_EVENT_CALLBACK_TYPE type, FMOD_STUDIO_EVENTINSTANCE* event, void* params)
    {
        FMOD::Studio::EventInstance* eventInst = reinterpret_cast<FMOD::Studio::EventInstance*>(event);
        if (eventInst)
        {
            ATLEventData_FMOD* eventData = nullptr;
            eventInst->getUserData(reinterpret_cast<void**>(&eventData));

            if (eventData != nullptr)
            {
                Audio::CallbackRequest::ReportFinishedEvent repFinishedEvt;
                repFinishedEvt.m_eventId = eventData->GetId();
                repFinishedEvt.m_status = EAudioRequestStatus::Success;
                AZ::Interface<IAudioSystem>::Get()->PushRequest(AZStd::move(repFinishedEvt));
            }
        }
        return FMOD_OK;
    }

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

    m_registeredAudioObjects.reserve(256);
}

AudioSystemImpl_FMOD::~AudioSystemImpl_FMOD()
{
    AudioSystemImplementationRequestBus::Handler::BusDisconnect();
    AudioSystemImplementationNotificationBus::Handler::BusDisconnect();
}

void AudioSystemImpl_FMOD::Update([[maybe_unused]] float updateIntervalMS) {
    AZ_PROFILE_FUNCTION(Audio);
    m_studioSystem->update();

    if(!m_pendingEvents.empty())
    {
        auto it = m_pendingEvents.begin();
        auto end = m_pendingEvents.end();

        while( it != end)
        {
            if((*it)->GetATLAudioObject()->PostAudioEvent(*it))
            {
                it = m_pendingEvents.erase(it);
                end = m_pendingEvents.end();
                continue;
            }
            ++it;
        }
    }
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

    m_studioSystem->getCoreSystem(&m_lowlvlSystem);
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
                                      FMOD_STUDIO_INIT_NORMAL,
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

    m_studioSystem->setNumListeners(FMOD_MAX_LISTENERS); //@TODO: Should be left as a Cvar?
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
    ATLAudioObject_FMOD* obj = static_cast<ATLAudioObject_FMOD*>(objectData);
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
    ATLAudioObject_FMOD* obj = static_cast<ATLAudioObject_FMOD*>(objectData);
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
    ATLAudioObject_FMOD* obj = static_cast<ATLAudioObject_FMOD*>(objectData);
    if(obj)
    {
        obj->Reset();
        return EAudioRequestStatus::Success;
    }

    return EAudioRequestStatus::Failure;
}

EAudioRequestStatus AudioSystemImpl_FMOD::UpdateAudioObject([[maybe_unused]]IATLAudioObjectData *objectData)
{
    return EAudioRequestStatus::Success;
}

EAudioRequestStatus AudioSystemImpl_FMOD::PrepareTriggerSync([[maybe_unused]] IATLAudioObjectData *audioObjectData, [[maybe_unused]] const IATLTriggerImplData *triggerData) {
    return EAudioRequestStatus::Success;
}

EAudioRequestStatus AudioSystemImpl_FMOD::UnprepareTriggerSync([[maybe_unused]] IATLAudioObjectData *objectData, [[maybe_unused]] const IATLTriggerImplData *triggerData) {

    return EAudioRequestStatus::Success;
}

EAudioRequestStatus AudioSystemImpl_FMOD::PrepareTriggerAsync([[maybe_unused]] IATLAudioObjectData *objectData, [[maybe_unused]] const IATLTriggerImplData *triggerData, [[maybe_unused]] IATLEventData *eventData) {
    return EAudioRequestStatus::Success;
}

EAudioRequestStatus AudioSystemImpl_FMOD::UnprepareTriggerAsync([[maybe_unused]] IATLAudioObjectData *pAudioObjectData, [[maybe_unused]] const IATLTriggerImplData *pTriggerData, [[maybe_unused]] IATLEventData *pEventData) {
    return EAudioRequestStatus::Success;
}

EAudioRequestStatus AudioSystemImpl_FMOD::ActivateTrigger(IATLAudioObjectData *objectData, const IATLTriggerImplData *triggerData, IATLEventData *eventData, const SATLSourceData *sourceData)
{
    auto result = EAudioRequestStatus::Failure;
    auto obj = static_cast<ATLAudioObject_FMOD*>(objectData);
    auto trigger = static_cast<const ATLTrigger_FMOD*>(triggerData);
    auto event = static_cast<ATLEventData_FMOD*>(eventData);

    if (obj && trigger && event)
    {
        if (trigger->m_type == FMODEventType::Play)
        {
            FMOD::Studio::EventDescription* description = trigger->m_eventDescription;
            if (description == nullptr)
            {
                if(g_PathIdToEventDescription.find(trigger->m_id) != g_PathIdToEventDescription.end())
                {
                    description = g_PathIdToEventDescription[trigger->m_id];
                }
                else
                {
                    FMOD_RESULT fmod_result = FMOD_ERR_UNINITIALIZED;
                    fmod_result = m_studioSystem->getEventByID(&trigger->m_guid, &description);
                    AZ_Assert(fmod_result == FMOD_OK, "Error trying to retrieve getEventID while activating trigger.");
                    g_PathIdToEventDescription.emplace(trigger->m_id, description);
                }

            }
            if (description != nullptr)
            {
                FMOD::Studio::EventInstance* instance = nullptr;
                description->createInstance(&instance);
                event->SetInstance(instance);

                FMOD_STUDIO_PARAMETER_DESCRIPTION desc;
                FMOD_RESULT paramRes = description->getParameterDescriptionByName("occlusion", &desc);
                if (paramRes == FMOD_OK)
                {
                    event->SetOcclusionParameterID(desc.id);
                }

                event->GetInstance()->setCallback(FMODEventCallback, FMOD_STUDIO_EVENT_CALLBACK_START_FAILED | FMOD_STUDIO_EVENT_CALLBACK_STOPPED);
                event->GetInstance()->setUserData(event);
                event->GetInstance()->set3DAttributes(&obj->Get3DAttributes());
                event->SetEventPathId(trigger->m_id);
                event->SetATLAudioObject(obj);

                auto match = AZStd::find(m_pendingEvents.begin(), m_pendingEvents.end(), event);
                if(match == m_pendingEvents.end())  {
                    m_pendingEvents.push_back(event);
                }

                result = EAudioRequestStatus::Success;
            }
        }
        else
        {

            obj->StopPlayingEvent(trigger->m_id);
            result = EAudioRequestStatus::Failure;
        }
    }
    return result;
}

EAudioRequestStatus AudioSystemImpl_FMOD::StopEvent(IATLAudioObjectData *objectData, const IATLEventData *eventData)
{
    EAudioRequestStatus result = EAudioRequestStatus::Failure;
    auto implEvtData = static_cast<const ATLEventData_FMOD*>(eventData);
    auto fmodObj = static_cast<ATLAudioObject_FMOD*>(objectData);
    if(implEvtData && fmodObj)
    {
        fmodObj->StopPlayingEvent(implEvtData->GetEventPathId());
        result = EAudioRequestStatus::Success;
    }
    return result;
}

EAudioRequestStatus AudioSystemImpl_FMOD::StopAllEvents(IATLAudioObjectData *objectData)
{
    ATLAudioObject_FMOD* obj = static_cast<ATLAudioObject_FMOD*>(objectData);
    if(!obj)
    {
        return EAudioRequestStatus::Failure;
    }

    obj->StopAllEvents();
    return EAudioRequestStatus::Success;
}

EAudioRequestStatus AudioSystemImpl_FMOD::SetPosition(IATLAudioObjectData *objectData, const SATLWorldPosition &worldPosition) {
    auto* fmodObj = static_cast<ATLAudioObject_FMOD*>(objectData);
    if(!fmodObj)
    {
        return EAudioRequestStatus::FailureInvalidObjectId;
    }

    fmodObj->Set3DAttributes(worldPosition);
    return EAudioRequestStatus::Success;
}

EAudioRequestStatus AudioSystemImpl_FMOD::SetMultiplePositions(IATLAudioObjectData *, const MultiPositionParams &) {
    AZ_Error("FMODAudioSystem", false, "Set Multiple Positions in FMOD are not supported!");
    return EAudioRequestStatus::FailureInvalidRequest;
}

EAudioRequestStatus AudioSystemImpl_FMOD::SetRtpc(IATLAudioObjectData *objectData, const IATLRtpcImplData *rtpcData, float value)
{
    AZ_PROFILE_FUNCTION(Audio);

    EAudioRequestStatus result = EAudioRequestStatus::Failure;
    auto obj  = static_cast<ATLAudioObject_FMOD*>(objectData);
    auto rtpc = static_cast<const ATLParameterRtpc_FMOD*>(rtpcData);

    if(rtpc)
    {
        if(rtpc->m_isGlobalParameter)
        {
            float calcValue = rtpc->m_mult * value + rtpc->m_shift;
            if(m_studioSystem->setParameterByID(rtpc->m_paramId, calcValue))
            {
                result = EAudioRequestStatus::Success;
            }
            else
            {
                AZ_Warning("FMODAudioSystem", false, "Unable to set global parameter!");
            }
        }
        else
        {
            if(obj)
            {
                if(rtpc->m_validParamId)
                {
                    obj->PostParameter(rtpc, value);
                    result = EAudioRequestStatus::Success;
                }
                else
                {
                    FMOD::Studio::EventDescription* desc = nullptr;
                    FMOD_STUDIO_PARAMETER_DESCRIPTION paramDesc;
                    if(auto it = g_PathIdToEventDescription.find(rtpc->m_targetId); it != g_PathIdToEventDescription.end())
                    {
                        desc = it->second;
                    }
                    else
                    {
                        AZ_Info("FMODAudioSystem", "We can't find event description from cache, we have to lookup manually.");
                        FMOD_GUID guid = {0};
                        if(m_studioSystem->lookupID(rtpc->m_targetPath.c_str(), &guid) == FMOD_OK)
                        {
                             if(m_studioSystem->getEventByID(&guid, &desc) == FMOD_OK)
                             {
                                 g_PathIdToEventDescription.emplace(rtpc->m_targetId, desc);
                             }
                        }
                    }
                    if(desc)
                    {
                        desc->getParameterDescriptionByName(rtpc->m_paramName.c_str(), &paramDesc);
                        rtpc->m_paramId = paramDesc.id;
                        rtpc->m_defaultValue = paramDesc.defaultvalue;
                        rtpc->m_validParamId = true;
                        obj->PostParameter(rtpc, value);
                        result = EAudioRequestStatus::Success;
                    }
                }
            }
        }
    }

    return result;
}

EAudioRequestStatus AudioSystemImpl_FMOD::SetSwitchState(IATLAudioObjectData *objectData, const IATLSwitchStateImplData *switchStateData)
{
    EAudioRequestStatus result = EAudioRequestStatus::Failure;

    auto* fmodObj = static_cast<ATLAudioObject_FMOD*>(objectData);
    const auto* fmodState = static_cast<const ATLSwitchState_FMOD*>(switchStateData);

    if(fmodState)
    {
        if(fmodState->m_isGlobal)
        {
            if(m_studioSystem->setParameterByID(fmodState->m_paramId, fmodState->m_value) == FMOD_OK)
            {
                result = EAudioRequestStatus::Success;
            }
            else
            {
                AZ_Warning("FMODAudioSystem", false, "Unable to set global parameter for switch!");
            }
        }
        else
        {
            if(fmodObj)
            {
                if(fmodState->m_paramIdValid)
                {
                    fmodObj->SetSwitch(fmodState);
                    result = EAudioRequestStatus::Success;
                }
                else
                {
                    FMOD::Studio::EventDescription* desc = nullptr;
                    FMOD_STUDIO_PARAMETER_DESCRIPTION paramDesc;
                    if(auto it = g_PathIdToEventDescription.find(fmodState->m_targetEventPathId); it != g_PathIdToEventDescription.end())
                    {
                        desc = it->second;
                    }
                    else
                    {
                        AZ_Info("FMODAudioSystem", "We can't find event description from cache, we have to lookup manually.");
                        FMOD_GUID guid = {0};
                        if(m_studioSystem->lookupID(fmodState->m_targetPath.c_str(), &guid) == FMOD_OK)
                        {
                             if(m_studioSystem->getEventByID(&guid, &desc) == FMOD_OK)
                             {
                                 g_PathIdToEventDescription.emplace(fmodState->m_targetEventPathId, desc);
                             }
                        }
                    }
                    if(desc)
                    {
                        desc->getParameterDescriptionByName(fmodState->m_name.c_str(), &paramDesc);
                        fmodState->m_paramId = paramDesc.id;
                        fmodState->m_paramIdValid = true;
                        fmodObj->SetSwitch(fmodState);
                        result = EAudioRequestStatus::Success;
                    }
                }
            }
        }
    }


    return result;
}

EAudioRequestStatus AudioSystemImpl_FMOD::SetObstructionOcclusion(IATLAudioObjectData *objectData, float obstruction, float occlusion)
{
    auto fmodObj = static_cast<ATLAudioObject_FMOD*>(objectData);

    if(fmodObj)
    {
        if(fmodObj->GetId() != m_globalAudioObjectId)
        {
            fmodObj->SetObstructionOcclusion(obstruction, occlusion);
        }
    }

    return EAudioRequestStatus::Success;
}

EAudioRequestStatus AudioSystemImpl_FMOD::SetEnvironment(IATLAudioObjectData *objectData, const IATLEnvironmentImplData *environmentData, float amount)
{
    EAudioRequestStatus result = EAudioRequestStatus::Failure;
    auto fmodObj = static_cast<ATLAudioObject_FMOD*>(objectData);
    const auto fmodEnv = static_cast<const ATLEnvironment_FMOD*>(environmentData);

    if(fmodObj && fmodEnv)
    {
        if(fmodObj->GetId() != m_globalAudioObjectId)
        {
            fmodObj->SetEnvironment(fmodEnv, amount);
        }
        else
        {
            for(const auto obj : m_registeredAudioObjects)
            {
                obj->SetEnvironment(fmodEnv, amount);
            }
        }
        result = EAudioRequestStatus::Success;
    }

    return result;
}

EAudioRequestStatus AudioSystemImpl_FMOD::SetListenerPosition(IATLListenerData *listenerData, const SATLWorldPosition &newPosition) {
    auto ldata = static_cast<ATLListenerData_FMOD*>(listenerData);
    if(!ldata)
    {
        return Audio::EAudioRequestStatus::Failure;
    }

    ldata->m_attributes = Utils::CreateFMOD3DAttributes(newPosition.GetPositionVec(), newPosition.GetForwardVec(), newPosition.GetUpVec());
    FMOD_RESULT res = m_studioSystem->setListenerAttributes(ldata->m_id, &ldata->m_attributes);

    return (res == FMOD_OK) ? EAudioRequestStatus::Success : EAudioRequestStatus::Failure;
}

EAudioRequestStatus AudioSystemImpl_FMOD::ResetRtpc(IATLAudioObjectData *objectData, const IATLRtpcImplData *rtpcData)
{
    AZ_PROFILE_FUNCTION(Audio);
    EAudioRequestStatus result = EAudioRequestStatus::Failure;

    auto fmodObj  = static_cast<ATLAudioObject_FMOD*>(objectData);
    auto fmodRtpc = static_cast<const ATLParameterRtpc_FMOD*>(rtpcData);

    if(fmodRtpc->m_isGlobalParameter)
    {
        if(m_studioSystem->setParameterByID(fmodRtpc->m_paramId, fmodRtpc->m_defaultValue) == FMOD_OK)
            result = EAudioRequestStatus::Success;
    }
    else
    {
        if(fmodObj)
        {
            fmodObj->ResetParameter(fmodRtpc);
        }
    }

    return result;
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
                AZ_Info("FMODAudioSystem", "$3Bank '%s.bank' loaded successfully", implFileEntryData->m_baseBankName.data());
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

//TODO: delete?
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
    ATLTrigger_FMOD* newTriggerImpl = nullptr;
    if(!audioTriggerNode)
    {
        return nullptr;
    }

    const char* nodeName = audioTriggerNode->name();
    if(AZ::StringFunc::Equal(nodeName, XMLTags::EventTag))
    {
        if(auto pathAttr = audioTriggerNode->first_attribute(XMLTags::PathAttr))
        {
            const char* path = pathAttr->value();
            FMOD_GUID guid = { 0 };
            if(m_studioSystem->lookupID(path, &guid) == FMOD_OK)
            {
                auto eventType = FMODEventType::Play;
                if(auto evtTypeAttr = audioTriggerNode->first_attribute(XMLTags::EventTypeAttr))
                {
                    eventType = Utils::GetActionFromXmlStr(evtTypeAttr->value());
                }
                newTriggerImpl = azcreate(ATLTrigger_FMOD, (AudioStringToID<Audio::TAudioTriggerImplID>(path), eventType, nullptr, guid), Audio::AudioImplAllocator);
            }
        }
    }
    else if(AZ::StringFunc::Equal(nodeName, XMLTags::SnapshotTag))
    {
        if(auto pathAttr = audioTriggerNode->first_attribute(XMLTags::PathAttr))
        {
            const char* path = pathAttr->value();
            FMOD_GUID guid = {0};
            if(m_studioSystem->lookupID(path, &guid) == FMOD_OK)
            {
                FMOD::Studio::EventDescription* evtDescription = nullptr;
                m_studioSystem->getEventByID(&guid, &evtDescription);
                newTriggerImpl = azcreate(ATLTrigger_FMOD, (AudioStringToID<Audio::TAudioTriggerImplID>(path), FMODEventType::Play, evtDescription, guid), Audio::AudioImplAllocator);
            }
        }
    }

    return newTriggerImpl;
}

void AudioSystemImpl_FMOD::DeleteAudioTriggerImplData(IATLTriggerImplData *oldTriggerData)
{
    azdestroy(oldTriggerData, Audio::AudioImplAllocator, ATLTrigger_FMOD);
}

IATLRtpcImplData *AudioSystemImpl_FMOD::NewAudioRtpcImplData(const AZ::rapidxml::xml_node<char> *audioRtpcNode)
{
    if (!audioRtpcNode)
    {
        return nullptr;
    }
    ATLParameterRtpc_FMOD* rtpcData = nullptr;

    auto tagName = audioRtpcNode->name();

    if(AZ::StringFunc::Equal(tagName, XMLTags::EventParameterTag) || AZ::StringFunc::Equal(tagName, XMLTags::SnapshotParameterTag))
    {
        AZ::u32 targetId = INVALID_AUDIO_EVENT_ID;
        AZStd::fixed_string<256> targetPath;

        if(auto targetAttr = audioRtpcNode->first_attribute(XMLTags::TargetAttr))
        {
            targetPath = targetAttr->value();
            targetId = AudioStringToID<AZ::u32>(targetPath.c_str());
        }
        else
        {
            AZ_Error("FMODAudioSystem", false, "Missing Target Event Attribute!");
            return nullptr;
        }

        const char* name = nullptr;
        if(auto nameAttr = audioRtpcNode->first_attribute(XMLTags::NameAttr))
        {
            name = nameAttr->value();
        }

        rtpcData = azcreate(ATLParameterRtpc_FMOD, (), Audio::AudioImplAllocator);
        rtpcData->m_isGlobalParameter = false;
        rtpcData->m_targetPath = targetPath;
        rtpcData->m_targetId  = targetId;
        rtpcData->m_paramName = name;
    }
    else if(AZ::StringFunc::Equal(tagName, XMLTags::GlobalParameterTag))
    {
        auto pathAttr = audioRtpcNode->first_attribute(XMLTags::PathAttr);
        if(pathAttr)
        {
            AZStd::string path = pathAttr->value();
            FMOD_STUDIO_PARAMETER_DESCRIPTION desc;
            if(m_studioSystem->getParameterDescriptionByName(path.c_str(), &desc) == FMOD_OK)
            {
                rtpcData = azcreate(ATLParameterRtpc_FMOD, (), Audio::AudioImplAllocator);
                rtpcData->m_isGlobalParameter = true;
                rtpcData->m_paramId = desc.id;
                rtpcData->m_defaultValue = desc.defaultvalue;
                rtpcData->m_validParamId = true; //Just in case.
            }
            else
            {
                AZ_Error("FMODAudioSystem", false, "Unable to get global parameter named: %s", path.c_str());
                return nullptr;
            }
        }
    }
    else
    {
        return nullptr;
    }

    if(rtpcData)
    {
        if(auto multAttr = audioRtpcNode->first_attribute(XMLTags::MultAttr))
        {
            rtpcData->m_mult = AZ::StringFunc::ToFloat(multAttr->value());
        }

        if(auto shiftAttr = audioRtpcNode->first_attribute(XMLTags::ShiftAttr))
        {
            rtpcData->m_shift = AZ::StringFunc::ToFloat(shiftAttr->value());
        }
    }

    return rtpcData;
}

void AudioSystemImpl_FMOD::DeleteAudioRtpcImplData(IATLRtpcImplData *oldRtpcData)
{
    for(const auto fmodObj : m_registeredAudioObjects)
    {
        fmodObj->UnpostParameter(static_cast<ATLParameterRtpc_FMOD*>(oldRtpcData));
    }
    azdestroy(oldRtpcData, Audio::AudioImplAllocator, ATLParameterRtpc_FMOD);
}

IATLSwitchStateImplData *AudioSystemImpl_FMOD::NewAudioSwitchStateImplData(const AZ::rapidxml::xml_node<char> *audioSwitchStateNode)
{
    const char* tag = audioSwitchStateNode->name();

    ATLSwitchState_FMOD* fmodState = nullptr;
    if(AZ::StringFunc::Equal(tag, XMLTags::EventParameterTag) || AZ::StringFunc::Equal(tag, XMLTags::SnapshotParameterTag))
    {

        AZ::u32 targetId = INVALID_AUDIO_EVENT_ID;
        AZStd::fixed_string<256> targetPath;

        if(auto targetAttr = audioSwitchStateNode->first_attribute(XMLTags::TargetAttr))
        {
            targetPath = targetAttr->value();
            targetId = AudioStringToID<AZ::u32>(targetPath.c_str());
        }
        else
        {
            AZ_Error("FMODAudioSystem", false, "Missing Target Event Attribute!");
            return nullptr;
        }

        const char* name = nullptr;
        if(auto nameAttr = audioSwitchStateNode->first_attribute(XMLTags::NameAttr))
        {
            name = nameAttr->value();
        }

        fmodState = azcreate(ATLSwitchState_FMOD, (), Audio::AudioImplAllocator);
        fmodState->m_isGlobal = false;
        fmodState->m_targetEventPathId = targetId;
        fmodState->m_name = name;
    }
    else if(AZ::StringFunc::Equal(tag, XMLTags::GlobalParameterTag))
    {
        auto pathAttr = audioSwitchStateNode->first_attribute(XMLTags::PathAttr);
        if(pathAttr)
        {
            AZStd::string path = pathAttr->value();
            FMOD_STUDIO_PARAMETER_DESCRIPTION desc;
            if(m_studioSystem->getParameterDescriptionByName(path.c_str(), &desc) == FMOD_OK)
            {
                fmodState = azcreate(ATLSwitchState_FMOD, (), Audio::AudioImplAllocator);
                fmodState->m_isGlobal = true;
                fmodState->m_paramId = desc.id;
                fmodState->m_paramIdValid = true; //Just in case.
            }
            else
            {
                AZ_Error("FMODAudioSystem", false, "Unable to get global parameter named: %s", path.c_str());
                return nullptr;
            }
        }
    }
    else
    {
        return nullptr;
    }

    if(fmodState)
    {
        if(auto multAttr = audioSwitchStateNode->first_attribute(XMLTags::MultAttr))
        {
            fmodState->m_value = AZ::StringFunc::ToFloat(multAttr->value());
        }
    }

    return fmodState;

}

void AudioSystemImpl_FMOD::DeleteAudioSwitchStateImplData(IATLSwitchStateImplData *oldAudioSwitchStateData)
{
    for(const auto fmodObject : m_registeredAudioObjects)
    {
        fmodObject->UnsetSwitch(static_cast<ATLSwitchState_FMOD*>(oldAudioSwitchStateData));
    }

    azdestroy(oldAudioSwitchStateData, Audio::AudioImplAllocator, ATLSwitchState_FMOD);
}

IATLEnvironmentImplData *AudioSystemImpl_FMOD::NewAudioEnvironmentImplData(const AZ::rapidxml::xml_node<char> *audioEnvironmentNode)
{
    const char* tag = audioEnvironmentNode->name();
    ATLEnvironment_FMOD* envData = nullptr;

    if(AZ::StringFunc::Equal(tag, XMLTags::ReturnTag))
    {
        if(auto pathAttr = audioEnvironmentNode->first_attribute(XMLTags::PathAttr))
        {
            FMOD_GUID guid  = {0};
            const char* path = pathAttr->value();
            if(m_studioSystem->lookupID(path, &guid) == FMOD_OK)
            {
                FMOD::Studio::Bus* rBus = nullptr;
                m_studioSystem->getBusByID(&guid, &rBus);
                envData = azcreate(ATLEnvironment_FMOD, (nullptr, rBus), Audio::AudioImplAllocator);
            }
        }
    }
    else if(AZ::StringFunc::Equal(tag, XMLTags::SnapshotTag))
    {
        if(auto pathAttr = audioEnvironmentNode->first_attribute(XMLTags::PathAttr))
        {
            FMOD_GUID guid  = {0};
            const char* path = pathAttr->value();
            if(m_studioSystem->lookupID(path, &guid) == FMOD_OK)
            {
                FMOD::Studio::EventDescription* snapshotDesc = nullptr;
                m_studioSystem->getEventByID(&guid, &snapshotDesc);
                envData = azcreate(ATLEnvironment_FMOD, (snapshotDesc, nullptr), Audio::AudioImplAllocator);
            }
        }
    }

    return envData;
}

void AudioSystemImpl_FMOD::DeleteAudioEnvironmentImplData(IATLEnvironmentImplData *oldEnvironmentData)
{
    for(const auto fmodObj : m_registeredAudioObjects)
    {
        fmodObj->UnsetEnvironment(static_cast<ATLEnvironment_FMOD*>(oldEnvironmentData));
    }

    azdestroy(oldEnvironmentData, Audio::AudioImplAllocator, ATLEnvironment_FMOD);
}

IATLAudioObjectData *AudioSystemImpl_FMOD::NewGlobalAudioObjectData(TAudioObjectID objectId)
{
    auto obj = azcreate(ATLAudioObject_FMOD, (objectId), Audio::AudioImplAllocator);
    m_globalAudioObjectId = objectId;
    return obj;
}

IATLAudioObjectData *AudioSystemImpl_FMOD::NewAudioObjectData(TAudioObjectID objectId)
{
    return azcreate(ATLAudioObject_FMOD, (objectId), Audio::AudioImplAllocator);
}

void AudioSystemImpl_FMOD::DeleteAudioObjectData(IATLAudioObjectData *oldObjectData) {

    azdestroy(oldObjectData, Audio::AudioImplAllocator, ATLAudioObject_FMOD);
}

IATLListenerData *AudioSystemImpl_FMOD::NewDefaultAudioListenerObjectData(TATLIDType objectId) {
    return azcreate(ATLListenerData_FMOD, (0), Audio::AudioImplAllocator);
}

IATLListenerData *AudioSystemImpl_FMOD::NewAudioListenerObjectData(TATLIDType objectId) {
    static int id = 0;
    return azcreate(ATLListenerData_FMOD, (++id), Audio::AudioImplAllocator);
}

void AudioSystemImpl_FMOD::DeleteAudioListenerObjectData(IATLListenerData *oldListenerData) {
    auto listener = static_cast<ATLListenerData_FMOD*>(oldListenerData);
    azdestroy(listener, Audio::AudioImplAllocator, ATLListenerData_FMOD);
}

IATLEventData *AudioSystemImpl_FMOD::NewAudioEventData(TAudioEventID eventID) {
    auto audioEventData = azcreate(ATLEventData_FMOD, (eventID), Audio::AudioImplAllocator);
    return audioEventData;
}

void AudioSystemImpl_FMOD::DeleteAudioEventData(IATLEventData *oldEventData) {
    azdestroy(oldEventData, Audio::AudioImplAllocator, ATLAudioObject_FMOD);
}

void AudioSystemImpl_FMOD::ResetAudioEventData(IATLEventData *eventData) {
    auto event = static_cast<ATLEventData_FMOD*>(eventData);
    if(event)
    {
        event->Reset();
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
        m_lowlvlSystem->mixerSuspend();
    }
}

void AudioSystemImpl_FMOD::OnAudioSystemGetFocus() {
    if(CVars::s_FMODStudio_PauseAudioOnFocusLost)
    {
        m_lowlvlSystem->mixerResume();
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

    g_PathIdToEventDescription.clear();
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
}