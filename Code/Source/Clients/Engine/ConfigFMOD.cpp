#include "ConfigFMOD.h"

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


}