#pragma once

#include <IAudioSystemEditor.h>
#include <IAudioConnection.h>
#include <IAudioSystemControl.h>

#include "AudioFMODDataLoader.h"
#include <AzCore/std/smart_ptr/shared_ptr.h>

namespace AudioEngineFMOD
{
    class CAudioSystemEditor_FMOD : public AudioControls::IAudioSystemEditor
    {
    public:
        CAudioSystemEditor_FMOD();
        ~CAudioSystemEditor_FMOD() override = default;

        // IAudioSystemEditor interface
        void Reload() override;
        AudioControls::IAudioSystemControl *CreateControl(const AudioControls::SControlDef &controlDefinition) override;
        AudioControls::IAudioSystemControl *GetRoot() override;
        AudioControls::IAudioSystemControl *GetControl(AudioControls::CID id) const override;
        AudioControls::EACEControlType ImplTypeToATLType(AudioControls::TImplControlType type) const override;
        AudioControls::TImplControlTypeMask GetCompatibleTypes(AudioControls::EACEControlType atlControlType) const override;
        AudioControls::TConnectionPtr CreateConnectionToControl(AudioControls::EACEControlType atlControlType, AudioControls::IAudioSystemControl *middlewareControl) override;
        AudioControls::TConnectionPtr CreateConnectionFromXMLNode(AZ::rapidxml::xml_node<char> *node, AudioControls::EACEControlType atlControlType) override;
        AZ::rapidxml::xml_node<char> *CreateXMLNodeFromConnection(const AudioControls::TConnectionPtr connection, const AudioControls::EACEControlType atlControlType) override;
        const AZStd::string_view GetTypeIcon(AudioControls::TImplControlType type) const override;
        const AZStd::string_view GetTypeIconSelected(AudioControls::TImplControlType type) const override;
        AZStd::string GetName() const override;
        AZ::IO::FixedMaxPath GetDataPath() const override;
        void DataSaved() override;
        QWidget* CreateConnectionPropertiesWidget(const AudioControls::TConnectionPtr connection, AudioControls::EACEControlType atlControlType) override;

    private:
        AudioControls::IAudioSystemControl* GetControlByName(AZStd::string name, bool isLocalized, AudioControls::IAudioSystemControl* parent = nullptr) const;
        using TControlPtr = AZStd::shared_ptr<AudioControls::IAudioSystemControl>;
        using TControlMap = AZStd::unordered_map<AudioControls::CID, TControlPtr>;
        TControlMap m_controls;

        using TConnectionsMap = AZStd::unordered_map<AudioControls::CID, int>;
        TConnectionsMap m_connectionsByID;

        AudioControls::IAudioSystemControl m_rootControl;

        CAudioFMODDataLoader m_loader;
    };
}