#pragma once

#include <ACETypes.h>
#include <AzCore/std/string/string_view.h>
#include <AzCore/std/containers/unordered_map.h>
#include <IAudioSystemControl.h>
#include <AzCore/IO/Path/Path.h>
#include <AzCore/XML/rapidxml.h>

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
        void Load(CAudioSystemEditor_FMOD* audioSystemImpl);
    private:
        friend class CAudioSystemEditor_FMOD;

        CAudioSystemEditor_FMOD* m_audioSystemImpl = nullptr;
        AZ::IO::FixedMaxPath m_path;

        void ScanFolder(const AZ::IO::FixedMaxPath& folderPath);
        void ParseFile(const AZ::IO::FixedMaxPath& filePath);

        AudioControls::IAudioSystemControl* LoadFolder(const AZ::rapidxml::xml_node<>* node);
        AudioControls::IAudioSystemControl* LoadEvent(const AZ::rapidxml::xml_node<>* node);
        AudioControls::IAudioSystemControl* LoadSnapshot(const AZ::rapidxml::xml_node<>* node);
        AudioControls::IAudioSystemControl* LoadGroup(const AZ::rapidxml::xml_node<>* node);
        AudioControls::IAudioSystemControl* LoadReturn(const AZ::rapidxml::xml_node<>* node);
        AudioControls::IAudioSystemControl* LoadParameter(const AZ::rapidxml::xml_node<>* node,
                                                          AudioControls::IAudioSystemControl* ctrl, int fmodCtrlType);

    };
}