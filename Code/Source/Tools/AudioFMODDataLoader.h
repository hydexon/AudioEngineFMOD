#pragma once

#include <ACETypes.h>
#include <AzCore/std/string/string_view.h>
#include <AzCore/std/containers/unordered_map.h>
#include <IAudioSystemControl.h>
#include <AzCore/IO/Path/Path.h>
#include <AzCore/std/smart_ptr/unique_ptr.h>
#include <AzCore/JSON/document.h>

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

        void LoadItems(rapidjson::Value& value, AudioControls::IAudioSystemControl* parentCtrl, EFMODStudioControlTypes itemType);
        void LoadSimpleItems(rapidjson::Value& value, AudioControls::IAudioSystemControl* parent, EFMODStudioControlTypes itemType);


    };
}