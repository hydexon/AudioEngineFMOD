#pragma once

#include <fmod_studio.hpp>
#include <AzCore/Math/Vector3.h>
#include <AzCore/std/containers/unordered_map.h>
#include <AzCore/std/string/string.h>

namespace AudioEngineFMOD
{
    namespace XMLTags
    {
        static constexpr const char* EventTag = "FMODEvent";
        static constexpr const char* SnapshotTag = "FMODSnapshot";
        static constexpr const char* EventParameterTag = "FMODEventParameter";
        static constexpr const char* SnapshotParameterTag = "FMODSnapshotParameter";
        static constexpr const char* GlobalParameterTag = "FMODGlobalParameter";
        static constexpr const char* BankTag = "FMODBank";
        static constexpr const char* ReturnTag = "FMODBus";

        static constexpr const char* PathAttr = "path";
        static constexpr const char* NameAttr = "name";
        static constexpr const char* TargetAttr = "target";
        static constexpr const char* LocalizedAttr = "localized";
        static constexpr const char* ValueAttr = "value";
        static constexpr const char* MultAttr = "multiplier";
        static constexpr const char* ShiftAttr = "shift";
        static constexpr const char* EventTypeAttr = "eventType";

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


    enum class FMODEventType {
        Play = 0,
        Stop
    };

    namespace Utils
    {

        FMODEventType GetActionFromXmlStr(const AZStd::string_view source);
        AZStd::string GetXmlStrFromAction(const FMODEventType action);

        FMOD_VECTOR AzToFMODVector(const AZ::Vector3& vector);
        AZ::Vector3 FMODToAzVector(const FMOD_VECTOR& vector);
        FMOD_3D_ATTRIBUTES CreateFMOD3DAttributes(const AZ::Vector3& pos,
                                                  const AZ::Vector3& fw,
                                                  const AZ::Vector3& up,
                                                  const AZ::Vector3& vel = AZ::Vector3::CreateOne());

    }

    typedef AZStd::fixed_string<256> FMODStringPath;
    typedef AZStd::unordered_map<AZ::u32, FMOD::Studio::EventDescription*> FMODPathIdToEventDescriptionMap;

    inline FMODPathIdToEventDescriptionMap g_PathIdToEventDescription;

}