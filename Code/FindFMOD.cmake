# - Find fmod
# Find the fmod includes and library
#
#  FMOD_INCLUDE_DIR - Where to find fmod includes
#  FMOD_LIBRARIES   - List of libraries when using fmod
#  FMOD_FOUND       - True if fmod was found

# To use in an O3DE Gem, place this code in the ROOT gem directory, CMakeLists.txt
#   list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/Code")
#   set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} PARENT_SCOPE)

SET(FMOD_TARGET_ARCH "x86_64") # Default.
if(CMAKE_SYSTEM_PROCESSOR MATCHES "amd64|x86_64|AMD64")
	if(NOT WIN32)
		set(FMOD_TARGET_ARCH "x86_64")
	else()
		set(FMOD_TARGET_ARCH "x64")
	endif()
elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "arm64|aarch64")
    set(FMOD_TARGET_ARCH "arm64")
else()
    message(WARNING "${CMAKE_SYSTEM_PROCESSOR} is not supported by FMOD, defaulting to x86_64")
endif()


find_path(FMOD_CORE_INCLUDE_DIR "fmod.hpp"
    PATHS
    $ENV{FMOD_HOME}/api
    ${FMOD_ROOT}/api
    PATH_SUFFIXES
    core/inc
    DOC "FMOD SDK - Core Headers Location"
)

find_path(FMOD_STUDIO_INCLUDE_DIR "fmod_studio.hpp"
    PATHS
    $ENV{FMOD_HOME}/api
    ${FMOD_ROOT}/api
    PATH_SUFFIXES
    studio/inc
    DOC "FMOD SDK - Studio Headers Location"
)

find_library(FMOD_CORE_LIBRARY NAMES fmod fmod_vc
    PATHS
    $ENV{FMOD_HOME}/api
    ${FMOD_ROOT}/api
    PATH_SUFFIXES
    core/lib/${FMOD_TARGET_ARCH}
    DOC "FMOD SDK -  Core Library"
)


find_library(FMOD_STUDIO_LIBRARY NAMES fmodstudio fmodstudio_vc
    PATHS
    $ENV{FMOD_HOME}/api
    ${FMOD_ROOT}/api
    PATH_SUFFIXES
    studio/lib/${FMOD_TARGET_ARCH}
    DOC "FMOD SDK - Studio Library"
)


find_library(FMOD_CORE_LIBRARY_LOGGING NAMES fmodL fmodL_vc
    PATHS
    $ENV{FMOD_HOME}/api
    ${FMOD_ROOT}/api
    PATH_SUFFIXES
    core/lib/${FMOD_TARGET_ARCH}
    DOC "FMOD SDK - Core Library (Logging Enabled)"
)


find_library(FMOD_STUDIO_LIBRARY_LOGGING NAMES fmodstudioL fmodstudioL_vc
    PATHS
    $ENV{FMOD_HOME}/api
    ${FMOD_ROOT}/api
    PATH_SUFFIXES
    studio/lib/${FMOD_TARGET_ARCH}
    DOC "FMOD SDK - Core Library (Logging Enabled)"
)

# Runtime DLLS (Windows Only)
if(WIN32)

find_file(FMOD_CORE_DLL NAMES fmod.dll
	PATHS
    $ENV{FMOD_HOME}/api
    ${FMOD_ROOT}/api
    PATH_SUFFIXES
    core/lib/${FMOD_TARGET_ARCH}
	DOC "FMOD SDK - Core Runtime DLL."
)

find_file(FMOD_CORE_LOGGING_DLL NAMES fmodL.dll
	PATHS
    $ENV{FMOD_HOME}/api
    ${FMOD_ROOT}/api
    PATH_SUFFIXES
    core/lib/${FMOD_TARGET_ARCH}
	DOC "FMOD SDK - Core Runtime DLL (Logging Enabled)."
)


find_file(FMOD_STUDIO_DLL NAMES fmodstudio.dll
	PATHS
    $ENV{FMOD_HOME}/api
    ${FMOD_ROOT}/api
    PATH_SUFFIXES
    studio/lib/${FMOD_TARGET_ARCH}
	DOC "FMOD SDK - Studio Runtime DLL."
)

find_file(FMOD_STUDIO_LOGGING_DLL NAMES fmodstudioL.dll
	PATHS
    $ENV{FMOD_HOME}/api
    ${FMOD_ROOT}/api
    PATH_SUFFIXES
    studio/lib/${FMOD_TARGET_ARCH}
	DOC "FMOD SDK - Studio Runtime DLL (Logging Enabled)."
)

endif()

INCLUDE(FindPackageHandleStandardArgs)

set(FMOD_LIBRARIES ${FMOD_CORE_LIBRARY} ${FMOD_STUDIO_LIBRARY})
set(FMOD_INCLUDE_DIRS ${FMOD_CORE_INCLUDE_DIR} ${FMOD_STUDIO_INCLUDE_DIR})

find_package_handle_standard_args(FMOD DEFAULT_MSG FMOD_LIBRARIES FMOD_INCLUDE_DIRS)

