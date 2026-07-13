
#pragma once

namespace AudioEngineFMOD
{
    // System Component TypeIds
    inline constexpr const char* AudioEngineFMODSystemComponentTypeId = "{1566931B-F4B4-4D54-ACF1-93B0955F1B3D}";
    inline constexpr const char* AudioEngineFMODEditorSystemComponentTypeId = "{FB3335FD-9819-4A86-B0F3-EB68481DD483}";

    // Module derived classes TypeIds
    inline constexpr const char* AudioEngineFMODModuleInterfaceTypeId = "{4F0B9A91-CE11-4A4D-9F8F-C9333E87869B}";
    inline constexpr const char* AudioEngineFMODModuleTypeId = "{C8CFAA2C-4812-4F4F-8616-33D7D0A47D64}";
    // The Editor Module by default is mutually exclusive with the Client Module
    // so they use the Same TypeId
    inline constexpr const char* AudioEngineFMODEditorModuleTypeId = AudioEngineFMODModuleTypeId;

    // Interface TypeIds
    inline constexpr const char* AudioEngineFMODRequestsTypeId = "{3403053C-02F2-49FA-B316-9379009D5ED2}";
} // namespace AudioEngineFMOD
