
#pragma once

#include <AudioEngineFMOD/AudioEngineFMODTypeIds.h>

#include <AzCore/EBus/EBus.h>
#include <AzCore/Interface/Interface.h>

namespace AudioEngineFMOD
{
    class AudioEngineFMODRequests
    {
    public:
        AZ_RTTI(AudioEngineFMODRequests, AudioEngineFMODRequestsTypeId);
        virtual ~AudioEngineFMODRequests() = default;
        // Put your public methods here
    };

    class AudioEngineFMODBusTraits
        : public AZ::EBusTraits
    {
    public:
        //////////////////////////////////////////////////////////////////////////
        // EBusTraits overrides
        static constexpr AZ::EBusHandlerPolicy HandlerPolicy = AZ::EBusHandlerPolicy::Single;
        static constexpr AZ::EBusAddressPolicy AddressPolicy = AZ::EBusAddressPolicy::Single;
        //////////////////////////////////////////////////////////////////////////
    };

    using AudioEngineFMODRequestBus = AZ::EBus<AudioEngineFMODRequests, AudioEngineFMODBusTraits>;
    using AudioEngineFMODInterface = AZ::Interface<AudioEngineFMODRequests>;

} // namespace AudioEngineFMOD
