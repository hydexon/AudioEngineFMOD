#include "Common_FMOD.h"

namespace AudioEngineFMOD
{
    namespace Utils
    {

    FMOD_VECTOR AzToFMODVector(const AZ::Vector3 &vector)
    {
        return { vector.GetX(), vector.GetZ(), vector.GetY() };
    }

    AZ::Vector3 FMODToAzVector(const FMOD_VECTOR &vector)
    {
        return AZ::Vector3(vector.x, vector.z, vector.y);
    }

    FMOD_3D_ATTRIBUTES CreateFMOD3DAttributes(const AZ::Vector3 &pos, const AZ::Vector3 &fw, const AZ::Vector3 &up, const AZ::Vector3 &vel)
    {
        return {
            .position = AzToFMODVector(pos),
            .forward  = AzToFMODVector(fw),
            .up       = AzToFMODVector(up),
            .velocity = AzToFMODVector(vel)
        };
    }




    }
}