set(CMAKE_SYSTEM_NAME Windows)

set(CMAKE_C_COMPILER clang-cl)
set(CMAKE_CXX_COMPILER clang-cl)
set(CMAKE_LINKER lld-link)

set(CMAKE_C_COMPILER_FRONTEND_VARIANT MSVC)
set(CMAKE_CXX_COMPILER_FRONTEND_VARIANT MSVC)

set(WINDOWS_SDK_PATH "" CACHE PATH "Path to Windows SDK root")
set(CMAKE_TRY_COMPILE_PLATFORM_VARIABLES WINDOWS_SDK_PATH)
# You have no idea how much suffering I went through to get this solution.
# I hate this build system so much

message(STATUS "WINDOWS_SDK_PATH=${WINDOWS_SDK_PATH}")

if(NOT WINDOWS_SDK_PATH)
    if(DEFINED ENV{WINDOWS_SDK_PATH})
        # rm: check if defined within env instead
        set(WINDOWS_SDK_PATH $ENV{WINDOWS_SDK_PATH})
    else()
        # rm: not found? check to find msbuild wrapper from mscv-wine
        find_program(_msbuild_exe msbuild)
        if(_msbuild_exe)
            get_filename_component(_msbuild_bin ${_msbuild_exe} DIRECTORY)   # .../bin/x64
            get_filename_component(_candidate   "${_msbuild_bin}/.."  ABSOLUTE) # .../bin
            if(NOT IS_DIRECTORY "${_candidate}/vc")
                get_filename_component(_candidate "${_candidate}/.." ABSOLUTE)  # .../<root>
            endif()
            message(STATUS "Found Windows SDK Path in ${_candidate}")
            set(WINDOWS_SDK_PATH ${_candidate})
        else()
            # rm: assume /opt/msvc as a fallback (which is usually the default path)
            message(STATUS "Fallback to ${_candidate}")
            set(WINDOWS_SDK_PATH /opt/msvc)
        endif()
    endif()
endif()

if(NOT EXISTS "${WINDOWS_SDK_PATH}/kits/10/Include")
    message(FATAL_ERROR "Invalid WINDOWS_SDK_PATH: ${WINDOWS_SDK_PATH}")
endif()

file(GLOB _winsdk_versions LIST_DIRECTORIES true
     "${WINDOWS_SDK_PATH}/kits/10/Include/*")

if(NOT _winsdk_versions)
    message(FATAL_ERROR "No Windows SDK versions found")
endif()

list(GET _winsdk_versions 0 _winsdk_dir)
get_filename_component(_winsdk_ver "${_winsdk_dir}" NAME)
add_compile_options(
    --target=x86_64-pc-windows-msvc
    -fms-compatibility
    -fms-extensions
    -fdelayed-template-parsing
    /winsysroot ${WINDOWS_SDK_PATH}
)

add_link_options(/winsysroot:${WINDOWS_SDK_PATH})

set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)
