#include "AudioSystemImpl_FMOD.h"
#include "AudioSystemImplCVars.h"
#include "ATLEntities_FMOD.h"
#include "FMOD_FileSystemIO.h"
#include "Common_FMOD.h"

#include <AzCore/Debug/Profiler.h>
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


AudioSystemImpl_FMOD::AudioSystemImpl_FMOD()
    : studioSystem(nullptr)
{
    AudioSystemImplementationRequestBus::Handler::BusConnect();
    AudioSystemImplementationNotificationBus::Handler::BusConnect();
}

AudioSystemImpl_FMOD::~AudioSystemImpl_FMOD()
{
    AudioSystemImplementationRequestBus::Handler::BusDisconnect();
    AudioSystemImplementationNotificationBus::Handler::BusDisconnect();
}

void AudioSystemImpl_FMOD::Update(float updateIntervalMS) {
    // TODO: Implement this pure virtual method.
    AZ_PROFILE_FUNCTION(Audio);
    studioSystem->update();
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

    FMOD_RESULT result = FMOD::Studio::System::create(&studioSystem);

    if(result != FMOD_OK)
    {
        AZ_Error("FMODAudioSystem", false, "Unable to create FMOD Studio System.");
        return EAudioRequestStatus::Failure;
    }

    FMOD_STUDIO_ADVANCEDSETTINGS studioSettings;
    studioSystem->getAdvancedSettings(&studioSettings);

    studioSettings.commandqueuesize  = CVars::s_FMODStudio_CommandQueueSize;
    studioSettings.studioupdateperiod = CVars::s_FMODStudio_StudioUpdatePeriod;
    studioSettings.idlesampledatapoolsize = CVars::s_FMODStudio_IdleSampleDataPoolSize;
    studioSettings.streamingscheduledelay = CVars::s_FMODStudio_StreamingScheduleDelay;

    studioSystem->setAdvancedSettings(&studioSettings);

    FMOD::System* coreSystem = nullptr;
    studioSystem->getCoreSystem(&coreSystem);

    FMOD_ADVANCEDSETTINGS coreSettings;
    coreSettings.profilePort = CVars::s_FMODCore_ProfilePort;
    coreSystem->setAdvancedSettings(&coreSettings);

    coreSystem->setFileSystem(
                SyncIO::AzFileOpen,
                SyncIO::AzFileClose,
                SyncIO::AzFileRead,
                SyncIO::AzFileSeek,
                AsyncIO::AzAsyncFileRead,
                AsyncIO::AzAsyncFileCancel,
                -1
                );

    result = studioSystem->initialize(CVars::s_FMODStudio_MaxChannels,
                                      CVars::s_FMODStudio_EnableProfiling ? FMOD_STUDIO_INIT_LIVEUPDATE : FMOD_STUDIO_INIT_NORMAL,
                                      FMOD_INIT_NORMAL,
                                      nullptr);



    if(result != FMOD_OK)
    {
        AZ_Error("FMODAudioSystem", false, "Unable to Initialize FMOD Studio");
        return EAudioRequestStatus::Failure;
    }

    return EAudioRequestStatus::Success;
}

EAudioRequestStatus AudioSystemImpl_FMOD::ShutDown() {
    studioSystem->release();
    return EAudioRequestStatus::Success;
}

EAudioRequestStatus AudioSystemImpl_FMOD::Release() {
    return EAudioRequestStatus::Success;
}

EAudioRequestStatus AudioSystemImpl_FMOD::StopAllSounds() {
    FMOD::Studio::Bus* masterBus = nullptr;
    studioSystem->getBus("bus:/", &masterBus);
    if(masterBus)
    {
        masterBus->stopAllEvents(FMOD_STUDIO_STOP_ALLOWFADEOUT);
        return EAudioRequestStatus::Success;
    }

    return EAudioRequestStatus::Failure;
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
    // TODO: Implement this pure virtual method.
    return EAudioRequestStatus::None;
}

EAudioRequestStatus AudioSystemImpl_FMOD::UpdateAudioObject(IATLAudioObjectData *objectData) {
    // TODO: Implement this pure virtual method.
    return EAudioRequestStatus::None;
}

EAudioRequestStatus AudioSystemImpl_FMOD::PrepareTriggerSync(IATLAudioObjectData *audioObjectData, const IATLTriggerImplData *triggerData) {

    const auto fmodTriggerData = static_cast<const SATLTriggerImplData_FMOD*>(triggerData);
    if(!fmodTriggerData)
    {
        return EAudioRequestStatus::Failure;
    }

    FMOD::Studio::EventDescription* description = nullptr;

    AZStd::string evtURI = AZStd::string::format("event:/%s", fmodTriggerData->eventPath.c_str());
    studioSystem->getEvent(evtURI.c_str(), &description);
    if(!description)
    {
        return EAudioRequestStatus::Failure;
    }

    if(fmodTriggerData->preloadSampleData)
    {
        description->loadSampleData();
    }


    m_preparedEventDescriptions.insert({fmodTriggerData->eventPath, description});
    return EAudioRequestStatus::Success;
}

