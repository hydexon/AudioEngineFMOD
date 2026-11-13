#include "AudioFMODDataLoader.h"

#include <AudioFileUtils.h>
#include "AudioSystemEditor_FMOD.h"
#include "AudioSystemCtrl_FMOD.h"
#include <AzCore/StringFunc/StringFunc.h>

#include <AzCore/IO/FileIO.h>
#include "AudioFileUtils.h"

using namespace AudioControls;

namespace AudioEngineFMOD
{

void CAudioFMODDataLoader::Load(CAudioSystemEditor_FMOD *audioSystemImpl)
{
    m_audioSystemImpl = audioSystemImpl;
    m_path = m_audioSystemImpl->GetDataPath();

    ScanFolder(AZ::IO::FixedMaxPath { m_path / "Metadata" / "EventFolder" });
    ScanFolder(AZ::IO::FixedMaxPath { m_path / "Metadata" / "Group"});
    ScanFolder(AZ::IO::FixedMaxPath { m_path / "Metadata" / "Event"});
    ScanFolder(AZ::IO::FixedMaxPath { m_path / "Metadata" / "Return"});
}

void CAudioFMODDataLoader::ScanFolder(const AZ::IO::FixedMaxPath& folderPath)
{
    auto foundFiles = Audio::FindFilesInPath(folderPath.Native(), "*.xml");
    for (const auto& foundPath : foundFiles)
    {
        if (AZ::IO::FileIOBase::GetInstance()->IsDirectory(foundPath.c_str()))
            continue;

        ParseFile(foundPath);
    }
}

void CAudioFMODDataLoader::ParseFile(const AZ::IO::FixedMaxPath& filePath)
{
    auto fileIO = AZ::IO::FileIOBase::GetInstance();
    if (!fileIO->Exists(filePath.c_str()))
        return;

    Audio::ScopedXmlLoader xmlLoader (filePath.Native());
    if (xmlLoader.HasError())
    {
        AZ_Warning("AudioFMODDataLoader", false, "Unable to load the xml file '%s'", filePath.c_str());
        return;
    }

    auto rootNode = xmlLoader.GetRootNode();
    AudioControls::IAudioSystemControl* ctrl = nullptr;
    for (const AZ::rapidxml::xml_node<>* child = rootNode->first_node(); child; child = rootNode->next_sibling())
    {
        const auto classAttr = child->first_attribute("class");
        if (!classAttr)
            continue;

        if (AZ::StringFunc::Equal(classAttr->value(), "EventFolder"))
        {
            ctrl = LoadFolder(child);
        }
        else if(AZ::StringFunc::Equal(classAttr->value(), "Event"))
        {
            ctrl = LoadEvent(child);
        }
        else if (AZ::StringFunc::Equal(classAttr->value(), "Snapshot"))
        {
            ctrl = LoadSnapshot(child);
        }
        else if (AZ::StringFunc::Equal(classAttr->value(), "GameParameter"))
        {
            if (ctrl)
            {

            }
        }
        else if (AZ::StringFunc::Equal(classAttr->value(), "MixerReturn"))
        {
            ctrl = LoadReturn(child);
        }
        else if (AZ::StringFunc::Equal(classAttr->value(), "MixerGroup"))
        {
            ctrl = LoadGroup(child);
        }
    }


}

AudioControls::IAudioSystemControl* CAudioFMODDataLoader::LoadFolder(const AZ::rapidxml::xml_node<>* node)
{
    return nullptr;
}

AudioControls::IAudioSystemControl* CAudioFMODDataLoader::LoadEvent(const AZ::rapidxml::xml_node<>* node)
{
    return nullptr;
}

AudioControls::IAudioSystemControl* CAudioFMODDataLoader::LoadSnapshot(const AZ::rapidxml::xml_node<>* node)
{
    return nullptr;
}

AudioControls::IAudioSystemControl* CAudioFMODDataLoader::LoadGroup(const AZ::rapidxml::xml_node<>* node)
{
    return nullptr;
}

AudioControls::IAudioSystemControl* CAudioFMODDataLoader::LoadReturn(const AZ::rapidxml::xml_node<>* node)
{
    return nullptr;
}

AudioControls::IAudioSystemControl* CAudioFMODDataLoader::LoadParameter(const AZ::rapidxml::xml_node<>* node,
    AudioControls::IAudioSystemControl* ctrl, int fmodCtrlType)
{
    return nullptr;
}
}
