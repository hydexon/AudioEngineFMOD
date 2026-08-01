#include "Common_FMOD.h"

#include <AzCore/StringFunc/StringFunc.h>


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
            .velocity = AzToFMODVector(vel),
            .forward  = AzToFMODVector(fw),
            .up       = AzToFMODVector(up),
        };
    }

    FMODEventAction GetActionFromXmlStr(const AZStd::string_view source)
    {
        FMODEventAction result = FMODEventAction::Unknown;

        if(AZ::StringFunc::Equal(source, "play"))        result = FMODEventAction::Play;
        else if(AZ::StringFunc::Equal(source, "pause"))  result = FMODEventAction::Pause;
        else if(AZ::StringFunc::Equal(source, "stop"))   result = FMODEventAction::Stop;
        else if(AZ::StringFunc::Equal(source, "resume")) result = FMODEventAction::Resume;

        return result;
    }

    AZStd::string GetXmlStrFromAction(const FMODEventAction action)
    {
        switch(action)
        {
        case FMODEventAction::Play:     return "play";
        case FMODEventAction::Pause:    return "pause";
        case FMODEventAction::Resume:   return "resume";
        case FMODEventAction::Stop:     return "stop";
        default:                        return "unknown";
        }
    }




    }
}