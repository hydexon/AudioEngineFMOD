#include "ConfigFMOD.h"

#include <AzCore/Component/ComponentApplicationBus.h>
#include <AzCore/Serialization/Json/JsonSerialization.h>
#include <AzCore/Serialization/Json/JsonUtils.h>
#include <AzCore/Serialization/SerializeContext.h>

namespace AudioEngineFMOD
{
    static AZStd::string_view s_configuredBankPath = Constants::DefaultFMODBanksPath;

    void SetBanksRootPath(const AZStd::string_view path)
    {
        s_configuredBankPath = path;
    }

    const AZStd::string_view GetFMODBanksRootPath()
    {
        return s_configuredBankPath;
    }

#if 0
    void FMODLocaleConfig::Reflect(AZ::ReflectContext* context)
    {
        if(auto serializeContext = azrtti_cast<AZ::SerializeContext*>(context))
        {
            serializeContext->Class<LocalizationMapping>()
                    ->Version(1)
                    ->Field("LanguageName", &LocalizationMapping::m_languageName)
                    ->Field("LocaleCode", &LocalizationMapping::m_localeCode);

            serializeContext->Class<FMODLocaleConfig>()
                    ->Version(1)
                    ->Field("FMODLocaleMaps", &FMODLocaleConfig::m_localeMappings);
        }
    }

    bool FMODLocaleConfig::Load(const AZStd::string& filePath)
    {
        AZ::IO::Path jsonPath(filePath);
        auto outcome = AZ::JsonSerializationUtils::ReadJsonFile(jsonPath.Native());
        if(!outcome)
        {
            AZ_Printf("FMODAudioSystem", "ERROR: %s\n", outcome.GetError().c_str());
            return false;
        }

        m_localeMappings.clear();
        AZ::JsonDeserializerSettings deserializeSettings;
        AZ::ComponentApplicationBus::BroadcastResult(
                    deserializeSettings.m_serializeContext,
                    &AZ::ComponentApplicationBus::Events::GetSerializeContext);

        auto result = AZ::JsonSerialization::Load(*this, outcome.GetValue(), deserializeSettings);
        if(result.GetProcessing() != AZ::JsonSerializationResult::Processing::Completed)
        {
            AZ_Printf("FMODAudioSystem", "ERROR: Deserializing JSON File:%s\n", filePath.c_str());
            return false;
        }

        AZ_Printf("FMODAudioSystem", "Loaded '%s' successfully", filePath.c_str());
        return true;
    }

    bool FMODLocaleConfig::Save(const AZStd::string& filePath)
    {
        AZ::JsonSerializerSettings serializeSettings;
        AZ::ComponentApplicationBus::BroadcastResult(
                    serializeSettings.m_serializeContext,
                    &AZ::ComponentApplicationBus::Events::GetSerializeContext);

        rapidjson::Document doc;
        auto result = AZ::JsonSerialization::Store(doc, doc.GetAllocator(), *this, serializeSettings);
        if(result.GetProcessing() != AZ::JsonSerializationResult::Processing::Completed)
        {
            AZ_Printf("FMODAudioSystem", "ERROR: Serializing JSON file '%s'\n", filePath.c_str());
            return false;
        }

        auto outcome = AZ::JsonSerializationUtils::WriteJsonFile(doc, filePath);
        if(!outcome)
        {
            AZ_Printf("FMODAudioSystem", "ERROR: %s\n", outcome.GetError().c_str());
            return false;
        }

        AZ_Printf("FMODAudioSystem","Saved '%s' successfully.\n", filePath.c_str());
        return true;
    }
#endif
}