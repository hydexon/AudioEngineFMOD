#pragma once

#include <ACETypes.h>
#include <AzCore/std/string/string_view.h>
#include <AzCore/std/containers/unordered_map.h>

namespace AudioControls
{
    class IAudioSystemControl;
}

namespace AudioEngineFMOD
{
    class CAudioSystemEditor_FMOD;

    class CAudioFMODDataLoader
    {
    public:
        void Load(CAudioSystemEditor_FMOD* audioSystemImpl, AudioControls::IAudioSystemControl* parent, AudioControls::IAudioSystemControl* locParent);
        const AZStd::unordered_map<AZStd::string, float>& GetEventParameters() const;
    private:
        void LoadControlsForEvents(const AZStd::string_view infoPath, AudioControls::IAudioSystemControl* parent, AudioControls::IAudioSystemControl* locParent); //TODO: This might be temporal.
        CAudioSystemEditor_FMOD* m_audioSystemImpl = nullptr;
        //AZStd::vector<AZStd::string> m_eventParameters;
        AZStd::unordered_map<AZStd::string, float> m_eventParameters;
        AZStd::unordered_map<AZStd::string, std::vector<AZStd::string>> m_parameterLabels;
    };
}