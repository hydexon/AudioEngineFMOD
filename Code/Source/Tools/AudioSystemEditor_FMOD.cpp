#include "AudioSystemEditor_FMOD.h"
#include "AudioSystemCtrl_FMOD.h"

#include <AzCore/Utils/Utils.h>
#include <IAudioSystem.h>
#include <AzCore/std/smart_ptr/make_shared.h>

#include "../Clients/Engine/Common_FMOD.h"
#include "AudioConnections.h"

#include "ConnectionWidgets/LoadSampleDataForm.h"
#include "ConnectionWidgets/EventPropertiesForm.h"

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
    m_loader.Load(this);

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

    m_loader.Load(this);

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
        return eFMOD_PARAMETER;
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
        case eFMOD_EVENT:
            return AZStd::make_shared<CFMODEventConnection>(middlewareControl->GetId());
        default:
            return AZStd::make_shared<IAudioConnection>(middlewareControl->GetId());
        }
    }
    return nullptr;
}

AudioControls::TConnectionPtr CAudioSystemEditor_FMOD::CreateConnectionFromXMLNode(AZ::rapidxml::xml_node<char> *node, AudioControls::EACEControlType atlControlType)
{
    if(node)
    {
        AZStd::string_view element(node->name());
        TImplControlType type = TagToType(element);
        if(type != AUDIO_IMPL_INVALID_TYPE)
        {
            AZStd::string name;
            AZStd::string_view localized;

            if(auto nameAttr = node->first_attribute(XMLTags::FMODPathAttribute, 0, false);
                    nameAttr != nullptr)
            {
                name = nameAttr->value();
            }

            if(auto localizedAttr = node->first_attribute(XMLTags::FMODLocalizedAttribute, 0, false);
                    localizedAttr != nullptr)
            {
                localized =  localizedAttr->value();
            }

            bool isLocalized = AZ::StringFunc::Equal(localized, "true");

            IAudioSystemControl* ctrl = GetControlByName(name, isLocalized);
            if(!ctrl)
            {
                ctrl = CreateControl(SControlDef(name, type));
                if(ctrl)
                {
                    ctrl->SetPlaceholder(true);
                    ctrl->SetLocalized(isLocalized);
                }
            }

            //TODO: Deal with the roughest equivalent of Switch Groups, Switch States,etc.

            if(ctrl)
            {
                ctrl->SetConnected(true);
                ++m_connectionsByID[ctrl->GetId()];

                //TODO: Special IAudioConnection derived if eFMOD_PARAMETER?
                //      Also deal with Switches and States.
                switch(type)
                {
                    case eFMOD_SOUNDBANK: {
                        auto conn = AZStd::make_shared<CFMODBankConnection>(ctrl->GetId());
                        if(auto lsdAttr = node->first_attribute(XMLTags::FMODSamplePreloadAttr); lsdAttr != nullptr)
                        {
                            AZStd::string_view lsdStr = lsdAttr->value();
                            bool isLsd = AZ::StringFunc::Equal(lsdStr, "true"); //Not the acid.
                            conn->m_loadSampleData = isLsd;
                        }
                        return conn;
                    }
                    case eFMOD_EVENT: {
                        auto conn = AZStd::make_shared<CFMODEventConnection>(ctrl->GetId());
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
                            conn->m_action = AZ::StringFunc::Equal(actionStr, "play") ?
                                        0 : AZ::StringFunc::Equal(actionStr, "pause") ? 1 : 2;
                        }

                        if(auto stopAttr = node->first_attribute(XMLTags::FMODStopMode);
                                stopAttr != nullptr)
                        {
                            AZStd::string_view stopModeStr = stopAttr->value();
                            conn->m_stopMode = AZ::StringFunc::Equal(stopModeStr, "AllowFadeout") ?
                                        FMOD_STUDIO_STOP_ALLOWFADEOUT : FMOD_STUDIO_STOP_IMMEDIATE;
                        }
                        return conn;
                    }

                    default: break;
                }
                return AZStd::make_shared<IAudioConnection>(ctrl->GetId());
            }
        }
    }
    return nullptr;
}

