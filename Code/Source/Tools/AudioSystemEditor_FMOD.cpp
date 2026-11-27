#include "AudioSystemEditor_FMOD.h"
#include "AudioSystemCtrl_FMOD.h"

#include <AzCore/Utils/Utils.h>
#include <IAudioSystem.h>
#include <AzCore/std/smart_ptr/make_shared.h>
#include <AzCore/Console/IConsole.h>

#include "../Clients/Engine/Common_FMOD.h"
#include "AudioConnections.h"

#include "ConnectionWidgets/EventPropertiesForm.h"
#include "ConnectionWidgets/FMODParamToRTPCWidget.h"
#include "ConnectionWidgets/FMODParamToStateWidget.h"

void InitFMODResources()
{
    Q_INIT_RESOURCE(EditorFMODIcons);
}

using namespace AudioControls;

namespace AudioEngineFMOD
{

TImplControlType TagToType(const AZStd::string_view tag)
{
    if (tag == XMLTags::EventTag)
    {
        return eFMOD_EVENT;
    }
    else if (tag == XMLTags::EventParameterTag)
    {
        return eFMOD_EVENT_PARAMETER;
    }
    else if (tag == XMLTags::SnapshotTag)
    {
        return eFMOD_SNAPSHOT;
    }
    else if (tag == XMLTags::SnapshotParameterTag)
    {
        return eFMOD_SNAPSHOT_PARAMETER;
    }
    else if (tag == XMLTags::GlobalParameterTag)
    {
        return eFMOD_GLOBAL_PARAMETER;
    }
    else if (tag == XMLTags::BankTag)
    {
        return eFMOD_SOUNDBANK;
    }
    else if (tag == XMLTags::ReturnTag)
    {
        return eFMOD_RETURN;
    }
    return eFMOD_INVALID;
}

AZStd::string_view TypeToTag(const TImplControlType type)
{
    switch (type)
    {
    case eFMOD_EVENT: return XMLTags::EventTag;
    case eFMOD_EVENT_PARAMETER: return XMLTags::EventParameterTag;
    case eFMOD_SNAPSHOT: return XMLTags::SnapshotTag;
    case eFMOD_SNAPSHOT_PARAMETER: return XMLTags::SnapshotParameterTag;
    case eFMOD_GLOBAL_PARAMETER: return XMLTags::GlobalParameterTag;
    case eFMOD_SOUNDBANK: return XMLTags::BankTag;
    case eFMOD_RETURN: return XMLTags::ReturnTag;
    default: return "";
    }
}

AZStd::string_view StripPrefixFromPath(AZStd::string_view ogPath)
{
    auto pos = ogPath.find(":");
    return (pos != std::string_view::npos) ? ogPath.substr(pos + 1) : ogPath;
}

CAudioSystemEditor_FMOD::CAudioSystemEditor_FMOD()
    : m_banksControl("Banks", ACE_INVALID_CID, eFMOD_GROUPCTRL)
    , m_eventsControl("Events", ACE_INVALID_CID, eFMOD_GROUPCTRL)
    , m_snapshotsCtrl("Snapshots", ACE_INVALID_CID, eFMOD_GROUPCTRL)
    , m_globalParamsCtrl("Global Parameters", ACE_INVALID_CID, eFMOD_GROUPCTRL)
    , m_returnsControl("Returns", ACE_INVALID_CID, eFMOD_GROUPCTRL)
{
    InitFMODResources();

    m_banksControl.SetParent(&m_rootControl);
    m_rootControl.AddChild(&m_banksControl);

    m_eventsControl.SetParent(&m_rootControl);
    m_rootControl.AddChild(&m_eventsControl);

    m_snapshotsCtrl.SetParent(&m_rootControl);
    m_rootControl.AddChild(&m_snapshotsCtrl);

    m_globalParamsCtrl.SetParent(&m_rootControl);
    m_rootControl.AddChild(&m_globalParamsCtrl);

    m_returnsControl.SetParent(&m_rootControl);
    m_rootControl.AddChild(&m_returnsControl);
}

void CAudioSystemEditor_FMOD::Reload()
{
    for (const auto& idControlPair : m_controls)
    {
        auto control = idControlPair.second;
        if (control)
        {
            control->SetPlaceholder(true);
        }
    }

    m_fmodControlPaths.clear();
    m_loader.Load(this);
    m_connectionsByID.clear();
    UpdateConnectedStatus();
}

AudioControls::IAudioSystemControl *CAudioSystemEditor_FMOD::CreateControl(const AudioControls::SControlDef &controlDefinition)
{
    AZStd::string fullName = controlDefinition.m_name;
    IAudioSystemControl* parent = controlDefinition.m_parentControl;

    if(controlDefinition.m_path.empty()) //Folders, Soundbanks and Returns.
    {
        auto pCtrl = parent ? parent : &m_rootControl;

        if(controlDefinition.m_isLocalized) // Localized Banks.
        {
            AZStd::string baseLocalized = "";
            AZ::StringFunc::Path::Join(pCtrl->GetName().c_str(), m_currentLanguageName.c_str(), baseLocalized);
            AZ::StringFunc::Path::Join(baseLocalized.c_str(), fullName.c_str(), fullName);
        }
        else
        {
            AZ::StringFunc::Path::Join(pCtrl->GetName().c_str(), fullName.c_str(), fullName);
        }
    }

    CID id = Audio::AudioStringToID<CID>(controlDefinition.m_path.empty() ? fullName.c_str() : controlDefinition.m_path.c_str());
    if (AudioControls::IAudioSystemControl* control = GetControl(id))
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
        TControlPtr newControl = AZStd::make_shared<IAudioSystemControl>(controlDefinition.m_name, id, controlDefinition.m_type);
        if (!parent)
        {
            parent = &m_rootControl;
        }

        parent->AddChild(newControl.get());
        newControl->SetParent(parent);
        newControl->SetLocalized(controlDefinition.m_isLocalized);
        m_controls[id] = newControl;
        m_fmodControlPaths[id] = controlDefinition.m_path;
        AZ_Assert(m_fmodControlPaths[id].empty() == false, "CreateControl Path is empty!");
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
        if (const auto it = m_controls.find(id); it != m_controls.end())
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
    case eFMOD_SNAPSHOT:
        return AudioControls::eACET_TRIGGER;
    case eFMOD_EVENT_PARAMETER:
    case eFMOD_SNAPSHOT_PARAMETER:
    case eFMOD_GLOBAL_PARAMETER:
        return AudioControls::eACET_RTPC;
    case eFMOD_SOUNDBANK:
        return AudioControls::eACET_PRELOAD;
    case eFMOD_RETURN:
        return AudioControls::eACET_ENVIRONMENT;
    default:
        return AudioControls::eACET_NUM_TYPES;
    }
}

