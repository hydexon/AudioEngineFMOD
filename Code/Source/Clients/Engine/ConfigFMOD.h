#pragma once

#include <AzCore/RTTI/ReflectContext.h>
#include <AzCore/std/string/string.h>

namespace AudioEngineFMOD
{
    namespace Constants
    {
        static constexpr const char DefaultFMODBanksPath[] = "Assets/Audio/FMOD/Banks/";
        static constexpr const char ConfigFile[] = "FMOD_Config.json";
        static constexpr const char BankExtensionp[] = ".bank";
        static constexpr const char MasterBank[] = "Master.bank";
        static constexpr const char MasterStringsBank[] = "Master.strings.bank";
    }

    const AZStd::string_view GetFMODBanksRootPath();
    void SetBanksRootPath(const AZStd::string_view path);

#if 0
    struct FMODConfigurationSettings
    {
        AZ_TYPE_INFO(FMODConfigurationSettings, "{A74FC07B-3D76-43FC-B25A-8FD9AB293926}");
        AZ_CLASS_ALLOCATOR(FMODConfigurationSettings, AZ::SystemAllocator);

        struct PlatformMapping
        {
            AZ_TYPE_INFO(PlatformMapping, "{7FAD0A3C-B8E3-49B8-82AC-17A1BF93036A}");
            AZ_CLASS_ALLOCATOR(PlatformMapping, AZ::SystemAllocator);

            AZStd::string m_assetPlatform; //O3DE Asset Platform Name (i.e: pc, linux, android,...)
            AZStd::string m_enginePlatform; //O3DE Engine Platform.
            AZStd::string m_FMODPlatform; //FMOD Studio Platform names(i.e: Desktop, Mobile, Android, iOS, Switch)
        };

        FMODConfigurationSettings() = default;
        ~FMODConfigurationSettings() = default;

        static void Reflect(AZ::ReflectContext* context);

        bool Load(const AZStd::string& filePath);
        bool Save(const AZStd::string& filePath);

        //Serialized Data:
        AZStd::vector<PlatformMapping> m_platformMappings;
    };
#endif
}