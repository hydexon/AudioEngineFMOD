#pragma once

#include <ACETypes.h>
#include <AzCore/std/string/string_view.h>

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
        const AZStd::vector<AZStd::string>& GetEventParameters();
    private:
        void LoadControlsForEvents(const AZStd::string_view infoPath, AudioControls::IAudioSystemControl* parent, AudioControls::IAudioSystemControl* locParent); //TODO: This might be temporal.
        CAudioSystemEditor_FMOD* m_audioSystemImpl = nullptr;
        AZStd::vector<AZStd::string> m_eventParameters;
    };
}