AudioControls::TImplControlTypeMask CAudioSystemEditor_FMOD::GetCompatibleTypes(AudioControls::EACEControlType atlControlType) const
{
    switch(atlControlType)
    {
    case AudioControls::eACET_PRELOAD:
        return eFMOD_SOUNDBANK;
    case AudioControls::eACET_TRIGGER:
        return eFMOD_EVENT | eFMOD_SNAPSHOT;
    case AudioControls::eACET_RTPC:
    case AudioControls::eACET_SWITCH_STATE:
        return eFMOD_EVENT_PARAMETER | eFMOD_SNAPSHOT_PARAMETER | eFMOD_GLOBAL_PARAMETER;
    case AudioControls::eACET_ENVIRONMENT:
        return eFMOD_RETURN;
    default:
        return AudioControls::AUDIO_IMPL_INVALID_TYPE;
    }
}

AudioControls::TConnectionPtr CAudioSystemEditor_FMOD::CreateConnectionToControl(AudioControls::EACEControlType atlControlType, AudioControls::IAudioSystemControl *middlewareControl)
{
    if (!middlewareControl)
        return nullptr;

    TConnectionPtr connection;
    middlewareControl->SetConnected(true);
    ++m_connectionsByID[middlewareControl->GetId()];

    switch (middlewareControl->GetType())
    {
    case eFMOD_SNAPSHOT_PARAMETER:
    case eFMOD_GLOBAL_PARAMETER:
    case eFMOD_EVENT_PARAMETER:
        {
            if (atlControlType == eACET_RTPC)
            {
                return AZStd::make_shared<FMODParamRtpcConnection>(middlewareControl->GetId());
            }
            else if (atlControlType == eACET_SWITCH || atlControlType == eACET_SWITCH_STATE)
            {
                return AZStd::make_shared<FMODParamToStateConnection>(middlewareControl->GetId());
            }
            break;
        }
    case eFMOD_EVENT:
    case eFMOD_SNAPSHOT:
        {
            return AZStd::make_shared<FMODEventConnection>(middlewareControl->GetId());
        }
    default:
        return AZStd::make_shared<AudioControls::IAudioConnection>(middlewareControl->GetId());
    }

    return AZStd::make_shared<AudioControls::IAudioConnection>(middlewareControl->GetId()); //I wonder why.
}

