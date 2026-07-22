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

        static constexpr const char* FMODStudioEventPathAttribute = "path";
        static constexpr const char* FMODStudioEventSamplePreloadAttr = "preload_sampledata";
    }

    namespace Utils
    {
        FMOD_VECTOR AzToFMODVector(const AZ::Vector3& vector);
        AZ::Vector3 FMODToAzVector(const FMOD_VECTOR& vector);
        FMOD_3D_ATTRIBUTES CreateFMOD3DAttributes(const AZ::Vector3& pos,
                                                  const AZ::Vector3& fw,
                                                  const AZ::Vector3& up,
                                                  const AZ::Vector3& vel = AZ::Vector3::CreateOne());

    }

}