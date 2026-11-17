#include "AudioFMODDataLoader.h"

#include <AudioFileUtils.h>
#include "AudioSystemEditor_FMOD.h"
//#include "AudioSystemCtrl_FMOD.h"
#include <AzCore/StringFunc/StringFunc.h>

#include <AzCore/IO/FileIO.h>
#include <AzCore/Utils/Utils.h>
#include <AzCore/JSON/error/en.h>

using namespace AudioControls;

namespace AudioEngineFMOD
{
    CAudioFMODDataLoader::CAudioFMODDataLoader()
    {
    }

    void CAudioFMODDataLoader::Load(CAudioSystemEditor_FMOD* audioSystemImpl)
    {
        m_audioEditorImpl = audioSystemImpl;
        m_path = AZ::IO::FixedMaxPath { m_audioEditorImpl->GetDataPath() / "FMODProjectInfo.json"};

        auto outcome = AZ::Utils::ReadFile(m_path.c_str());
        if(!outcome.IsSuccess())
        {
            AZ_Error("AudioSystemEditor_FMOD", false, "Failed to open FMOD Project file JSON '%s': %s", m_path.c_str(), outcome.GetError().c_str());
            return;
        }

        AZStd::string jsonContent = outcome.TakeValue();
        rapidjson::Document doc;
        doc.Parse<rapidjson::kParseCommentsFlag>(jsonContent.data(), jsonContent.size());

        if(doc.HasParseError())
        {
            AZ_Error("AudioSystemEditor_FMOD", false, "Failed to parse FMOD Project file JSON: %s", rapidjson_ly::GetParseError_En(doc.GetParseError()));
            return;
        }

        if(doc.IsObject())
        {
            const auto& root = doc.GetObject();
            AZ_UNUSED(root);
        }
    }

    void CAudioFMODDataLoader::LoadBanks(rapidjson_ly::Value &value)
    {
        if(!value.IsArray())
            return;
    }

    void CAudioFMODDataLoader::LoadEvents(rapidjson::Value &value)
    {
        if(!value.IsArray())
            return;

    }

    void CAudioFMODDataLoader::LoadSnapshots(rapidjson::Value &value)
    {
        if(!value.IsArray())
            return;

    }

}