AudioControls::TConnectionPtr CAudioSystemEditor_FMOD::CreateConnectionFromXMLNode(AZ::rapidxml::xml_node<char> *node, AudioControls::EACEControlType atlControlType)
{
    using namespace Audio;
    if(!node)
    {
        return nullptr;
    }
    TImplControlType type = TagToType(node->name());
    if (type == eFMOD_INVALID)
        return nullptr;

    IAudioSystemControl* control = nullptr;
    //Sound banks and Returns don't need get the controls by their path, so we process them first.
    if (type == eFMOD_SOUNDBANK || type == eFMOD_RETURN )
    {
        auto parentControl = type == eFMOD_SOUNDBANK ? &m_banksControl : &m_returnsControl;
        bool isLocalized = false;
        if (type == eFMOD_SOUNDBANK)
        {
            auto localizedAttr = node->first_attribute(XMLTags::LocalizedAttr);
            if (localizedAttr)
            {
                isLocalized = AZ::StringFunc::ToBool(localizedAttr->value());
            }
        }

        auto nameAttr = node->first_attribute(XMLTags::NameAttr);
        if (nameAttr && nameAttr->value()[0] != '\0')
        {
            auto name = nameAttr->value();
            control = GetControlByName(name, isLocalized, parentControl);

            if (!control)
            {
                control = CreateControl(SControlDef(name, type, isLocalized, parentControl));
                if (control)
                {
                    control->SetPlaceholder(true);
                }
            }
        }
    }
    else
    {
        //We look for our control by path.
        if (auto pathAttr = node->first_attribute(XMLTags::PathAttr))
        {
            AZStd::string path = pathAttr->value();
            control = GetControlByPath(path, type);

            if (!control || control->GetType() != type)
            {
                //I'm not fully convinced of this part.
                AZStd::string name = "";
                AZStd::string parentPath = path;
                auto pos = path.find_last_of("/");
                if (pos != decltype(path)::npos)
                {
                    name = path.substr(pos + 1);
                    parentPath = (pos == 0) ? "/" : path.substr(0, pos);
                }
                control = CreateControl(SControlDef(name, type, false, CreatePlaceholderFolderPath(parentPath, type), path));
                if (control)
                {
                    control->SetPlaceholder(true);
                }
            }
        }
    }

    if (!control)
        return nullptr;

    control->SetConnected(true);
    ++m_connectionsByID[control->GetId()];

    switch (type)
    {
    case eFMOD_SNAPSHOT:
    case eFMOD_EVENT:
        {
            auto conn = AZStd::make_shared<FMODEventConnection>(control->GetId());
            auto evtTypeAttr = node->first_attribute(XMLTags::EventTypeAttr);

            if(evtTypeAttr)
            {
                conn->m_eventType = Utils::GetActionFromXmlStr(evtTypeAttr->value());
            }
            return conn;
        }
    break;
    case eFMOD_SNAPSHOT_PARAMETER:
    case eFMOD_GLOBAL_PARAMETER:
    case eFMOD_EVENT_PARAMETER:
        {
            if(atlControlType == AudioControls::eACET_RTPC)
            {
                auto conn = AZStd::make_shared<FMODParamRtpcConnection>(control->GetId());
                if(auto shiftAttr = node->first_attribute(XMLTags::ShiftAttr))
                {
                    conn->m_shift = AZ::StringFunc::ToFloat(shiftAttr->value());
                }
                if(auto multAttr = node->first_attribute(XMLTags::MultAttr))
                {
                    conn->m_mult = AZ::StringFunc::ToFloat(multAttr->value());
                }
                return conn;
            }
            else if(atlControlType == AudioControls::eACET_SWITCH || atlControlType == AudioControls::eACET_SWITCH_STATE)
            {
                auto conn = AZStd::make_shared<FMODParamToStateConnection>(control->GetId());
                if(auto valueAttr = node->first_attribute(XMLTags::ValueAttr))
                {
                    conn->m_value = AZ::StringFunc::ToFloat(valueAttr->value());
                }
                return conn;
            }
        }
    break;
    default:
        return AZStd::make_shared<IAudioConnection>(control->GetId());
    }

    return nullptr;
}