EAudioRequestStatus AudioSystemImpl_FMOD::UnprepareTriggerSync(IATLAudioObjectData *objectData, const IATLTriggerImplData *triggerData) {
    // TODO: Implement this pure virtual method.
    const auto fmodTriggerData = static_cast<const SATLTriggerImplData_FMOD*>(triggerData);
    if(!fmodTriggerData)
    {
        return EAudioRequestStatus::Failure;
    }

    auto eventPos = m_preparedEventDescriptions.find(fmodTriggerData->eventPath);
    if(eventPos != m_preparedEventDescriptions.end())
    {
        eventPos->second->releaseAllInstances();
        m_preparedEventDescriptions.erase(eventPos);
    }
    else
    {
        return EAudioRequestStatus::Failure;
    }

    return EAudioRequestStatus::Success;
}

EAudioRequestStatus AudioSystemImpl_FMOD::PrepareTriggerAsync(IATLAudioObjectData *objectData, const IATLTriggerImplData *triggerData, IATLEventData *eventData) {
    return EAudioRequestStatus::Failure;
}

EAudioRequestStatus AudioSystemImpl_FMOD::UnprepareTriggerAsync(IATLAudioObjectData *pAudioObjectData, const IATLTriggerImplData *pTriggerData, IATLEventData *pEventData) {
    return EAudioRequestStatus::Failure;
}

EAudioRequestStatus AudioSystemImpl_FMOD::ActivateTrigger(IATLAudioObjectData *objectData, const IATLTriggerImplData *triggerData, IATLEventData *tventData, const SATLSourceData *sourceData) {
    // TODO: Implement this pure virtual method.
    return EAudioRequestStatus::None;
}

EAudioRequestStatus AudioSystemImpl_FMOD::StopEvent(IATLAudioObjectData *objectData, const IATLEventData *eventData) {
    // TODO: Implement this pure virtual method.
    return EAudioRequestStatus::None;
}

EAudioRequestStatus AudioSystemImpl_FMOD::StopAllEvents(IATLAudioObjectData *objectData) {
    // TODO: Implement this pure virtual method.
    return EAudioRequestStatus::None;
}

EAudioRequestStatus AudioSystemImpl_FMOD::SetPosition(IATLAudioObjectData *objectData, const SATLWorldPosition &worldPosition) {
    // TODO: Implement this pure virtual method.
    return EAudioRequestStatus::None;
}

EAudioRequestStatus AudioSystemImpl_FMOD::SetMultiplePositions(IATLAudioObjectData *objectData, const MultiPositionParams &multiPositions) {
    AZ_Error("FMODAudioSystem", false, "Set Multiple Positions in FMOD are not supported!");
    return EAudioRequestStatus::FailureInvalidRequest;
}

EAudioRequestStatus AudioSystemImpl_FMOD::SetRtpc(IATLAudioObjectData *objectData, const IATLRtpcImplData *rtpcData, float value) {
    // TODO: Implement this pure virtual method.
    return EAudioRequestStatus::None;
}

EAudioRequestStatus AudioSystemImpl_FMOD::SetSwitchState(IATLAudioObjectData *objectData, const IATLSwitchStateImplData *switchStateData) {
    // TODO: Implement this pure virtual method.
    return EAudioRequestStatus::None;
}

EAudioRequestStatus AudioSystemImpl_FMOD::SetObstructionOcclusion(IATLAudioObjectData *objectData, float obstruction, float occlusion) {
    AZ_Warning("FMODAudioSystem", false, "Set Obstruction Occlusion is not supported in this FMOD implementation.");
    return EAudioRequestStatus::Success;
}

EAudioRequestStatus AudioSystemImpl_FMOD::SetEnvironment(IATLAudioObjectData *objectData, const IATLEnvironmentImplData *environmentData, float amount) {
    // TODO: Implement this pure virtual method.
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

    studioSystem->setListenerAttributes(ldata->listenerIndex, &listenerAttributes);
    studioSystem->setListenerWeight(ldata->listenerIndex, ldata->weight);

    return EAudioRequestStatus::Success;
}

EAudioRequestStatus AudioSystemImpl_FMOD::ResetRtpc(IATLAudioObjectData *objectData, const IATLRtpcImplData *rtpcData) {
    // TODO: Implement this pure virtual method.
    return EAudioRequestStatus::None;
}

EAudioRequestStatus AudioSystemImpl_FMOD::RegisterInMemoryFile(SATLAudioFileEntryInfo *audioFileEntry) {
    // TODO: Implement this pure virtual method.
    return EAudioRequestStatus::None;
}

