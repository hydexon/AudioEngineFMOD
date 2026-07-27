#include "AudioFMODDataLoader.h"

#include <IAudioSystemControl.h>
#include "AudioSystemEditor_FMOD.h"
#include "AudioSystemCtrl_FMOD.h"

#include <AzCore/IO/FileIO.h>
#include <AzCore/JSON/document.h>
#include <AzCore/std/string/string.h>

namespace AudioEngineFMOD
{

void CAudioFMODDataLoader::Load(CAudioSystemEditor_FMOD *audioSystemImpl)
{
    m_audioSystemImpl = audioSystemImpl;
    const AZ::IO::FixedMaxPath RootFMODPath { m_audioSystemImpl->GetDataPath() };
    LoadControlsForEvents(AZ::IO::FixedMaxPath { RootFMODPath / "FMODProjectInfo.json" }.Native());
}

void CAudioFMODDataLoader::LoadControlsForEvents(const AZStd::string_view infoPath)
{
    bool failedOpenFile = false;

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
                        m_audioSystemImpl->CreateControl(AudioControls::SControlDef(event.GetString(), eFMOD_EVENT));
                    }

                    const auto& buses = root["buses"].GetArray();
                    for(const auto& bus : buses)
                    {
                        m_audioSystemImpl->CreateControl(AudioControls::SControlDef(bus.GetString(), eFMOD_AUXBUS));
                    }

                    const auto& snapshotsArr = root["snapshots"].GetArray();
                    for(const auto& snapshot : snapshotsArr)
                    {
                        m_audioSystemImpl->CreateControl(AudioControls::SControlDef(snapshot.GetString(), eFMOD_SNAPSHOT));
                    }

                    const auto& banks = root["banks"].GetArray();
                    for(const auto& bnk : banks)
                    {
                        const auto& bank = bnk.GetObject();
                        m_audioSystemImpl->CreateControl(
                                    AudioControls::SControlDef(
                                        bank["name"].GetString(),
                                        eFMOD_SOUNDBANK,
                                        bank["isLocalized"].GetBool()
                                        )
                                    );
                    }

                    const auto& params = root["parameters"].GetArray();
                    for(const auto& param : params)
                    {
                        const auto& obj = param.GetObject();
                        m_audioSystemImpl->CreateControl(AudioControls::SControlDef(obj["path"].GetString(),eFMOD_PARAMETER));
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