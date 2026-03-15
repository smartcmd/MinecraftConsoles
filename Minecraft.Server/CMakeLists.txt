# Note: Alot of this file is the same as the client due to the shared code

include("${CMAKE_CURRENT_LIST_DIR}/cmake/sources/Common.cmake")

include("${CMAKE_SOURCE_DIR}/cmake/CommonSources.cmake")

# Combine all source files into a single variable for the target
# We cant use CMAKE_CONFIGURE_PRESET here as VS doesn't set it, so just rely on the folder
set(MINECRAFT_SERVER_SOURCES
  ${MINECRAFT_SERVER_COMMON}
  ${SOURCES_COMMON}
)

add_executable(Minecraft.Server ${MINECRAFT_SERVER_SOURCES})

target_include_directories(Minecraft.Server PRIVATE
  "${CMAKE_BINARY_DIR}/generated/" # This is for the generated BuildVer.h
  "${CMAKE_SOURCE_DIR}/Minecraft.Client/"
  "${CMAKE_SOURCE_DIR}/Minecraft.Client/${PLATFORM_NAME}/Iggy/include"
  "${CMAKE_CURRENT_SOURCE_DIR}"
  "${CMAKE_SOURCE_DIR}/include/"
)
target_compile_definitions(Minecraft.Server PRIVATE
  ${MINECRAFT_SHARED_DEFINES}
  MINECRAFT_SERVER_BUILD
)
target_precompile_headers(Minecraft.Server PRIVATE "$<$<COMPILE_LANGUAGE:CXX>:stdafx.h>")
set_source_files_properties("${CMAKE_SOURCE_DIR}/Minecraft.Client/compat_shims.cpp" PROPERTIES SKIP_PRECOMPILE_HEADERS ON) # This redefines internal MSVC CRT symbols which will cause an issue with PCH

configure_compiler_target(Minecraft.Server)

set_target_properties(Minecraft.Server PROPERTIES
  OUTPUT_NAME "Minecraft.Server"
  VS_DEBUGGER_WORKING_DIRECTORY "$<TARGET_FILE_DIR:Minecraft.Server>"
  VS_DEBUGGER_COMMAND_ARGUMENTS "-port 25565 -bind 0.0.0.0 -name DedicatedServer"
)

target_link_libraries(Minecraft.Server PRIVATE
  Minecraft.World
  d3d11
  d3dcompiler
  XInput9_1_0
  wsock32
  legacy_stdio_definitions
  $<$<CONFIG:Debug>: # Debug 4J libraries
    "${CMAKE_SOURCE_DIR}/Minecraft.Client/${PLATFORM_NAME}/4JLibs/libs/4J_Input_d.lib"
    "${CMAKE_SOURCE_DIR}/Minecraft.Client/${PLATFORM_NAME}/4JLibs/libs/4J_Storage_d.lib"
    "${CMAKE_SOURCE_DIR}/Minecraft.Client/${PLATFORM_NAME}/4JLibs/libs/4J_Render_PC_d.lib"
  >
  $<$<NOT:$<CONFIG:Debug>>: # Release 4J libraries
    "${CMAKE_SOURCE_DIR}/Minecraft.Client/${PLATFORM_NAME}/4JLibs/libs/4J_Input.lib"
    "${CMAKE_SOURCE_DIR}/Minecraft.Client/${PLATFORM_NAME}/4JLibs/libs/4J_Storage.lib"
    "${CMAKE_SOURCE_DIR}/Minecraft.Client/${PLATFORM_NAME}/4JLibs/libs/4J_Render_PC.lib"
  >
)

# Iggy libs
foreach(lib IN LISTS IGGY_LIBS)
  target_link_libraries(Minecraft.Server PRIVATE "${CMAKE_SOURCE_DIR}/Minecraft.Client/${PLATFORM_NAME}/Iggy/lib/${lib}")
endforeach()

# ---
# Asset / redist copy
# ---
include("${CMAKE_SOURCE_DIR}/cmake/CopyAssets.cmake")

# Copy res
set(ASSET_FOLDER_PAIRS
  "${CMAKE_SOURCE_DIR}/Minecraft.Client/Common/res"         "Common/res"
)
setup_asset_folder_copy(Minecraft.Server "${ASSET_FOLDER_PAIRS}")

# Copy arc media
set(ASSET_FILES_PAIRS
  "${CMAKE_SOURCE_DIR}/Minecraft.Client/Common/Media/MediaWindows64.arc" "Common/Media/"
)
setup_asset_file_copy(Minecraft.Server "${ASSET_FILES_PAIRS}")

# Copy redist files
add_copyredist_target(Minecraft.Server)

# Make sure GameHDD exists on Windows
if(PLATFORM_NAME STREQUAL "Windows64")
  add_gamehdd_target(Minecraft.Server)
endif()
