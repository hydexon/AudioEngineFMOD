#include "ATLEntities_FMOD.h"

#include "Common_FMOD.h"
#include <AzCore/std/string/string.h>
#include <AzCore/StringFunc/StringFunc.h>

namespace AudioEngineFMOD
{

bool SATLAudioRtpcImplData_FMOD::ReadFromXml(const AZ::rapidxml::xml_node<char> &node)
{
    //Check and Process for Three Things: Global Parameters, Per-Object Parameters, and Single-Event Parameters.
    m_type = RtpcImpl::Unknown;

    //Single-Event Parameter.
    if(AZ::StringFunc::Equal(node.name(), XMLTags::FMODEventTag))
    {
        m_type = RtpcImpl::SingleEvent;
    }
    else if(AZ::StringFunc::Equal(node.name(), XMLTags::FMODParameterTag))
    {
        auto isGlobalAttr = node.first_attribute(XMLTags::FMODIsGlobalParam);

        m_type = RtpcImpl::PerObject;
        if(AZ::StringFunc::Equal(isGlobalAttr->value(), "true"))
        {
            m_type = RtpcImpl::Global;
        }
    }

    //TODO: open for improvement.
    if(m_type == RtpcImpl::SingleEvent)
    {
        if(auto paramAttr = node.first_attribute(XMLTags::FMODParamAttr);
                paramAttr != nullptr)
        {
            m_paramPath = paramAttr->value();
        }

        if(auto pathAttr = node.first_attribute(XMLTags::FMODPathAttribute);
                pathAttr != nullptr)
        {
            m_singleEvtPath = pathAttr->value();
        }

        if(auto initVal = node.first_attribute(XMLTags::FMODPathAttribute);
                initVal != nullptr)
        {
            m_initialValue = AZ::StringFunc::ToFloat(initVal->value());
        }


        return true;
    }
    else
    {
        if(auto pathAttr = node.first_attribute(XMLTags::FMODPathAttribute);
                pathAttr != nullptr)
        {
            m_paramPath = pathAttr->value();
        }

        if(auto initVal = node.first_attribute(XMLTags::FMODPathAttribute);
                initVal != nullptr)
        {
            m_initialValue = AZ::StringFunc::ToFloat(initVal->value());
        }

        return true;
    }

    return false;
}

}