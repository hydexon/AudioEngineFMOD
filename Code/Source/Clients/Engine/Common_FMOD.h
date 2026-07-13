#pragma once

#include <fmod_studio.hpp>
#include <AzCore/Math/Vector3.h>
#include <AzCore/std/containers/unordered_map.h>

namespace AudioEngineFMOD
{
    typedef AZStd::unordered_map<FMOD_GUID, FMOD::Studio::EventInstance*> InstancesMultiMap;

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