EAudioRequestStatus AudioSystemImpl_FMOD::UnregisterInMemoryFile(SATLAudioFileEntryInfo *audioFileEntry) {
    // TODO: Implement this pure virtual method.
    return EAudioRequestStatus::None;

}

EAudioRequestStatus AudioSystemImpl_FMOD::ParseAudioFileEntry(const AZ::rapidxml::xml_node<char> *audioFileEntryNode, SATLAudioFileEntryInfo *fileEntryInfo) {
    // TODO: Implement this pure virtual method.
    return EAudioRequestStatus::None;
}

void AudioSystemImpl_FMOD::DeleteAudioFileEntryData(IATLAudioFileEntryData *oldAudioFileEntryData) {
    // TODO: Implement this pure virtual method.
}

const char * const AudioSystemImpl_FMOD::GetAudioFileLocation(SATLAudioFileEntryInfo *fileEntryInfo) {
    // TODO: Implement this pure virtual method.
    return nullptr;
}

IATLTriggerImplData *AudioSystemImpl_FMOD::NewAudioTriggerImplData(const AZ::rapidxml::xml_node<char> *audioTriggerNode) {
    // TODO: Implement this pure virtual method.
    return nullptr;
}

void AudioSystemImpl_FMOD::DeleteAudioTriggerImplData(IATLTriggerImplData *oldTriggerData) {
    // TODO: Implement this pure virtual method.

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
    // TODO: Implement this pure virtual method.
    return nullptr;
}

IATLAudioObjectData *AudioSystemImpl_FMOD::NewAudioObjectData(TAudioObjectID objectId) {
    // TODO: Implement this pure virtual method.
    return nullptr;
}

void AudioSystemImpl_FMOD::DeleteAudioObjectData(IATLAudioObjectData *oldObjectData) {
    // TODO: Implement this pure virtual method.
}

IATLListenerData *AudioSystemImpl_FMOD::NewDefaultAudioListenerObjectData(TATLIDType objectId) {
    // TODO: Implement this pure virtual method.
    return nullptr;
}

IATLListenerData *AudioSystemImpl_FMOD::NewAudioListenerObjectData(TATLIDType objectId) {
    // TODO: Implement this pure virtual method.
    return nullptr;
}

void AudioSystemImpl_FMOD::DeleteAudioListenerObjectData(IATLListenerData *oldListenerData) {
    // TODO: Implement this pure virtual method.
}

IATLEventData *AudioSystemImpl_FMOD::NewAudioEventData(TAudioEventID eventID) {
    // TODO: Implement this pure virtual method.
    return nullptr;
}

void AudioSystemImpl_FMOD::DeleteAudioEventData(IATLEventData *oldEventData) {
    // TODO: Implement this pure virtual method.
}

void AudioSystemImpl_FMOD::ResetAudioEventData(IATLEventData *eventData) {
    // TODO: Implement this pure virtual method.
}

void AudioSystemImpl_FMOD::SetLanguage(const char *language) {
    // TODO: Implement this pure virtual method.
}

const char * const AudioSystemImpl_FMOD::GetImplSubPath() const {
    return "fmod/";
}

const char * const AudioSystemImpl_FMOD::GetImplementationNameString() const {
    return "FMODStudio";

}

void AudioSystemImpl_FMOD::GetMemoryInfo(SAudioImplMemoryInfo &memoryInfo) const {
    // TODO: Implement this pure virtual method.
}

AZStd::vector<AudioImplMemoryPoolInfo> AudioSystemImpl_FMOD::GetMemoryPoolInfo() {
    // TODO: Implement this pure virtual method.
    return AZStd::vector<AudioImplMemoryPoolInfo>();
}

bool AudioSystemImpl_FMOD::CreateAudioSource(const SAudioInputConfig &sourceConfig) {
    // TODO: Implement this pure virtual method.
    return false;
}

void AudioSystemImpl_FMOD::DestroyAudioSource(TAudioSourceId sourceId) {
    // TODO: Implement this pure virtual method.
}

void AudioSystemImpl_FMOD::SetPanningMode(PanningMode mode) {
    // TODO: Implement this pure virtual method.
}

void AudioSystemImpl_FMOD::OnAudioSystemLoseFocus() {
    // TODO: Implement this pure virtual method.
}

void AudioSystemImpl_FMOD::OnAudioSystemGetFocus() {
    // TODO: Implement this pure virtual method.
}

void AudioSystemImpl_FMOD::OnAudioSystemMuteAll() {
    // TODO: Implement this pure virtual method.
}

void AudioSystemImpl_FMOD::OnAudioSystemUnmuteAll() {
    // TODO: Implement this pure virtual method.
}

void AudioSystemImpl_FMOD::OnAudioSystemRefresh() {
    // TODO: Implement this pure virtual method.
}


}