#pragma once

#include <ACETypes.h>
#include <AzCore/std/string/string_view.h>
#include <AzCore/std/containers/unordered_map.h>
#include <IAudioSystemControl.h>
#include <AzCore/IO/Path/Path.h>
#include <AzCore/XML/rapidxml.h>
#include <AzCore/std/smart_ptr/unique_ptr.h>

#include "AudioSystemCtrl_FMOD.h"

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
        CAudioFMODDataLoader();
        ~CAudioFMODDataLoader() = default;

        void Load(CAudioSystemEditor_FMOD* audioSystemImpl);
    private:
        friend class CAudioSystemEditor_FMOD;

        CAudioSystemEditor_FMOD* m_audioEditorImpl = nullptr;
        AZ::IO::FixedMaxPath m_path;
    };
}