AZ::rapidxml::xml_node<char> *CAudioSystemEditor_FMOD::CreateXMLNodeFromConnection(const AudioControls::TConnectionPtr connection, const AudioControls::EACEControlType atlControlType)
{
    auto control = GetControl(connection->GetID());
    if (!control)
        return nullptr;

    XmlAllocator& xmlAlloc(AudioControls::s_xmlAllocator);
    auto pNode = xmlAlloc.allocate_node(AZ::rapidxml::node_element, xmlAlloc.allocate_string(TypeToTag((control->GetType())).data()));

    switch (control->GetType())
    {
    case eFMOD_EVENT:
    case eFMOD_SNAPSHOT:
        {
            auto pathAttr = xmlAlloc.allocate_attribute(XMLTags::PathAttr, xmlAlloc.allocate_string(m_fmodControlPaths[control->GetId()].c_str()));
            pNode->append_attribute(pathAttr);
            auto conn = static_cast<const FMODEventConnection*>(connection.get());
            if (conn->m_eventType == FMODEventType::Stop)
            {
                auto eventTypeAttr = xmlAlloc.allocate_attribute(XMLTags::EventTypeAttr, xmlAlloc.allocate_string("stop"));
                pNode->append_attribute(eventTypeAttr);
            }
        }
    break;
    case eFMOD_RETURN:
        {
            auto pathAttr = xmlAlloc.allocate_attribute(XMLTags::PathAttr, xmlAlloc.allocate_string(m_fmodControlPaths[control->GetId()].c_str()));
            pNode->append_attribute(pathAttr);
        }
    break;
    case eFMOD_EVENT_PARAMETER:
    case eFMOD_GLOBAL_PARAMETER:
    case eFMOD_SNAPSHOT_PARAMETER:
        {
            auto nameAttr = xmlAlloc.allocate_attribute(XMLTags::NameAttr, xmlAlloc.allocate_string(control->GetName().c_str()));
            pNode->append_attribute(nameAttr);

            auto pathAttr = xmlAlloc.allocate_attribute(XMLTags::PathAttr, xmlAlloc.allocate_string(m_fmodControlPaths[control->GetId()].c_str()));
            pNode->append_attribute(pathAttr);

            if (control->GetType() == eFMOD_EVENT_PARAMETER || control->GetType() == eFMOD_SNAPSHOT_PARAMETER)
            {
                auto targetPath = m_fmodControlPaths[control->GetParent()->GetId()];
                AZ_Assert(targetPath.empty() == false, "Target Path for Event or Snapshot Parameters Must not be empty!");
                const auto targetAttr = xmlAlloc.allocate_attribute(XMLTags::TargetAttr, xmlAlloc.allocate_string(targetPath.c_str()));
                pNode->append_attribute(targetAttr);
            }

            if (atlControlType == eACET_RTPC)
            {
                auto conn = static_cast<const FMODParamRtpcConnection*>(connection.get());
                auto multAttr = xmlAlloc.allocate_attribute(XMLTags::MultAttr, xmlAlloc.allocate_string(AZStd::to_string(conn->m_mult).c_str()));
                auto shiftAttr = xmlAlloc.allocate_attribute(XMLTags::ShiftAttr, xmlAlloc.allocate_string(AZStd::to_string(conn->m_shift).c_str()));
                pNode->append_attribute(multAttr);
                pNode->append_attribute(shiftAttr);
            }
            else if (atlControlType == eACET_SWITCH_STATE)
            {
                auto conn = static_cast<const FMODParamToStateConnection*>(connection.get());
                auto valueAttr = xmlAlloc.allocate_attribute(XMLTags::ValueAttr, xmlAlloc.allocate_string(AZStd::to_string(conn->m_value).c_str()));
                pNode->append_attribute(valueAttr);
            }
        }
    break;
    case eFMOD_SOUNDBANK:
        {
            auto nameAttr = xmlAlloc.allocate_attribute(XMLTags::NameAttr, xmlAlloc.allocate_string(control->GetName().c_str()));
            pNode->append_attribute(nameAttr);

            auto locallizedAttr = xmlAlloc.allocate_attribute(XMLTags::LocalizedAttr, xmlAlloc.allocate_string(AZStd::to_string(control->IsLocalized()).c_str()));
            pNode->append_attribute(locallizedAttr);
        }
    break;
    default: break;
    }

    return pNode;
}

