#pragma once

#include <AzCore/RTTI/ReflectContext.h>
#include <AzCore/std/string/string.h>

namespace AudioEngineFMOD
{
    namespace Constants
    {
        static constexpr const char DefaultFMODRootPath[] = "Assets/Audio/FMOD/";
        static constexpr const char DefaultFMODBanksPath[] = "Assets/Audio/FMOD/Banks/";
        static constexpr const char LocaleConfigFile[] = "FMODLocaleConfig.json";
        static constexpr const char BankExtension[] = ".bank";
        static constexpr const char MasterBank[] = "Master.bank";
        static constexpr const char MasterStringsBank[] = "Master.strings.bank";
    }

    const AZStd::string_view GetFMODBanksRootPath();
    void SetBanksRootPath(const AZStd::string_view path);

    struct FMODLocaleConfig
    {
        AZ_TYPE_INFO(FMODLocaleConfig, "{A74FC07B-3D76-43FC-B25A-8FD9AB293926}");
        AZ_CLASS_ALLOCATOR(FMODLocaleConfig, AZ::SystemAllocator);

        struct LocalizationMapping
        {
            AZ_TYPE_INFO(LocalizationMapping, "{7FAD0A3C-B8E3-49B8-82AC-17A1BF93036A}");
            AZ_CLASS_ALLOCATOR(LocalizationMapping, AZ::SystemAllocator);

            AZStd::string m_languageName; //O3DE Language Name: English, Spanish, Japanese, Chinese.
            AZStd::string m_localeCode; //FMOD Locale Code: EN, ESP, JP, CN
        };

        FMODLocaleConfig() = default;
        ~FMODLocaleConfig() = default;

        static void Reflect(AZ::ReflectContext* context);

        bool Load(const AZStd::string& filePath);
        bool Save(const AZStd::string& filePath);

        //Serialized Data:
        AZStd::vector<LocalizationMapping> m_localeMappings;
    };
}