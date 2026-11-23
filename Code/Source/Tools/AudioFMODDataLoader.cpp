#include "AudioFMODDataLoader.h"

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
            LoadSimpleItems(root["banks"], &m_audioEditorImpl->m_banksControl, eFMOD_SOUNDBANK);
            LoadItems(root["events"], &m_audioEditorImpl->m_eventsControl, eFMOD_EVENT);
            LoadItems(root["snapshots"], &m_audioEditorImpl->m_snapshotsCtrl, eFMOD_SNAPSHOT);
            LoadItems(root["parameters"], &m_audioEditorImpl->m_globalParamsCtrl, eFMOD_GLOBAL_PARAMETER);
            LoadSimpleItems(root["returns"], &m_audioEditorImpl->m_returnsControl, eFMOD_RETURN);
        }
    }

    void CAudioFMODDataLoader::LoadItems(rapidjson::Value& value, AudioControls::IAudioSystemControl* parentCtrl,
        EFMODStudioControlTypes itemType)
    {
        if (!value.IsArray())
            return;

        for (auto& item : value.GetArray())
        {
            const auto& obj = item.GetObject();

            if (const auto jtype = obj["type"].GetString(); AZ::StringFunc::Equal(jtype, "Folder"))
            {
                AZ_Info("AudioSystemEditor_FMOD", "Iterating Folder: '%s'", obj["name"].GetString());
                auto ctrlDef = AudioControls::SControlDef(obj["name"].GetString(), eFMOD_FOLDER, false, parentCtrl);
                auto folderCtrl = m_audioEditorImpl->CreateControl(ctrlDef);
                LoadItems(obj["items"], folderCtrl, itemType);
            }
            else
            {
                auto ctrlDef = AudioControls::SControlDef(obj["name"].GetString(), itemType, false, parentCtrl, AZStd::string(obj["path"].GetString()));
                auto itemCtrl = m_audioEditorImpl->CreateControl(ctrlDef);
                if (itemType == eFMOD_EVENT || itemType == eFMOD_SNAPSHOT)
                {
                    if (obj.HasMember("parameters"))
                        LoadSimpleItems(obj["parameters"], itemCtrl, itemType == eFMOD_EVENT ? eFMOD_EVENT_PARAMETER : eFMOD_SNAPSHOT_PARAMETER);
                }
            }
        }
    }

    void CAudioFMODDataLoader::LoadSimpleItems(rapidjson::Value& value,
        AudioControls::IAudioSystemControl* parent, EFMODStudioControlTypes itemType)
    {
        if (!value.IsArray())
            return;

        for (const auto& item : value.GetArray())
        {
            const auto& obj = item.GetObject();
            bool isLocalized = obj.HasMember("isLocalized") ? obj["isLocalized"].GetBool() : false;
            auto ctrldef = AudioControls::SControlDef(obj["name"].GetString(), itemType, isLocalized, parent, obj.HasMember("path") ? AZStd::string(obj["path"].GetString()) : "");
            m_audioEditorImpl->CreateControl(ctrldef);
        }
    }
}
