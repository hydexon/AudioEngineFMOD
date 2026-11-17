#include "AudioFMODDataLoader.h"

#include <AudioFileUtils.h>
#include "AudioSystemEditor_FMOD.h"
#include "AudioSystemCtrl_FMOD.h"
#include <AzCore/StringFunc/StringFunc.h>

#include <AzCore/IO/FileIO.h>
#include "AudioFileUtils.h"
#include "AudioConnections.h"
#include <AzCore/JSON/rapidjson.h>

using namespace AudioControls;

namespace AudioEngineFMOD
{
    CAudioFMODDataLoader::CAudioFMODDataLoader()
    {
    }

    void CAudioFMODDataLoader::Load(CAudioSystemEditor_FMOD* audioSystemImpl)
    {
        m_audioEditorImpl = audioSystemImpl;
        m_path = m_audioEditorImpl->GetDataPath();
    }

}
