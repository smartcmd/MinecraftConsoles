cmake_minimum_required(VERSION 3.24)

project(MinecraftConsoles LANGUAGES C CXX RC ASM_MASM)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(NOT WIN32)
  message(FATAL_ERROR "This CMake build currently supports Windows only.")
endif()

if(NOT CMAKE_SIZEOF_VOID_P EQUAL 8)
  message(FATAL_ERROR "Use a 64-bit generator/toolchain (x64).")
endif()

set(CMAKE_CONFIGURATION_TYPES
  "Debug"
  "Release"
  CACHE STRING "" FORCE
)
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

function(configure_msvc_target target)
  target_compile_options(${target} PRIVATE
    $<$<AND:$<CONFIG:Debug>,$<COMPILE_LANGUAGE:C,CXX>>:/W3>
    $<$<AND:$<CONFIG:Release>,$<COMPILE_LANGUAGE:C,CXX>>:/W0>
    $<$<COMPILE_LANGUAGE:C,CXX>:/MP>
    $<$<COMPILE_LANGUAGE:C,CXX>:/FS>
    $<$<COMPILE_LANGUAGE:CXX>:/EHsc>
    $<$<AND:$<CONFIG:Release>,$<COMPILE_LANGUAGE:C,CXX>>:/GL /O2 /Oi /GT /GF>
  )
endfunction()

# ---
# Configuration
# ---
set(MINECRAFT_SHARED_DEFINES
  _LARGE_WORLDS
  _DEBUG_MENUS_ENABLED
  $<$<CONFIG:Debug>:_DEBUG>
  _CRT_NON_CONFORMING_SWPRINTFS
  _CRT_SECURE_NO_WARNINGS
)

# Add platform-specific defines
list(APPEND MINECRAFT_SHARED_DEFINES ${PLATFORM_DEFINES})

set(WORLD_DIR  "${CMAKE_CURRENT_SOURCE_DIR}/Minecraft.World")
set(CLIENT_DIR "${CMAKE_CURRENT_SOURCE_DIR}/Minecraft.Client")

# ---
# Sources
# ---
add_subdirectory(Minecraft.World)
add_subdirectory(Minecraft.Client)

# ---
# MinecraftWorld
# ---
add_library(MinecraftWorld STATIC ${MINECRAFT_WORLD_SOURCES})
target_include_directories(MinecraftWorld
  PRIVATE "${WORLD_DIR}"
  PUBLIC  "${WORLD_DIR}/x64headers"
)
target_compile_definitions(MinecraftWorld PRIVATE
  ${MINECRAFT_SHARED_DEFINES}
  _LIB
)
if(MSVC)
  configure_msvc_target(MinecraftWorld)
endif()

# ---
# MinecraftClient
# ---
add_executable(MinecraftClient WIN32 ${MINECRAFT_CLIENT_SOURCES})
target_include_directories(MinecraftClient PRIVATE
  "${CLIENT_DIR}"
  "${CLIENT_DIR}/Windows64/Iggy/include"
  "${CLIENT_DIR}/Xbox/Sentient/Include"
  "${CMAKE_CURRENT_SOURCE_DIR}/include/"
)
target_compile_definitions(MinecraftClient PRIVATE
  ${MINECRAFT_SHARED_DEFINES}
)
if(MSVC)
  configure_msvc_target(MinecraftClient)
  target_link_options(MinecraftClient PRIVATE
    $<$<CONFIG:Release>:/LTCG /INCREMENTAL:NO>
  )
endif()

set_target_properties(MinecraftClient PROPERTIES
  VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}"
)

target_link_libraries(MinecraftClient PRIVATE
  MinecraftWorld
  d3d11
  XInput9_1_0
  wsock32
  legacy_stdio_definitions
  "${CLIENT_DIR}/Windows64/Iggy/lib/iggy_w64.lib"
  "${CLIENT_DIR}/Windows64/Iggy/lib/iggyperfmon_w64.lib"
  "${CLIENT_DIR}/Windows64/Iggy/lib/iggyexpruntime_w64.lib"
  $<$<CONFIG:Debug>: # Debug 4J libraries
    "${CLIENT_DIR}/Windows64/4JLibs/libs/4J_Input_d.lib"
    "${CLIENT_DIR}/Windows64/4JLibs/libs/4J_Storage_d.lib"
    "${CLIENT_DIR}/Windows64/4JLibs/libs/4J_Render_PC_d.lib"
  >
  $<$<NOT:$<CONFIG:Debug>>: # Release 4J libraries
    "${CLIENT_DIR}/Windows64/4JLibs/libs/4J_Input.lib"
    "${CLIENT_DIR}/Windows64/4JLibs/libs/4J_Storage.lib"
    "${CLIENT_DIR}/Windows64/4JLibs/libs/4J_Render_PC.lib"
  >
)

# ---
# Build versioning
# ---
set(BUILDVER_SCRIPT "${CMAKE_CURRENT_SOURCE_DIR}/cmake/GenerateBuildVer.cmake")

add_custom_target(GenerateBuildVer
  COMMAND ${CMAKE_COMMAND}
    "-DOUTPUT_FILE=${CLIENT_DIR}/Common/BuildVer.h"
    -P "${BUILDVER_SCRIPT}"
  COMMENT "Generating BuildVer.h..."
  VERBATIM
)

add_dependencies(MinecraftClient GenerateBuildVer)

add_custom_command(TARGET MinecraftClient POST_BUILD
  COMMAND git restore "**/BuildVer.h"
  WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
  COMMENT "Restoring BuildVer.h..."
  VERBATIM
)

# ---
# Asset / redist copy
# ---
include("${CMAKE_CURRENT_SOURCE_DIR}/cmake/AssetCopyTargets.cmake")
setup_asset_copy_targets()

# Copy redist files
add_custom_target(CopyRedist ALL
  COMMAND ${CMAKE_COMMAND}
    "-DCOPY_SOURCE=${CMAKE_CURRENT_SOURCE_DIR}/x64/Release"
    "-DCOPY_DEST=${CMAKE_CURRENT_BINARY_DIR}/$<CONFIG>"
    -P "${COPY_SCRIPT}"
  COMMENT "Copying redist files..."
  VERBATIM
)

add_dependencies(MinecraftClient CopyRedist)

# ---
# Project organisation
# ---
# Set the startup project for Visual Studio
set_property(DIRECTORY PROPERTY VS_STARTUP_PROJECT MinecraftClient)

# Setup folders for Visual Studio, just hides the build targets under a sub folder
set_property(GLOBAL PROPERTY USE_FOLDERS ON)
set_property(TARGET CopyRedist GenerateBuildVer PROPERTY FOLDER "Build")
