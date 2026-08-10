#include "ATLEntities_FMOD.h"

#include "Common_FMOD.h"
#include <AzCore/std/string/string.h>
#include <AzCore/StringFunc/StringFunc.h>

namespace AudioEngineFMOD
{

bool SATLAudioRtpcImplData_FMOD::ReadFromXml(const AZ::rapidxml::xml_node<char> &node)
{
    if(AZ::StringFunc::Equal(node.name(), XMLTags::FMODParameterTag))
    {
        auto isGlobalAttr = node.first_attribute(XMLTags::FMODIsGlobalParam);

        m_isGlobal = AZ::StringFunc::Equal(isGlobalAttr->value(), "true");
    }
    else
    {
        return false;
    }

    if(auto paramAttr = node.first_attribute(XMLTags::FMODPathAttribute);
            paramAttr != nullptr)
    {
        m_paramPath = paramAttr->value();
    }

    /*
    if(auto initVal = node.first_attribute(XMLTags::FMODInitialValue);
            initVal != nullptr)
    {
        m_initialValue = AZ::StringFunc::ToFloat(initVal->value());
    }*/

    return true;
}

}