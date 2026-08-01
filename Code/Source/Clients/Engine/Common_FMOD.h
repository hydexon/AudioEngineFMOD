#pragma once

#include <fmod_studio.hpp>
#include <AzCore/Math/Vector3.h>
#include <AzCore/std/containers/unordered_map.h>
#include <AzCore/std/string/string.h>

namespace AudioEngineFMOD
{
    namespace XMLTags
    {
        static constexpr const char* FMODEventTag = "FMODStudioEvent";
        static constexpr const char* FMODParameterTag = "FMODStudioParameter";
        static constexpr const char* FMODStudioBankTag = "FMODStudioBankFile";
        static constexpr const char* FMODFileTag = "FMODAudioFile";

        static constexpr const char* FMODPathAttribute = "path";
        static constexpr const char* FMODLocalizedAttribute = "localized";
        static constexpr const char* FMODSamplePreloadAttr = "loadSampleData";
        static constexpr const char* FMODStopMode = "stopMode";
        static constexpr const char* FMODEvtAction = "eventAction";
    }

    enum class FMODEventAction {
        Play = 0,
        Pause,
        Stop,
        Resume,
        Unknown
    };

    namespace Utils
    {
        FMODEventAction GetActionFromXmlStr(const AZStd::string_view source);
        AZStd::string GetXmlStrFromAction(const FMODEventAction action);

        FMOD_VECTOR AzToFMODVector(const AZ::Vector3& vector);
        AZ::Vector3 FMODToAzVector(const FMOD_VECTOR& vector);
        FMOD_3D_ATTRIBUTES CreateFMOD3DAttributes(const AZ::Vector3& pos,
                                                  const AZ::Vector3& fw,
                                                  const AZ::Vector3& up,
                                                  const AZ::Vector3& vel = AZ::Vector3::CreateOne());

    }

}