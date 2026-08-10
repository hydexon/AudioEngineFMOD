#include "AudioFMODDataLoader.h"

#include <IAudioSystemControl.h>
#include "AudioSystemEditor_FMOD.h"
#include "AudioSystemCtrl_FMOD.h"
#include <AzCore/StringFunc/StringFunc.h>

#include <AzCore/IO/FileIO.h>
#include <AzCore/JSON/document.h>
#include <AzCore/std/string/string.h>

namespace AudioEngineFMOD
{

void CAudioFMODDataLoader::Load(CAudioSystemEditor_FMOD *audioSystemImpl, AudioControls::IAudioSystemControl *parent, AudioControls::IAudioSystemControl *locParent)
{
    m_audioSystemImpl = audioSystemImpl;
    const AZ::IO::FixedMaxPath RootFMODPath { m_audioSystemImpl->GetDataPath() };
    LoadControlsForEvents(AZ::IO::FixedMaxPath { RootFMODPath / "FMODProjectInfo.json" }.Native(), parent, locParent);
}

AZStd::string_view CAudioFMODDataLoader::GetEventGUID(AZStd::string_view eventPath)
{
    auto entry = m_eventGUIDs.find(eventPath);
    if(entry != m_eventGUIDs.end())
    {
        return entry->second;
    }

    return AZStd::string_view();
}

const AZStd::unordered_map<AZStd::string, float> &CAudioFMODDataLoader::GetEventParameters() const
{
    return m_eventParameters;
}

void CAudioFMODDataLoader::LoadControlsForEvents(const AZStd::string_view infoPath, AudioControls::IAudioSystemControl *parent, AudioControls::IAudioSystemControl *locParent)
{
    bool failedOpenFile = false;
    m_eventGUIDs.clear();

    AZ_Info("FMODAudioSystem", "Trying to read: %s", infoPath.data());
    if(AZ::IO::FileIOBase::GetInstance()->Exists(infoPath.data()))
    {
        auto fileIO = AZ::IO::FileIOBase::GetInstance();
        AZ::u64 jsonSize = 0;
        fileIO->Size(infoPath.data(), jsonSize);

        AZStd::vector<char> buffer(jsonSize);
        AZ::IO::HandleType handle;

        if(fileIO->Open(infoPath.data(), AZ::IO::OpenMode::ModeRead | AZ::IO::OpenMode::ModeText, handle))
        {
            if(fileIO->Read(handle, buffer.data(), jsonSize))
            {
                rapidjson::Document doc;
                doc.Parse(buffer.data());

                if(doc.IsObject())
                {
                    const auto& root = doc.GetObject();
                    const auto& eventArr = root["events"].GetArray();

                    for(const auto& event : eventArr)
                    {
                        auto evtObj = event.GetObject();
                        auto path = evtObj["path"].GetString();
                        m_audioSystemImpl->CreateControl(AudioControls::SControlDef(path, eFMOD_EVENT, false, &m_audioSystemImpl->m_eventsCtrl, "Events"));
                        m_eventGUIDs[path] = evtObj["id"].GetString();
                    }

                    /*
                    const auto& buses = root["buses"].GetArray();
                    for(const auto& bus : buses)
                    {
                        m_audioSystemImpl->CreateControl(AudioControls::SControlDef(bus.GetString(), eFMOD_AUXBUS));
                    }
                    */
                    const auto& snapshotsArr = root["snapshots"].GetArray();
                    for(const auto& snapshot : snapshotsArr)
                    {
                        auto snapObj = snapshot.GetObject();
                        auto path = snapObj["path"].GetString();
                        m_audioSystemImpl->CreateControl(AudioControls::SControlDef(snapObj["path"].GetString(), eFMOD_SNAPSHOT, false, &m_audioSystemImpl->m_eventsCtrl, "Snapshots"));
                        m_eventGUIDs[path] = snapObj["id"].GetString();
                    }
                    //TODO: VCAs

                    const auto& banks = root["banks"].GetArray();
                    for(const auto& bnk : banks)
                    {
                        const auto& bank = bnk.GetObject();
                        if (AZ::StringFunc::Equal(bank["name"].GetString(), "Master")) //Skip the master bank, it always will be loaded internally.
                        {
                            continue;
                        }

                        const auto bankNameWithExt = AZStd::string::format("%s.bank", bank["name"].GetString());
                        const bool isLocalized = bank["isLocalized"].GetBool();
                        m_audioSystemImpl->CreateControl(
                                    AudioControls::SControlDef(
                                        bankNameWithExt.c_str(),
                                        eFMOD_SOUNDBANK,
                                        isLocalized,
                                        isLocalized ? locParent : &m_audioSystemImpl->m_banksCtrl,
                                        "Banks")
                                    );
                    }

                    const auto& params = root["parameters"].GetArray();
                    for(const auto& param : params)
                    {
                        const auto& obj = param.GetObject();
                        const AZStd::string paramName = obj["path"].GetString();

                        if(!obj["isGlobal"].GetBool())
                        {
                            m_eventParameters.emplace(paramName, obj["initialValue"].GetFloat());
                        }

                        AudioControls::IAudioSystemControl* group = m_audioSystemImpl->CreateControl(AudioControls::SControlDef(paramName,eFMOD_PARAMETER, false, &m_audioSystemImpl->m_paramsCtrl, "Parameters"));
                        if(obj["labels"].IsArray())
                        {
                            auto labelArray = obj["labels"].GetArray();
                            for(const auto& label : labelArray)
                            {
                                m_audioSystemImpl->CreateControl(AudioControls::SControlDef(label.GetString(), eFMOD_LABELVALUE, false, group));
                                m_parameterLabels[paramName].push_back(label.GetString());
                            }
                        }



                    }


                }
            }
            fileIO->Close(handle);
        } else { failedOpenFile = true; }
    } else { failedOpenFile = true; }

    if(failedOpenFile)
    {
        AZ_Error("FMODAudioSystem", false, "Error trying to open FMODProjectInfo.json at: %s", infoPath.data());
    }
}


}