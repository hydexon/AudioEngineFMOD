#pragma once

#include <fmod_studio.hpp>
#include <AzCore/Math/Vector3.h>
#include <AzCore/std/containers/unordered_map.h>
#include <AzCore/std/string/string.h>

namespace AudioEngineFMOD
{
    namespace XMLTags
    {
        static constexpr const char* FMODEventTag = "FMODEvent";
        static constexpr const char* FMODParameterTag = "FMODParameter";
        static constexpr const char* FMODStudioBankTag = "FMODBankFile";
        static constexpr const char* FMODFileTag = "FMODAudioFile";

        static constexpr const char* FMODSnapshotAttr = "IsSnapshot";
        static constexpr const char* FMODPathAttribute = "Path";
        static constexpr const char* FMODLocalizedAttribute = "Localized";
        static constexpr const char* FMODSamplePreloadAttr = "loadSampleData";
        static constexpr const char* FMODStopMode = "StopMode";
        static constexpr const char* FMODEvtAction = "EventAction";
        static constexpr const char* FMODParamAttr = "Parameter";
        static constexpr const char* FMODPerObjectAttr = "PerObject";
    }

    struct FMOD_GUID_CmpEqual {
        bool operator()(const FMOD_GUID& lhs, const FMOD_GUID& rhs) const {
            return memcmp(&lhs, &rhs, sizeof(FMOD_GUID)) == 0;
        }
    };

    struct FMOD_GUID_Hashing {
        AZStd::size_t operator()(const FMOD_GUID& guid) const noexcept {
            const auto* ptr  = reinterpret_cast<const char*>(&guid);
            //Simple hash combination (Should use FNV-1a instead)?
#if 1
            size_t hash = 0;
            for(AZStd::size_t i = 0; i < sizeof(FMOD_GUID); ++i)
            {
                hash = hash * 31 + ptr[i];
            }
#else
            AZStd::size_t hash = 14695981039346656037ULL;
            for(size_t i = 0; i < sizeof(FMOD_GUID); ++i)
            {
                hash ^= static_cast<size_t>(ptr[i]);
                hash *= 1099511628211ULL;
            }
#endif
            return hash;
        }
    };


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