AZ::rapidxml::xml_node<char> *CAudioSystemEditor_FMOD::CreateXMLNodeFromConnection(const AudioControls::TConnectionPtr connection, const AudioControls::EACEControlType atlControlType)
{
    const IAudioSystemControl* control = GetControl(connection->GetID());
    if(control)
    {
        XmlAllocator& xmlAlloc(AudioControls::s_xmlAllocator);
        switch(control->GetType())
        {
            case eFMOD_EVENT:
            {
                auto connectionNode = xmlAlloc.allocate_node(
                            AZ::rapidxml::node_element,
                            xmlAlloc.allocate_string(TypeToTag(control->GetType()).data()));

                auto pathAttr = xmlAlloc.allocate_attribute(
                            XMLTags::FMODPathAttribute,
                            xmlAlloc.allocate_string(control->GetName().c_str())
                            );

                connectionNode->append_attribute(pathAttr);

                auto conn = static_cast<const CFMODEventConnection*>(connection.get());
                auto loadSampleDataAttr = xmlAlloc.allocate_attribute(XMLTags::FMODSamplePreloadAttr,
                                                                      xmlAlloc.allocate_string(
                                                                          conn->m_loadSampleData ? "true" : "false"));

                connectionNode->append_attribute(loadSampleDataAttr);

                auto evtActionAttr = xmlAlloc.allocate_attribute(XMLTags::FMODEvtAction,
                                                                 xmlAlloc.allocate_string(
                                                                     conn->m_action == 0 ? "play" :
                                                                                           conn->m_action == 1 ? "paused"
                                                                                                               : "stop"));

                connectionNode->append_attribute(evtActionAttr);


                auto evtStopMode = xmlAlloc.allocate_attribute(XMLTags::FMODStopMode,
                                                                xmlAlloc.allocate_string(
                                                                    conn->m_stopMode == FMOD_STUDIO_STOP_ALLOWFADEOUT ? "AllowFadeout" : "Immediate"));
                connectionNode->append_attribute(evtStopMode);

                return connectionNode;
            }
            case eFMOD_SOUNDBANK:
            {
                auto connectionNode = xmlAlloc.allocate_node(
                            AZ::rapidxml::node_element,
                            xmlAlloc.allocate_string(TypeToTag(control->GetType()).data()));

                auto pathAttr = xmlAlloc.allocate_attribute(
                            XMLTags::FMODPathAttribute,
                            xmlAlloc.allocate_string(control->GetName().c_str())
                            );

                connectionNode->append_attribute(pathAttr);

                if(control->IsLocalized())
                {
                    auto locAttr = xmlAlloc.allocate_attribute(
                                XMLTags::FMODLocalizedAttribute,
                                xmlAlloc.allocate_string("true")
                                );

                    connectionNode->append_attribute(locAttr);
                }
                auto conn = static_cast<const CFMODBankConnection*>(connection.get());
                auto preloadSampleDataAttr = xmlAlloc.allocate_attribute(XMLTags::FMODSamplePreloadAttr,
                                                                         xmlAlloc.allocate_string(
                                                                             conn->m_loadSampleData ? "true" : "false"
                                                                             ));
                connectionNode->append_attribute(preloadSampleDataAttr);

                return connectionNode;
            }
            default:
            {
                AZ_Warning("FMODAudioSystem", false, "Support to create XML node for '%s' not implemented yet!", TypeToTag(control->GetType()).data());
                break;
            }
        }
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

QWidget *CAudioSystemEditor_FMOD::CreateConnectionPropertiesWidget(const AudioControls::TConnectionPtr connection, AudioControls::EACEControlType atlControlType)
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
        case eFMOD_SNAPSHOT:
            return new EventPropertiesForm(connection, control->GetType());
        default:
            return nullptr;
    }
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