#pragma once

#include <ACETypes.h>
#include <AzCore/std/string/string_view.h>

namespace AudioEngineFMOD
{
    class CAudioSystemEditor_FMOD;

    class CAudioFMODDataLoader
    {
    public:
        CAudioFMODDataLoader() = default;
        void Load(CAudioSystemEditor_FMOD* audioSystemImpl);
    private:
        void LoadControlsForEvents(const AZStd::string_view infoPath); //TODO: This might be temporal.
        CAudioSystemEditor_FMOD* m_audioSystemImpl = nullptr;
    };
}