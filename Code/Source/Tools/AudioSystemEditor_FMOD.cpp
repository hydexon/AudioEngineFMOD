#include "AudioSystemEditor_FMOD.h"
#include "AudioSystemCtrl_FMOD.h"

#include <AzCore/Utils/Utils.h>
#include <IAudioSystem.h>
#include <AzCore/std/smart_ptr/make_shared.h>
#include <AzCore/Console/IConsole.h>

#include "../Clients/Engine/Common_FMOD.h"
#include "AudioConnections.h"

#include "ConnectionWidgets/LoadSampleDataForm.h"
#include "ConnectionWidgets/EventPropertiesForm.h"
#include "ConnectionWidgets/EvtToRtpcParamsForm.h"

void InitFMODResources()
{
    Q_INIT_RESOURCE(EditorFMODIcons);
}

using namespace AudioControls;

namespace AudioEngineFMOD
{

TImplControlType TagToType(const AZStd::string_view tag)
{
    if(tag == XMLTags::FMODEventTag)
    {
        return eFMOD_EVENT;
    }
    else if(tag == XMLTags::FMODParameterTag)
    {
        return eFMOD_PARAMETER;
    }
    else if(tag == XMLTags::FMODStudioBankTag)
    {
        return eFMOD_SOUNDBANK;
    }

    return eFMOD_INVALID;
}

const AZStd::string_view TypeToTag(const TImplControlType type)
{
    switch(type)
    {
    case eFMOD_EVENT:
        return XMLTags::FMODEventTag;
    case eFMOD_PARAMETER:
        return XMLTags::FMODParameterTag;
    case eFMOD_SOUNDBANK:
        return XMLTags::FMODStudioBankTag;
    default:
        break;
    }

    return "";
}


CAudioSystemEditor_FMOD::CAudioSystemEditor_FMOD()
    : QObject()
{
    InitFMODResources();

    m_localizedParentControl.SetParent(&m_rootControl);
    m_rootControl.AddChild(&m_localizedParentControl);

    m_loader.Load(this, &m_rootControl, &m_localizedParentControl);
}

void CAudioSystemEditor_FMOD::Reload()
{
    // set all the controls as placeholder as we don't know if
    // any of them have been removed but still have connections to them
    for (const auto& idControlPair : m_controls)
    {
        TControlPtr control = idControlPair.second;
        if (control)
        {
            control->SetPlaceholder(true);
        }
    }

    m_loader.Load(this,&m_rootControl, &m_localizedParentControl);
    {
        m_currentLanguageName.clear();
        if(auto console = AZ::Interface<AZ::IConsole>::Get())
        {
            console->GetCvarValue("g_languageAudio", m_currentLanguageName);
            AZStd::to_lower(m_currentLanguageName.begin(), m_currentLanguageName.end());
        }

        AZStd::string parentName = "Localized Banks";
        if(!m_currentLanguageName.empty())
        {
            parentName += AZStd::string::format(" (%s)", m_currentLanguageName.c_str());
        }

        m_localizedParentControl.SetName(parentName);
    }
    m_connectionsByID.clear();
}

AudioControls::IAudioSystemControl *CAudioSystemEditor_FMOD::CreateControl(const AudioControls::SControlDef &controlDefinition)
{
    AZStd::string fullName = controlDefinition.m_name;
    IAudioSystemControl* parent = controlDefinition.m_parentControl;
    if (parent)
    {
        AZ::StringFunc::Path::Join(controlDefinition.m_parentControl->GetName().c_str(), fullName.c_str(), fullName);
    }

    if (!controlDefinition.m_path.empty())
    {
        AZ::StringFunc::Path::Join(controlDefinition.m_path.c_str(), fullName.c_str(), fullName);
    }

    CID id = Audio::AudioStringToID<CID>(fullName.c_str());

    AudioControls::IAudioSystemControl* control = GetControl(id);
    if (control)
    {
        if (control->IsPlaceholder())
        {
            control->SetPlaceholder(false);
            if (parent && parent->IsPlaceholder())
            {
                parent->SetPlaceholder(false);
            }
        }
        return control;
    }
    else
    {
        TControlPtr newControl = AZStd::make_shared<IAudioSystemCtrl_FMOD>(controlDefinition.m_name, id, controlDefinition.m_type);
        if (!parent)
        {
            parent = &m_rootControl;
        }

        parent->AddChild(newControl.get());
        newControl->SetParent(parent);
        newControl->SetLocalized(controlDefinition.m_isLocalized);
        m_controls[id] = newControl;
        return newControl.get();
    }
}

AudioControls::IAudioSystemControl *CAudioSystemEditor_FMOD::GetRoot()
{
    return &m_rootControl;
}

AudioControls::IAudioSystemControl *CAudioSystemEditor_FMOD::GetControl(AudioControls::CID id) const
{
    if (id != ACE_INVALID_CID)
    {
        auto it = m_controls.find(id);
        if (it != m_controls.end())
        {
            return it->second.get();
        }
    }
    return nullptr;
}

AudioControls::EACEControlType CAudioSystemEditor_FMOD::ImplTypeToATLType(AudioControls::TImplControlType type) const
{
    switch(type)
    {
    case eFMOD_SNAPSHOT:
    case eFMOD_EVENT:
        return AudioControls::eACET_TRIGGER;
    case eFMOD_PARAMETER:
        return AudioControls::eACET_RTPC;
    case eFMOD_SOUNDBANK:
        return AudioControls::eACET_PRELOAD;
    default:
        break;
    }

    return AudioControls::eACET_NUM_TYPES;
}

AudioControls::TImplControlTypeMask CAudioSystemEditor_FMOD::GetCompatibleTypes(AudioControls::EACEControlType atlControlType) const
{
    switch(atlControlType)
    {
    case AudioControls::eACET_TRIGGER:
        return eFMOD_EVENT | eFMOD_SNAPSHOT;
    case AudioControls::eACET_RTPC:
        return eFMOD_PARAMETER | eFMOD_EVENT | eFMOD_SNAPSHOT;
    case AudioControls::eACET_PRELOAD:
        return eFMOD_SOUNDBANK;
    default:
        break;
    }
    return AudioControls::AUDIO_IMPL_INVALID_TYPE;
}

AudioControls::TConnectionPtr CAudioSystemEditor_FMOD::CreateConnectionToControl(AudioControls::EACEControlType atlControlType, AudioControls::IAudioSystemControl *middlewareControl)
{
    if(middlewareControl)
    {
        middlewareControl->SetConnected(true);
        ++m_connectionsByID[middlewareControl->GetId()];

        switch(middlewareControl->GetType())
        {
        case eFMOD_SOUNDBANK:
            return AZStd::make_shared<CFMODBankConnection>(middlewareControl->GetId());
        case eFMOD_SNAPSHOT:
        case eFMOD_EVENT: {
            switch(atlControlType)
            {
            case eACET_TRIGGER:
                return AZStd::make_shared<CFMODEventConnection>(middlewareControl->GetId());
            case eACET_RTPC:
                return AZStd::make_shared<CFMODEventParamRTPC>(middlewareControl->GetId());
            default:
                break;
            }
        }
        default:
            return AZStd::make_shared<IAudioConnection>(middlewareControl->GetId());
        }
    }
    return nullptr;
}

AudioControls::TConnectionPtr CAudioSystemEditor_FMOD::CreateConnectionFromXMLNode(AZ::rapidxml::xml_node<char> *node, AudioControls::EACEControlType atlControlType)
{
    using namespace Audio;
    if(!node)
    {
        return nullptr;
    }

    IAudioSystemControl* control = nullptr;
    TConnectionPtr connection;

    AZStd::string_view element(node->name());
    TImplControlType type = TagToType(element);

    //Connections from FMODEvent:
    if(type != AUDIO_IMPL_INVALID_TYPE)
    {
        auto attr = node->first_attribute(XMLTags::FMODPathAttribute, 0, false);
        if(!attr || attr->value()[0] == '\0')
        {
            return nullptr;
        }

        const char* controlName = attr->value();

        attr = node->first_attribute(XMLTags::FMODLocalizedAttribute);
        bool isLocalized = attr ? AZ::StringFunc::Equal(attr->value(), "true") : false;

        control = GetControlByName(controlName, isLocalized);
        if(!control)
        {
            control = CreateControl(SControlDef(controlName, type));
            if(control)
            {
                control->SetPlaceholder(true);
                control->SetLocalized(isLocalized);
            }
        }

        if(control)
        {
            control->SetConnected(true);
            ++m_connectionsByID[control->GetId()];

            switch(type)
            {
            case eFMOD_SOUNDBANK: {
                auto conn = AZStd::make_shared<CFMODBankConnection>(control->GetId());
                if(auto lsdAttr = node->first_attribute(XMLTags::FMODSamplePreloadAttr); lsdAttr != nullptr)
                {
                    AZStd::string_view lsdStr = lsdAttr->value();
                    bool isLsd = AZ::StringFunc::Equal(lsdStr, "true"); //Not the acid.
                    conn->m_loadSampleData = isLsd;
                }
                return conn;
            }
            case eFMOD_SNAPSHOT:
            case eFMOD_EVENT: {
                if(atlControlType == AudioControls::eACET_TRIGGER)
                {
                    auto conn = AZStd::make_shared<CFMODEventConnection>(control->GetId());
                    if(auto lsdAttr = node->first_attribute(XMLTags::FMODSamplePreloadAttr);
                            lsdAttr != nullptr)
                    {
                        AZStd::string_view lsdStr = lsdAttr->value();
                        bool isLsd = AZ::StringFunc::Equal(lsdStr, "true");
                        conn->m_loadSampleData = isLsd;
                    }

                    if(auto actionAttr = node->first_attribute(XMLTags::FMODEvtAction);
                            actionAttr != nullptr)
                    {
                        AZStd::string_view actionStr = actionAttr->value();
                        conn->m_action = Utils::GetActionFromXmlStr(actionStr);
                    }

                    if(auto stopAttr = node->first_attribute(XMLTags::FMODStopMode);
                            stopAttr != nullptr)
                    {
                        AZStd::string_view stopModeStr = stopAttr->value();
                        conn->m_stopMode = AZ::StringFunc::Equal(stopModeStr, "AllowFadeout") ?
                                    FMOD_STUDIO_STOP_ALLOWFADEOUT : FMOD_STUDIO_STOP_IMMEDIATE;
                    }

                    if(auto isSnapAttr = node->first_attribute(XMLTags::FMODSnapshotAttr);
                            isSnapAttr != nullptr)
                    {
                        conn->m_isSnapshot = AZ::StringFunc::Equal(isSnapAttr->value(), "true");
                    }
                    return conn;
                }
                else //AudioControls::eACET_RTPC
                {
                    auto conn = AZStd::make_shared<CFMODEventParamRTPC>(control->GetId());
                    if(auto paramAttr = node->first_attribute(XMLTags::FMODParamAttr);
                            paramAttr != nullptr)
                    {
                        conn->m_paramName = paramAttr->value();
                    }
                    return conn;
                }
            }
            case eFMOD_PARAMETER: {
                auto isGlobalAttr  = node->first_attribute(XMLTags::FMODIsGlobalParam);
                bool isGlobalParam = isGlobalAttr ? AZ::StringFunc::Equal(isGlobalAttr->value(), "true") : false;

                auto conn = AZStd::make_shared<CFMODParamRTPC>(control->GetId());
                conn->m_isGlobal = isGlobalParam;
                return conn;
            }
            default:
                break;
            }
        }
        return AZStd::make_shared<IAudioConnection>(control->GetId());
    }

    return nullptr;
}

AZ::rapidxml::xml_node<char> *CAudioSystemEditor_FMOD::CreateXMLNodeFromConnection(const AudioControls::TConnectionPtr connection, [[maybe_unused]] const AudioControls::EACEControlType atlControlType)
{
    const IAudioSystemControl* control = GetControl(connection->GetID());
    if(!control)
    {
        return nullptr;
    }

    XmlAllocator& xmlAlloc(AudioControls::s_xmlAllocator);

    switch(control->GetType())
    {
    case eFMOD_SNAPSHOT:
    case eFMOD_EVENT:
    {
        auto connNode = xmlAlloc.allocate_node(AZ::rapidxml::node_element);
        connNode->name(XMLTags::FMODEventTag);

        auto pathAttr = xmlAlloc.allocate_attribute(XMLTags::FMODPathAttribute, xmlAlloc.allocate_string(control->GetName().c_str()));
        connNode->append_attribute(pathAttr);

        auto locAttr = xmlAlloc.allocate_attribute(XMLTags::FMODLocalizedAttribute,
                                                   xmlAlloc.allocate_string(
                                                       control->IsLocalized() ? "true" : "false"));
        connNode->append_attribute(locAttr);

        auto isSnapshotAttr = xmlAlloc.allocate_attribute(XMLTags::FMODSnapshotAttr, xmlAlloc.allocate_string(control->GetType() == eFMOD_SNAPSHOT ? "true" : "false"));
        connNode->append_attribute(isSnapshotAttr);

        switch(atlControlType)
        {
        case eACET_TRIGGER: {
            auto conn = static_cast<const CFMODEventConnection*>(connection.get());
            AZ_Assert(conn, "CFMODEventConnection is a invalid cast!");

            auto actionStr  = Utils::GetXmlStrFromAction(conn->m_action);
            auto actionAttr = xmlAlloc.allocate_attribute(XMLTags::FMODEvtAction, xmlAlloc.allocate_string(actionStr.c_str()));
            connNode->append_attribute(actionAttr);

            auto stopModeAttr = xmlAlloc.allocate_attribute(XMLTags::FMODStopMode, xmlAlloc.allocate_string(conn->m_stopMode == FMOD_STUDIO_STOP_ALLOWFADEOUT ? "AllowFadeout" : "Immediate"));
            connNode->append_attribute(stopModeAttr);

            auto snapAttr = xmlAlloc.allocate_attribute(XMLTags::FMODSnapshotAttr, xmlAlloc.allocate_string(conn->m_isSnapshot ? "true" : "false"));
            connNode->append_attribute(snapAttr);
            return connNode;
        }
        case eACET_RTPC: {
            auto conn = static_cast<const CFMODEventParamRTPC*>(connection.get());
            AZ_Assert(conn, "CFMODEventParamRTPC is a invalid cast!");

            auto paramNameAttr = xmlAlloc.allocate_attribute(XMLTags::FMODParamAttr, xmlAlloc.allocate_string(conn->m_paramName.c_str()));
            connNode->append_attribute(paramNameAttr);

            auto defaultValueIt = m_loader.GetEventParameters().find(conn->m_paramName);
            float value = 0.0f;
            if(defaultValueIt != m_loader.GetEventParameters().end())
            {
                value = defaultValueIt->second;
            }
            auto initValAttr  = xmlAlloc.allocate_attribute(XMLTags::FMODInitialValue, xmlAlloc.allocate_string(AZStd::to_string(value).c_str()));
            connNode->append_attribute(initValAttr);

            return connNode;
        }
        case eACET_SWITCH_STATE:
        case eACET_ENVIRONMENT:
        case eACET_SWITCH:
        default:
            break;
        }
        break;
    }
    case eFMOD_PARAMETER: { //Global and Per-Object FMOD Parameters
        auto connNode = xmlAlloc.allocate_node(AZ::rapidxml::node_element);
        connNode->name(XMLTags::FMODParameterTag);

        auto pathAttr = xmlAlloc.allocate_attribute(XMLTags::FMODPathAttribute, xmlAlloc.allocate_string(control->GetName().c_str()));
        connNode->append_attribute(pathAttr);

        auto isGlobalAttr = xmlAlloc.allocate_attribute(XMLTags::FMODIsGlobalParam, xmlAlloc.allocate_string(IsFMODParameterGlobal(control->GetName()) ? "true" : "false"));
        connNode->append_attribute(isGlobalAttr);

        auto defaultValueIt = m_loader.GetEventParameters().find(control->GetName());
        float value = 0.0f;
        if(defaultValueIt != m_loader.GetEventParameters().end())
        {
            value = defaultValueIt->second;
        }
        auto initValAttr  = xmlAlloc.allocate_attribute(XMLTags::FMODInitialValue, xmlAlloc.allocate_string(AZStd::to_string(value).c_str()));
        connNode->append_attribute(initValAttr);

        return connNode;
    }
    case eFMOD_SOUNDBANK: {
        auto connNode = xmlAlloc.allocate_node(AZ::rapidxml::node_element);
        connNode->name(XMLTags::FMODStudioBankTag);

        auto pathAttr = xmlAlloc.allocate_attribute(XMLTags::FMODPathAttribute, xmlAlloc.allocate_string(control->GetName().c_str()));
        connNode->append_attribute(pathAttr);

        return connNode;
    }
    default:
        return nullptr;
    }
    return nullptr;
}

const AZStd::string_view CAudioSystemEditor_FMOD::GetTypeIcon(AudioControls::TImplControlType type) const
{
    switch(type)
    {
    case eFMOD_EVENT:
        return ":/Editor/EditorFMODIcons/events.png";
    case eFMOD_PARAMETER:
        return ":/Editor/EditorFMODIcons/switch.png";
    case eFMOD_AUXBUS:
        return ":/Editor/EditorFMODIcons/bus.png";
    case eFMOD_SOUNDBANK:
        return ":/Editor/EditorFMODIcons/soundbank.png";
    case eFMOD_SNAPSHOT:
        return ":/Editor/EditorFMODIcons/snapshot.png";
    default:
        return ":/Editor/EditorFMODIcons/invalid.png";
    }
}

const AZStd::string_view CAudioSystemEditor_FMOD::GetTypeIconSelected(AudioControls::TImplControlType type) const
{
    return GetTypeIcon(type);
}

AZStd::string CAudioSystemEditor_FMOD::GetName() const
{
    return "FMOD Studio";
}

AZ::IO::FixedMaxPath CAudioSystemEditor_FMOD::GetDataPath() const
{
    auto projectPath = AZ::IO::FixedMaxPath( AZ::Utils::GetProjectPath() );
    return (projectPath / "Assets" / "Audio" / "FMOD"  );
}

void CAudioSystemEditor_FMOD::DataSaved()
{

}

QWidget *CAudioSystemEditor_FMOD::CreateConnectionPropertiesWidget(const AudioControls::TConnectionPtr connection, [[maybe_unused]] AudioControls::EACEControlType atlControlType)
{
    if(!connection)
        return nullptr;

    IAudioSystemControl* control = GetControl(connection->GetID());
    if(!control)
        return nullptr;

    switch(control->GetType())
    {
        case eFMOD_SOUNDBANK:
            return new LoadSampleDataForm(connection);
        case eFMOD_EVENT:
        case eFMOD_SNAPSHOT: {
            if(atlControlType == AudioControls::eACET_TRIGGER)
                return new EventPropertiesForm(connection, control->GetType());
            else //eACET_RTPC.
                return new EvtToRtpcParamsForm(connection, &m_loader);
        }
        default:
            return nullptr;
    }
}

bool CAudioSystemEditor_FMOD::IsFMODParameterGlobal(const AZStd::string_view paramPath)
{
    for(auto& localParam : m_loader.GetEventParameters())
    {
        if(localParam.first == paramPath)
        {
            return false;
        }
    }

    return true;
}

IAudioSystemControl *CAudioSystemEditor_FMOD::GetControlByName(AZStd::string name, bool isLocalized, AudioControls::IAudioSystemControl *parent) const
{
    if (parent) //This is definitively used for the Switch Groups.
    {
        AZ::StringFunc::Path::Join(parent->GetName().c_str(), name.c_str(), name);
    }
    //TODO: Deal with the localized one.
    AZ_UNUSED(isLocalized);

    return GetControl(Audio::AudioStringToID<CID>(name.data()));
}


}