void CAudioSystemEditor_FMOD::ConnectionRemoved(AudioControls::IAudioSystemControl* middlewareControl)
{
    if (!middlewareControl)
        return;

    int connectionCount = m_connectionsByID[middlewareControl->GetId()] - 1;
    if (connectionCount <= 0)
    {
        connectionCount = 0;
        middlewareControl->SetConnected(false);
    }
    m_connectionsByID[middlewareControl->GetId()] = connectionCount;
}

const AZStd::string_view CAudioSystemEditor_FMOD::GetTypeIcon(AudioControls::TImplControlType type) const
{
    switch (type)
    {
    case eFMOD_SOUNDBANK:
        return ":/AEFMODIcons/bank.png";
    case eFMOD_GROUPCTRL:
        return ":/AEFMODIcons/folder-horizontal.png";
    case eFMOD_EVENT:
        return ":/AEFMODIcons/event.png";
    case eFMOD_SNAPSHOT:
        return ":/AEFMODIcons/snapshot.png";
    case eFMOD_EVENT_PARAMETER:
    case eFMOD_SNAPSHOT_PARAMETER:
    case eFMOD_GLOBAL_PARAMETER:
        return ":/AEFMODIcons/tag_icon.png";
    case eFMOD_FOLDER:
        return ":/AEFMODIcons/folder_closed.png";
    case eFMOD_RETURN:
        return ":/AEFMODIcons/return.png";
    default:
        return ":/AEFMODIcons/folder-horizontal.png";
    }
}

const AZStd::string_view CAudioSystemEditor_FMOD::GetTypeIconSelected(AudioControls::TImplControlType type) const
{
    switch (type)
    {
    case eFMOD_GROUPCTRL:
        return ":/AEFMODIcons/folder-horizontal-open.png";
    case eFMOD_FOLDER:
        return ":/AEFMODIcons/folder_open.png";
    default:
        return GetTypeIcon(type);
    }
}

AZStd::string CAudioSystemEditor_FMOD::GetName() const
{
    return "FMOD Studio";
}

AZ::IO::FixedMaxPath CAudioSystemEditor_FMOD::GetDataPath() const
{
    auto projectPath = AZ::IO::FixedMaxPath( AZ::Utils::GetProjectPath() );
    return (projectPath / "Assets" / "Audio" / "FMOD" );
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
    case eFMOD_EVENT:
    case eFMOD_SNAPSHOT:
        return new EventPropertiesForm(connection, control->GetType());
    case eFMOD_SNAPSHOT_PARAMETER:
    case eFMOD_GLOBAL_PARAMETER:
    case eFMOD_EVENT_PARAMETER:
        {
            if(atlControlType == AudioControls::eACET_RTPC)
            {
                return new FMODParamToRTPCWidget(connection);
            }
            else if(atlControlType == AudioControls::eACET_SWITCH || atlControlType == AudioControls::eACET_SWITCH_STATE)
            {
                return new FMODParamToStateWidget(connection);
            }
        }
    default:
        return nullptr;
    }

    return nullptr;
}

