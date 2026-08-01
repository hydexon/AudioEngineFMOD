#include "AudioSystemImpl_FMOD.h"
#include "AudioSystemImplCVars.h"
#include "ATLEntities_FMOD.h"
#include "FMOD_FileSystemIO.h"
#include "Common_FMOD.h"
#include "ConfigFMOD.h"
#include "fmod_errors.h"

#include <AzCore/IO/FileIO.h>
#include <ATLEntityData.h>
#include <AzCore/Debug/Profiler.h>
#include <AzCore/std/algorithm.h>
#include <fmod.hpp>

using namespace Audio;

namespace AudioEngineFMOD
{

    namespace MemCallbacks
    {
        void* F_CALL Malloc(unsigned int size, FMOD_MEMORY_TYPE type, const char* srcstr)
        {
            return AZ::AllocatorInstance<Audio::AudioImplAllocator>::Get().Allocate(size, 0, 0, nullptr);
        }

        void* F_CALL Realloc(void* ptr, unsigned int size, FMOD_MEMORY_TYPE type, const char* srcstr)
        {
            return AZ::AllocatorInstance<Audio::AudioImplAllocator>::Get().ReAllocate(ptr, size, 0);
        }

        void F_CALL Free(void* ptr, FMOD_MEMORY_TYPE type, const char* srcstr)
        {
            return AZ::AllocatorInstance<Audio::AudioImplAllocator>::Get().DeAllocate(ptr);
        }
    }


const char* AudioSystemImpl_FMOD::s_defaultFMODLocaleCode = "EN";

AudioSystemImpl_FMOD::AudioSystemImpl_FMOD(const char* assetPlatformName)
    : m_studioSystem(nullptr)
    , m_masterBank(nullptr)
    , m_masterStringsBank(nullptr)
    , m_currentFMODLocale(s_defaultFMODLocaleCode)
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

void AudioSystemImpl_FMOD::Update(float updateIntervalMS) {
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

    FMOD_RESULT result = FMOD::Studio::System::create(&m_studioSystem);

    if(result != FMOD_OK)
    {
        AZ_Error("FMODAudioSystem", false, "Unable to create FMOD Studio System.");
        return EAudioRequestStatus::Failure;
    }

    FMOD_STUDIO_ADVANCEDSETTINGS studioSettings;
    m_studioSystem->getAdvancedSettings(&studioSettings);

    studioSettings.commandqueuesize  = CVars::s_FMODStudio_CommandQueueSize;
    studioSettings.studioupdateperiod = CVars::s_FMODStudio_StudioUpdatePeriod;
    studioSettings.idlesampledatapoolsize = CVars::s_FMODStudio_IdleSampleDataPoolSize;
    studioSettings.streamingscheduledelay = CVars::s_FMODStudio_StreamingScheduleDelay;

    m_studioSystem->setAdvancedSettings(&studioSettings);

    FMOD::System* coreSystem = nullptr;
    m_studioSystem->getCoreSystem(&coreSystem);

    FMOD_ADVANCEDSETTINGS coreSettings;
    coreSettings.profilePort = CVars::s_FMODCore_ProfilePort;
    coreSystem->setAdvancedSettings(&coreSettings);

    coreSystem->setFileSystem(
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
    LoadFMODLocaleMappings();

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

    m_studioSystem->setNumListeners(4); //@HACK: No idea why O3DE is trying to set 3 listeners.

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
        masterBus->stopAllEvents(FMOD_STUDIO_STOP_ALLOWFADEOUT);
        return EAudioRequestStatus::Success;
    }

    return EAudioRequestStatus::Success;
}

EAudioRequestStatus AudioSystemImpl_FMOD::RegisterAudioObject(IATLAudioObjectData *objectData, const char *objectName) {
    // FMOD Has no motion of "Audio Objects" like wwise..
    return EAudioRequestStatus::Success;
}

EAudioRequestStatus AudioSystemImpl_FMOD::UnregisterAudioObject(IATLAudioObjectData *objectData) {
    // Same here either.
    return EAudioRequestStatus::Success;
}

EAudioRequestStatus AudioSystemImpl_FMOD::ResetAudioObject(IATLAudioObjectData *objectData) {
    if(objectData)
    {
        StopAllAndClearInstancesFromAudioObject(objectData);
        auto fmodObj = static_cast<SATLAudioObjectData_FMOD*>(objectData);
        fmodObj->m_3dAttributes = {};

        return EAudioRequestStatus::Success;
    }

    return EAudioRequestStatus::Failure;
}

EAudioRequestStatus AudioSystemImpl_FMOD::UpdateAudioObject(IATLAudioObjectData *objectData) {
    auto audioObject = static_cast<SATLAudioObjectData_FMOD*>(objectData);
    if(!audioObject)
    {
        return EAudioRequestStatus::FailureInvalidObjectId;
    }

    ClearStoppedEventInstances(objectData);

    return EAudioRequestStatus::Success;
}

EAudioRequestStatus AudioSystemImpl_FMOD::PrepareTriggerSync(IATLAudioObjectData *audioObjectData, const IATLTriggerImplData *triggerData) {
    AZ_Info("FMODAudioSystem", "Preparing Trigger!");
    return EAudioRequestStatus::Success;
}

EAudioRequestStatus AudioSystemImpl_FMOD::UnprepareTriggerSync(IATLAudioObjectData *objectData, const IATLTriggerImplData *triggerData) {

    return EAudioRequestStatus::Success;
}

EAudioRequestStatus AudioSystemImpl_FMOD::PrepareTriggerAsync(IATLAudioObjectData *objectData, const IATLTriggerImplData *triggerData, IATLEventData *eventData) {
    AZ_Info("FMODAudioSystem", "Preparing Trigger (Async)!");
    return EAudioRequestStatus::Success;
}

EAudioRequestStatus AudioSystemImpl_FMOD::UnprepareTriggerAsync(IATLAudioObjectData *pAudioObjectData, const IATLTriggerImplData *pTriggerData, IATLEventData *pEventData) {
    return EAudioRequestStatus::Success;
}

EAudioRequestStatus AudioSystemImpl_FMOD::ActivateTrigger(IATLAudioObjectData *objectData, const IATLTriggerImplData *triggerData, IATLEventData *eventData, const SATLSourceData *sourceData) {
    auto result = EAudioRequestStatus::Success;

    auto implObjData     = static_cast<SATLAudioObjectData_FMOD*>(objectData);
    auto implTriggerData = static_cast<const SATLTriggerImplData_FMOD*>(triggerData);
    auto implEventData   = static_cast<SATLEventData_FMOD*>(eventData);

    AZ_UNUSED(sourceData); //We should implement FMOD Core playback of .ogg, .mp3 and .FLACs?
    if(implObjData && implTriggerData && implEventData)
    {
        if(!implEventData->m_eventDescription)
        {
            m_studioSystem->getEventByID(&implTriggerData->m_eventGUID, &implEventData->m_eventDescription);
        }

        switch(implTriggerData->m_action) {
        case FMODEventAction::Play: {
            if(!implEventData->m_currentInstance)
            {
                FMOD_RESULT evtRst = implEventData->m_eventDescription->createInstance(&implEventData->m_currentInstance);
                if(evtRst != FMOD_OK)
                {
                    AZ_Error("FMODAudioSystem", false, "FMOD Studio Failed to create event instance of %s: %s",
                             implTriggerData->m_eventPath.c_str(), FMOD_ErrorString(evtRst));
                    result = EAudioRequestStatus::Failure;
                }
            }
            bool evtIs3D = false;
            implEventData->m_eventDescription->is3D(&evtIs3D);
            if(evtIs3D)
            {
                implEventData->m_currentInstance->set3DAttributes(&implObjData->m_3dAttributes);
            }
            implEventData->m_currentInstance->start();
            implObjData->m_activeInstances.push_back(implEventData->m_currentInstance);
            implEventData->m_stopMode = implTriggerData->m_stopMode;
            implEventData->m_eventPath = implTriggerData->m_eventPath;
            implEventData->atlName = implTriggerData->atlName;
            break;
        }
        case FMODEventAction::Resume:
            AZ_Assert(implEventData->m_currentInstance, "implEventData->m_currentInstance is null!");
            implEventData->m_currentInstance->setPaused(true);
            break;
        case FMODEventAction::Pause:
            AZ_Assert(implEventData->m_currentInstance, "implEventData->m_currentInstance is null!");
            implEventData->m_currentInstance->setPaused(false);
            break;
        case FMODEventAction::Stop: {
            AZ_Assert(implEventData->m_currentInstance, "implEventData->m_currentInstance is null!");
            implEventData->m_currentInstance->stop(implTriggerData->m_stopMode);
            implEventData->m_currentInstance = nullptr; //ClearStoppedEventInstances will take care about in the Audio Object.
            implEventData->atlName = implTriggerData->atlName;
            break;
        }
        default:
            break;
        }
    }

    return result;
}

EAudioRequestStatus AudioSystemImpl_FMOD::StopEvent(IATLAudioObjectData *objectData, const IATLEventData *eventData) {
    EAudioRequestStatus result = EAudioRequestStatus::Success;
    //this->StopAllEvents(objectData);
    auto implEvtData = static_cast<const SATLEventData_FMOD*>(eventData);
    auto implObjData = static_cast<SATLAudioObjectData_FMOD*>(objectData);

    AZ_Info("FMODAudioSystem", "Trying to Stop ATLName: %s, Event: %s (%s), Objects EventInstance Count: %d",
            implEvtData->atlName.c_str(),
            implEvtData->m_eventPath.c_str(),
            Utils::GetXmlStrFromAction(implEvtData->m_actionMode).c_str(),
            implObjData->m_activeInstances.size()
            );

    if(implEvtData->m_actionMode != FMODEventAction::Play)
    {
        return EAudioRequestStatus::Success;
    }

    if(implEvtData)
    {
        if(implObjData)
        {
            AZ_Assert(implEvtData->m_eventDescription, "SATLEventData_FMOD m_eventDescription is null!");
            if(implEvtData->m_currentInstance)
            {
                implEvtData->m_currentInstance->stop(implEvtData->m_stopMode);
                implEvtData->m_currentInstance->release();
                //ClearStoppedEventInstances will take care.
                result = EAudioRequestStatus::Success;
            }
        }
    }
    return result;
}

EAudioRequestStatus AudioSystemImpl_FMOD::StopAllEvents(IATLAudioObjectData *objectData) {
    StopAllAndClearInstancesFromAudioObject(objectData);
    return EAudioRequestStatus::Success;
}

EAudioRequestStatus AudioSystemImpl_FMOD::SetPosition(IATLAudioObjectData *objectData, const SATLWorldPosition &worldPosition) {
    auto* fmodObj = static_cast<SATLAudioObjectData_FMOD*>(objectData);
    if(!fmodObj)
    {
        return EAudioRequestStatus::FailureInvalidObjectId;
    }

    fmodObj->m_3dAttributes = Utils::CreateFMOD3DAttributes(worldPosition.GetPositionVec(),
                                                            worldPosition.GetForwardVec(),
                                                            worldPosition.GetUpVec());

    for(auto* instances : fmodObj->m_activeInstances)
    {
        instances->set3DAttributes(&fmodObj->m_3dAttributes);
    }

    return EAudioRequestStatus::Success;
}

EAudioRequestStatus AudioSystemImpl_FMOD::SetMultiplePositions(IATLAudioObjectData *objectData, const MultiPositionParams &multiPositions) {
    AZ_Error("FMODAudioSystem", false, "Set Multiple Positions in FMOD are not supported!");
    return EAudioRequestStatus::FailureInvalidRequest;
}

EAudioRequestStatus AudioSystemImpl_FMOD::SetRtpc(IATLAudioObjectData *objectData, const IATLRtpcImplData *rtpcData, float value) {
    // TODO: Implement this pure virtual method.
    return EAudioRequestStatus::Success;
}

EAudioRequestStatus AudioSystemImpl_FMOD::SetSwitchState(IATLAudioObjectData *objectData, const IATLSwitchStateImplData *switchStateData) {
    // TODO: Implement this pure virtual method.
    return EAudioRequestStatus::Success;
}

EAudioRequestStatus AudioSystemImpl_FMOD::SetObstructionOcclusion(IATLAudioObjectData *objectData, float obstruction, float occlusion) {
    AZ_Warning("FMODAudioSystem", false, "Set Obstruction is not supported yet in this FMOD implementation, only occlusion");
    auto fmodObj = static_cast<SATLAudioObjectData_FMOD*>(objectData);

    constexpr float ObstructionOcclusionMin = 0.0f;
    constexpr float ObstructionOcclusionMax = 1.0f;

    if (obstruction < ObstructionOcclusionMin || obstruction > ObstructionOcclusionMax)
    {
        AZ_Warning("FMODAudioSystem", false,
            "Obstruction value %f is out of range, Obstruction should be in range [%f, %f]", obstruction, ObstructionOcclusionMin,
            ObstructionOcclusionMax);
    }

    if (occlusion < ObstructionOcclusionMin || occlusion > ObstructionOcclusionMax)
    {
        AZ_Warning("FMODAudioSystem", false,
            "Occlusion value %f is out of range, Occlusion should be in range [%f, %f]", occlusion, ObstructionOcclusionMin,
            ObstructionOcclusionMax);
    }

    if(fmodObj)
    {
        for(auto instance : fmodObj->m_activeInstances)
        {
            instance->setParameterByName("Occlusion", occlusion);
        }
    }

    return EAudioRequestStatus::Success;
}

EAudioRequestStatus AudioSystemImpl_FMOD::SetEnvironment(IATLAudioObjectData *objectData, const IATLEnvironmentImplData *environmentData, float amount) {
    return EAudioRequestStatus::None;
}

EAudioRequestStatus AudioSystemImpl_FMOD::SetListenerPosition(IATLListenerData *listenerData, const SATLWorldPosition &newPosition) {
    auto ldata = static_cast<SATLListenerData_FMOD*>(listenerData);
    if(!ldata)
    {
        return Audio::EAudioRequestStatus::Failure;
    }

    FMOD_3D_ATTRIBUTES listenerAttributes = Utils::CreateFMOD3DAttributes(newPosition.GetPositionVec(),
                                                                          newPosition.GetForwardVec(),
                                                                          newPosition.GetUpVec(),
                                                                          ldata->velocity);

    m_studioSystem->setListenerAttributes(ldata->listenerIndex, &listenerAttributes);
    m_studioSystem->setListenerWeight(ldata->listenerIndex, ldata->weight);

    return EAudioRequestStatus::Success;
}

EAudioRequestStatus AudioSystemImpl_FMOD::ResetRtpc(IATLAudioObjectData *objectData, const IATLRtpcImplData *rtpcData) {
    return EAudioRequestStatus::Success;
}

EAudioRequestStatus AudioSystemImpl_FMOD::RegisterInMemoryFile(SATLAudioFileEntryInfo *audioFileEntry) {

    EAudioRequestStatus result = EAudioRequestStatus::Success;
    if(audioFileEntry)
    {
        //@TODO: Localized banks.
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
                if(implFileEntryData->m_loadSampleData)
                {
                    if(FMOD_RESULT loadResult = implFileEntryData->m_FMODBank->loadSampleData(); loadResult != FMOD_OK)
                    {
                        AZ_Error("FMODAudioSystem", false, "Unable to load sample data for bank '%s', FMOD Error: %s",
                                 audioFileEntry->sFileName, FMOD_ErrorString(loadResult));
                        result = EAudioRequestStatus::PartialSuccess;
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
        auto const implFileEntryData = static_cast<SATLAudioFileEntryData_FMOD*>(audioFileEntry->pImplData);
        if(implFileEntryData)
        {
            FMOD_RESULT bankResult = implFileEntryData->m_FMODBank->unload();
            if(bankResult != FMOD_OK)
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
    if(audioFileEntryNode && azstricmp(audioFileEntryNode->name(), XMLTags::FMODStudioBankTag) == 0 && fileEntryInfo)
    {
        const char* audioFileEntryName = nullptr;
        auto bankPathAttr = audioFileEntryNode->first_attribute(XMLTags::FMODPathAttribute, 0, false);
        if(bankPathAttr)
        {
            audioFileEntryName = bankPathAttr->value();
        }

        bool isLocalized = false;
        auto localizedAttr = audioFileEntryNode->first_attribute(XMLTags::FMODLocalizedAttribute, 0, false);
        if(localizedAttr)
        {
            if(azstricmp(localizedAttr->value(), "true") == 0)
            {
                isLocalized = true;
            }
        }

        bool loadSampleData = false;
        auto loadSampleDataAttr = audioFileEntryNode->first_attribute(XMLTags::FMODSamplePreloadAttr, 0, false);
        if(loadSampleDataAttr)
        {
            if(azstricmp(loadSampleDataAttr->value(), "true") == 0)
            {
                loadSampleData = true;
            }
        }

        if(audioFileEntryName && audioFileEntryName[0] != '\0')
        {
            AZStd::string bankBaseName = audioFileEntryName;
            AZ::StringFunc::Path::ReplaceExtension(bankBaseName, "");

            if(isLocalized)
            {
                /*
                 * HACK: Due we can't modify audioFileEntry because is a C string reference from the Audio Control XML file
                 *       we're reading from, expected due the Wwise behavior the ATL replicates from, where every localized
                 *       soundbank, audio file has their own localized folder, unlike FMOD's where localized banks has an
                 *       locale code postfix like Dialogue_EN.bank, Dialogue_JP.bank and Dialogue_CN.bank and so on, instead.
                 *
                 *       As a solution we have our own pool of strings handled by this class with the modified strings to locate
                 *       the banks easily without modifying the ATL API or other nastier hacks more than this.
                 */
                m_loadedLocalizedBanksNames.emplace_back();
                auto& localizedBankName = m_loadedLocalizedBanksNames.back();

                localizedBankName = AZStd::string::format("%s_%s.bank", bankBaseName.c_str(), m_currentFMODLocale.data());
                AZ_Printf("FMODAudioSystem", "New Bank Name for Localized Bank (%s): %s", m_currentFMODLocale.data(), localizedBankName.data());

                audioFileEntryName = localizedBankName.c_str();
            }

            fileEntryInfo->bLocalized = isLocalized;
            fileEntryInfo->nMemoryBlockAlignment = FMOD_STUDIO_LOAD_MEMORY_ALIGNMENT;
            fileEntryInfo->sFileName = audioFileEntryName;
            fileEntryInfo->pImplData = azcreate(SATLAudioFileEntryData_FMOD, (bankBaseName.c_str(), loadSampleData), Audio::AudioImplAllocator);
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

void AudioSystemImpl_FMOD::DeleteAudioFileEntryData(IATLAudioFileEntryData *oldAudioFileEntryData) {
    azdestroy(oldAudioFileEntryData, Audio::AudioImplAllocator, SATLAudioFileEntryData_FMOD);
}

const char * const AudioSystemImpl_FMOD::GetAudioFileLocation(SATLAudioFileEntryInfo *fileEntryInfo) {
    if(fileEntryInfo)
    {
        return m_fmodBankPath.c_str();
    }

    return nullptr;
}

const char *AudioSystemImpl_FMOD::GetAudioFilenameFixup(Audio::IATLAudioFileEntryData *audioFileEntryData)
{
    const char* result = nullptr;
    auto* fmodFileData = static_cast<SATLAudioFileEntryData_FMOD*>(audioFileEntryData);

    if(fmodFileData)
    {
        m_loadedLocalizedBanksNames.emplace_back();
        auto& localizedBankName = m_loadedLocalizedBanksNames.back();

        localizedBankName = AZStd::string::format("%s_%s.bank", fmodFileData->m_baseBankName.c_str(), m_currentFMODLocale.data());
        result = localizedBankName.c_str();
    }

    return result;
}

IATLTriggerImplData *AudioSystemImpl_FMOD::NewAudioTriggerImplData(const AZ::rapidxml::xml_node<char> *audioTriggerNode) {
    SATLTriggerImplData_FMOD* newTriggerImpl = nullptr;

    if(audioTriggerNode && azstricmp(audioTriggerNode->name(), XMLTags::FMODEventTag) == 0) {
        //Get the ATL Control Trigger name for debug purposes:
        AZStd::string atlName;
        if(audioTriggerNode->parent())
        {
            const char* parentName = audioTriggerNode->parent()->name();
            if(azstricmp(parentName, "ATLTrigger") == 0)
            {
                auto atlnameAttr = audioTriggerNode->parent()->first_attribute("atl_name");
                if(atlnameAttr)
                {
                    atlName = atlnameAttr->value();
                }
            }
        }
        auto eventNameAttr = audioTriggerNode->first_attribute(XMLTags::FMODPathAttribute, 0, false);
        auto preloadSampleDataAttr = audioTriggerNode->first_attribute(XMLTags::FMODSamplePreloadAttr, 0, false);
        auto actionAttr = audioTriggerNode->first_attribute(XMLTags::FMODEvtAction, 0, false);
        if(eventNameAttr)
        {
            const char* eventName = eventNameAttr->value();
            newTriggerImpl = azcreate(SATLTriggerImplData_FMOD, (), Audio::AudioImplAllocator);
            newTriggerImpl->m_eventPath = eventName;
            newTriggerImpl->m_preloadSampleData = !preloadSampleDataAttr ? false : azstricmp(preloadSampleDataAttr->value(), "true") == 0;
            newTriggerImpl->m_action = FMODEventAction::Play;
            newTriggerImpl->atlName = atlName;
            if(actionAttr)
            {
                auto actionStr = actionAttr->value();
                newTriggerImpl->m_action = Utils::GetActionFromXmlStr(actionStr);
            }

            FMOD_RESULT evtIdR = m_studioSystem->lookupID(eventName, &newTriggerImpl->m_eventGUID);
            AZ_Warning("FMODAudioSystem", evtIdR == FMOD_OK, "FMOD Error trying to query up GUID of event %s: %s", eventName, FMOD_ErrorString(evtIdR));
            //@TODO: The rest of the data from the XML.
        }
    }
    return newTriggerImpl;
}

void AudioSystemImpl_FMOD::DeleteAudioTriggerImplData(IATLTriggerImplData *oldTriggerData) {
    azdestroy(oldTriggerData, Audio::AudioImplAllocator, SATLTriggerImplData_FMOD);
}

IATLRtpcImplData *AudioSystemImpl_FMOD::NewAudioRtpcImplData(const AZ::rapidxml::xml_node<char> *audioRtpcNode) {
    // TODO: Implement this pure virtual method.
    return nullptr;
}

void AudioSystemImpl_FMOD::DeleteAudioRtpcImplData(IATLRtpcImplData *oldRtpcData) {
    // TODO: Implement this pure virtual method.
}

IATLSwitchStateImplData *AudioSystemImpl_FMOD::NewAudioSwitchStateImplData(const AZ::rapidxml::xml_node<char> *audioSwitchStateNode) {
    // TODO: Implement this pure virtual method.
    return nullptr;

}

void AudioSystemImpl_FMOD::DeleteAudioSwitchStateImplData(IATLSwitchStateImplData *oldAudioSwitchStateData) {
    // TODO: Implement this pure virtual method.
}

IATLEnvironmentImplData *AudioSystemImpl_FMOD::NewAudioEnvironmentImplData(const AZ::rapidxml::xml_node<char> *audioEnvironmentNode) {
    // TODO: Implement this pure virtual method.
    return nullptr;

}

void AudioSystemImpl_FMOD::DeleteAudioEnvironmentImplData(IATLEnvironmentImplData *oldEnvironmentData) {
    // TODO: Implement this pure virtual method.
}

IATLAudioObjectData *AudioSystemImpl_FMOD::NewGlobalAudioObjectData(TAudioObjectID objectId) {
    return NewAudioObjectData(objectId);
}

IATLAudioObjectData *AudioSystemImpl_FMOD::NewAudioObjectData(TAudioObjectID objectId) {
    AZ_UNUSED(objectId);
    auto newObjectData = azcreate(SATLAudioObjectData_FMOD, (), Audio::AudioImplAllocator);
    return newObjectData;
}

void AudioSystemImpl_FMOD::DeleteAudioObjectData(IATLAudioObjectData *oldObjectData) {

    StopAllAndClearInstancesFromAudioObject(oldObjectData);
    azdestroy(oldObjectData, Audio::AudioImplAllocator, SATLAudioObjectData_FMOD);
}

IATLListenerData *AudioSystemImpl_FMOD::NewDefaultAudioListenerObjectData(TATLIDType objectId) {
    return NewAudioListenerObjectData(objectId);
}

IATLListenerData *AudioSystemImpl_FMOD::NewAudioListenerObjectData(TATLIDType objectId) {
    auto newListenerData = azcreate(SATLListenerData_FMOD, (aznumeric_cast<int>(objectId)), Audio::AudioImplAllocator);
    if(newListenerData)
    {
        m_studioSystem->getListenerWeight(newListenerData->listenerIndex, &newListenerData->weight);
    }
    return newListenerData;
}

void AudioSystemImpl_FMOD::DeleteAudioListenerObjectData(IATLListenerData *oldListenerData) {
    azdestroy(oldListenerData, Audio::AudioImplAllocator, SATLListenerData_FMOD);
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
        *event = SATLEventData_FMOD();
    }
}

void AudioSystemImpl_FMOD::SetLanguage(const char *language) {
    if(m_availableLocales.find(language) != m_availableLocales.end())
    {
        m_currentFMODLocale = m_availableLocales[language];
        AZ_Info("FMODAudioSystem", "SetLanguage changed!, New Locale Set: %s", m_availableLocales[language].c_str());
    }
    else
    {
        AZ_Error("FMODAudioSystem", false, "FMOD Locale Code for AudioSystem Language '%s' is missing or non-existing, defaulting to EN (English) instead", language);
        m_currentFMODLocale = s_defaultFMODLocaleCode;
    }
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
    return false;
}

void AudioSystemImpl_FMOD::DestroyAudioSource(TAudioSourceId sourceId) {
    // TODO: Implement this pure virtual method.
}

void AudioSystemImpl_FMOD::SetPanningMode(PanningMode mode) {
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
}

void AudioSystemImpl_FMOD::OnAudioSystemUnmuteAll() {
    //See comments from OnAudioSystemMuteAll().
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

    m_loadedLocalizedBanksNames.clear();

}

void AudioSystemImpl_FMOD::SetBankPaths()
{
    //Default:
    // "Assets/Audio/FMOD/"
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
    SetBanksRootPath(m_fmodBankPath);
}

void AudioSystemImpl_FMOD::LoadFMODLocaleMappings()
{
    //FMOD Root Path:
    //'Assets/Audio/FMOD/'
    AZStd::string rootPath   = Constants::DefaultFMODRootPath;
    //'Assets/Audio/FMOD/FMODLocaleConfig.json'
    AZStd::string configFile = rootPath + Constants::LocaleConfigFile;

    if(AZ::IO::FileIOBase::GetInstance()
            && AZ::IO::FileIOBase::GetInstance()->Exists(configFile.c_str()))
    {
        FMODLocaleConfig localeConfigs;
        if(localeConfigs.Load(configFile))
        {
            for(const auto& localesMap : localeConfigs.m_localeMappings)
            {
                m_availableLocales[localesMap.m_languageName] = localesMap.m_localeCode;
            }
        }
    }
    else
    {
        AZ_Error("FMODAudioSystem", false, "Failed to find '%s' to load locales", configFile.c_str());
    }
}

void AudioSystemImpl_FMOD::StopAllAndClearInstancesFromAudioObject(Audio::IATLAudioObjectData *sndObj)
{
    SATLAudioObjectData_FMOD* fmodObj = static_cast<SATLAudioObjectData_FMOD*>(sndObj);
    if(!fmodObj)
        return;

    for(auto* instance : fmodObj->m_activeInstances)
    {
        instance->stop(FMOD_STUDIO_STOP_IMMEDIATE);
        instance->release();
        instance = nullptr;
    }
    fmodObj->m_activeInstances.clear();
}

void AudioSystemImpl_FMOD::ClearStoppedEventInstances(Audio::IATLAudioObjectData *sndObj)
{
    SATLAudioObjectData_FMOD* fmodObj = static_cast<SATLAudioObjectData_FMOD*>(sndObj);
    if(!fmodObj)
        return;

    auto clearCondition = [](FMOD::Studio::EventInstance* inst) {
        FMOD_STUDIO_PLAYBACK_STATE state;
        inst->getPlaybackState(&state);
        if(state == FMOD_STUDIO_PLAYBACK_STOPPED)
        {
            AZ_Info("FMODAudioSystem", "Freeing Stopped EventInstance");
            inst->release();
            inst = nullptr;
            return true;
        }
        return false;
    };

    fmodObj->m_activeInstances.erase(
                AZStd::remove_if(fmodObj->m_activeInstances.begin(),
                                 fmodObj->m_activeInstances.end(),
                                 clearCondition),
                fmodObj->m_activeInstances.end());

}


}