#TODO: Deal with the Logging versions of the libraries, (enable when debug mode?)
if(FMOD_FOUND)
    file(STRINGS "${FMOD_CORE_INCLUDE_DIR}/fmod_common.h" FMOD_VERSION_LINE REGEX "^#define[ \t]+FMOD_VERSION[ \t]+0x[0-9A-Fa-f]+")
    string(REGEX REPLACE
       "^#define[ \t]+FMOD_VERSION[ \t]+0x([0-9A-Fa-f]+).*"
       "\\1"
       FMOD_VERSION_HEX
       "${FMOD_VERSION_LINE}")

    string(SUBSTRING "${FMOD_VERSION_HEX}" 0 4 PRODUCT_VERSION_HEX)
    string(SUBSTRING "${FMOD_VERSION_HEX}" 4 2 MAJOR_VERSION_HEX)
    string(SUBSTRING "${FMOD_VERSION_HEX}" 6 2 MINOR_VERSION_HEX)

    math(EXPR PRODUCT_VERSION "0x${PRODUCT_VERSION_HEX}")
    math(EXPR MAJOR_VERSION   "0x${MAJOR_VERSION_HEX}")
    math(EXPR MINOR_VERSION   "0x${MINOR_VERSION_HEX}")

    message(STATUS "FMOD SDK Version Found = ${PRODUCT_VERSION}.${MAJOR_VERSION}.${MINOR_VERSION}")
    message(STATUS " - FMOD Library: ${FMOD_CORE_LIBRARY}, FMOD Studio Library: ${FMOD_STUDIO_LIBRARY}")
	message(STATUS " - FMOD Logging Library: ${FMOD_CORE_LIBRARY_LOGGING}, FMOD Studio LOGGING Library: ${FMOD_STUDIO_LIBRARY_LOGGING}")
	message(STATUS " - FMOD Core Include Directory: ${FMOD_CORE_INCLUDE_DIR}")
	message(STATUS " - FMOD Studio Include Directory: ${FMOD_STUDIO_INCLUDE_DIR}")
	if(WIN32)
		message(STATUS " - FMOD Core DLL: ${FMOD_CORE_DLL}, Logging: ${FMOD_CORE_LOGGING_DLL}")
		message(STATUS " - FMOD Studio DLL: ${FMOD_STUDIO_DLL}, Logging: ${FMOD_STUDIO_LOGGING_DLL}")
	endif()

    add_library(3rdParty::FMOD UNKNOWN IMPORTED GLOBAL)
	if(WIN32)
		set_target_properties(3rdParty::FMOD PROPERTIES
			INTERFACE_INCLUDE_DIRECTORIES "${FMOD_CORE_INCLUDE_DIR}"
			IMPORTED_LOCATION "${FMOD_CORE_DLL}"
			IMPORTED_LOCATION_DEBUG "${FMOD_CORE_LOGGING_DLL}"
			IMPORTED_IMPLIB "${FMOD_CORE_LIBRARY}"
			IMPORTED_IMPLIB_DEBUG "${FMOD_CORE_LIBRARY_LOGGING}")
	else()
		set_target_properties(3rdParty::FMOD PROPERTIES
			INTERFACE_INCLUDE_DIRECTORIES "${FMOD_CORE_INCLUDE_DIR}"
			IMPORTED_LOCATION "${FMOD_CORE_LIBRARY}"
			IMPORTED_LOCATION_DEBUG "${FMOD_CORE_LIBRARY_LOGGING}")
	endif()
    ly_target_include_system_directories(TARGET 3rdParty::FMOD INTERFACE ${FMOD_CORE_INCLUDE_DIR})
    
	add_library(3rdParty::FMODStudio UNKNOWN IMPORTED GLOBAL)
	if(WIN32)
		set_target_properties(3rdParty::FMODStudio PROPERTIES
			INTERFACE_INCLUDE_DIRECTORIES "${FMOD_STUDIO_INCLUDE_DIR}"
			IMPORTED_LOCATION "${FMOD_STUDIO_DLL}"
			IMPORTED_LOCATION_DEBUG "${FMOD_STUDIO_LOGGING_DLL}"
			IMPORTED_IMPLIB "${FMOD_STUDIO_LIBRARY}"
			IMPORTED_IMPLIB_DEBUG "${FMOD_STUDIO_LIBRARY_LOGGING}")
	else()
	    set_target_properties(3rdParty::FMODStudio PROPERTIES
			INTERFACE_INCLUDE_DIRECTORIES "${FMOD_STUDIO_INCLUDE_DIR}"
			IMPORTED_LOCATION "${FMOD_STUDIO_LIBRARY}"
			IMPORTED_LOCATION_DEBUG "${FMOD_STUDIO_LIBRARY_LOGGING}")
	endif()
    ly_target_include_system_directories(TARGET 3rdParty::FMODStudio INTERFACE ${FMOD_STUDIO_INCLUDE_DIR})
endif()