IAudioSystemControl *CAudioSystemEditor_FMOD::GetControlByName(AZStd::string name, bool isLocalized, AudioControls::IAudioSystemControl *parent) const
{
    //Used likely by the Return and Soundbank controls.
    auto pCtrl = parent ? parent : &m_rootControl;
    auto fullName = name;

    AZStd::string basePath = "";
    if(isLocalized)
    {
        AZ::StringFunc::Path::Join(pCtrl->GetName().c_str(), m_currentLanguageName.c_str(), basePath);
        AZ::StringFunc::Path::Join(basePath.c_str(), name.c_str(), fullName);
    }
    else
    {
        AZ::StringFunc::Path::Join(pCtrl->GetName().c_str(), name.c_str(), fullName);
    }

    return GetControl(Audio::AudioStringToID<CID>(fullName.data()));
}

IAudioSystemControl* CAudioSystemEditor_FMOD::GetControlByPath(const AZStd::string& name,
                                                                     AudioControls::TImplControlType type)
{
    IAudioSystemControl* pCtrl = GetRootControlForType(type);
    auto cleanName = StripPrefixFromPath(name);
    AZStd::vector<AZStd::string> tokens;
    AZ::StringFunc::Tokenize(cleanName, tokens, '/');

    for (const auto& token : tokens)
    {
        IAudioSystemControl* child = nullptr;
        for (size_t i = 0; i < pCtrl->ChildCount(); ++i)
        {
            auto nextChild = pCtrl->GetChildAt(i);
            if (nextChild && nextChild->GetName() == token)
            {
                child = nextChild;
                break;
            }
        }
        if (child == nullptr)
            return nullptr;

        pCtrl = child;
    }
    return pCtrl;
}
AudioControls::IAudioSystemControl* CAudioSystemEditor_FMOD::CreatePlaceholderFolderPath(const AZStd::string& path,
                                                                                         AudioControls::TImplControlType type)
{
    IAudioSystemControl* pCtrl = GetRootControlForType(type);
    const auto cleanPath = StripPrefixFromPath(path);
    if (cleanPath.empty())
    {
        return pCtrl;
    }

    AZStd::vector<AZStd::string> tokens;
    AZ::StringFunc::Tokenize(cleanPath, tokens, '/');

    for (const auto& token : tokens)
    {
        IAudioSystemControl* foundChild = nullptr;
        for (size_t i = 0; i < pCtrl->ChildCount(); ++i)
        {
            if (IAudioSystemControl* child = pCtrl->GetChildAt(i); child && child->GetName() == token)
            {
                foundChild =child;
                break;
            }
        }
        if (!foundChild)
        {
            foundChild = CreateControl(SControlDef(token, eFMOD_FOLDER, false, pCtrl));
            foundChild->SetPlaceholder(true);
        }
        pCtrl = foundChild;
    }

    return pCtrl;
}

AudioControls::IAudioSystemControl* CAudioSystemEditor_FMOD::GetRootControlForType(const AudioControls::TImplControlType type)
{
    IAudioSystemControl* pRoot = nullptr;
    switch (type)
    {
    case eFMOD_EVENT:
        pRoot = &m_eventsControl;
        break;
    case eFMOD_SNAPSHOT:
        pRoot = &m_snapshotsCtrl;
        break;
    case eFMOD_GLOBAL_PARAMETER:
        pRoot = &m_globalParamsCtrl;
        break;
    case eFMOD_SOUNDBANK:
        pRoot = &m_banksControl;
        break;
    case eFMOD_RETURN:
        pRoot = &m_returnsControl;
        break;
    default:
        pRoot = &m_rootControl;
        break;
    }

    return pRoot;
}

void CAudioSystemEditor_FMOD::UpdateConnectedStatus()
{
    for (const auto& idCountPair : m_connectionsByID)
    {
        if (idCountPair.second > 0)
        {
            auto* control = GetControl(idCountPair.first);
            if (control)
            {
                control->SetConnected(true);
            }
        }
    }
}
}
