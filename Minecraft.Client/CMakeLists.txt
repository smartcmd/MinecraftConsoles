include("${CMAKE_CURRENT_LIST_DIR}/cmake/sources/Common.cmake")
include("${CMAKE_CURRENT_LIST_DIR}/cmake/sources/Durango.cmake")
include("${CMAKE_CURRENT_LIST_DIR}/cmake/sources/ORBIS.cmake")
include("${CMAKE_CURRENT_LIST_DIR}/cmake/sources/PS3.cmake")
include("${CMAKE_CURRENT_LIST_DIR}/cmake/sources/PSVita.cmake")
include("${CMAKE_CURRENT_LIST_DIR}/cmake/sources/Windows.cmake")
include("${CMAKE_CURRENT_LIST_DIR}/cmake/sources/Xbox360.cmake")

include("${CMAKE_SOURCE_DIR}/cmake/CommonSources.cmake")

# Combine all source files into a single variable for the target
# We cant use CMAKE_CONFIGURE_PRESET here as VS doesn't set it, so just rely on the folder
set(MINECRAFT_CLIENT_SOURCES
  ${MINECRAFT_CLIENT_COMMON}
  $<$<STREQUAL:${PLATFORM_FOLDER},Durango>:${MINECRAFT_CLIENT_DURANGO}>
  $<$<STREQUAL:${PLATFORM_FOLDER},Orbis>:${MINECRAFT_CLIENT_ORBIS}>
  $<$<STREQUAL:${PLATFORM_FOLDER},PS3>:${MINECRAFT_CLIENT_PS3}>
  $<$<STREQUAL:${PLATFORM_FOLDER},PSVita>:${MINECRAFT_CLIENT_PSVITA}>
  $<$<STREQUAL:${PLATFORM_FOLDER},Windows64>:${MINECRAFT_CLIENT_WINDOWS}>
  $<$<STREQUAL:${PLATFORM_FOLDER},Xbox360>:${MINECRAFT_CLIENT_XBOX360}>
  ${SOURCES_COMMON}
)

add_executable(Minecraft.Client WIN32 ${MINECRAFT_CLIENT_SOURCES})

target_include_directories(Minecraft.Client PRIVATE
  "${CMAKE_BINARY_DIR}/generated/" # This is for the generated BuildVer.h
  "${CMAKE_CURRENT_SOURCE_DIR}"
  "${CMAKE_CURRENT_SOURCE_DIR}/${PLATFORM_FOLDER}/Iggy/include"
  "${CMAKE_SOURCE_DIR}/include/"
)
target_compile_definitions(Minecraft.Client PRIVATE
  ${MINECRAFT_SHARED_DEFINES}
)
target_precompile_headers(Minecraft.Client PRIVATE "$<$<COMPILE_LANGUAGE:CXX>:stdafx.h>")
set_source_files_properties(compat_shims.cpp PROPERTIES SKIP_PRECOMPILE_HEADERS ON) # This redefines internal MSVC CRT symbols which will cause an issue with PCH
if(MSVC)
  configure_msvc_target(Minecraft.Client)
  target_link_options(Minecraft.Client PRIVATE
    $<$<CONFIG:Release>:/LTCG:incremental>
  )
endif()

set_target_properties(Minecraft.Client PROPERTIES
  VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}"
)

target_link_libraries(Minecraft.Client PRIVATE
  Minecraft.World
  d3d11
  d3dcompiler
  XInput9_1_0
  wsock32
  legacy_stdio_definitions
  "${CMAKE_CURRENT_SOURCE_DIR}/${PLATFORM_FOLDER}/Iggy/lib/iggy_w64.lib"
  "${CMAKE_CURRENT_SOURCE_DIR}/${PLATFORM_FOLDER}/Iggy/lib/iggyperfmon_w64.lib"
  "${CMAKE_CURRENT_SOURCE_DIR}/${PLATFORM_FOLDER}/Iggy/lib/iggyexpruntime_w64.lib"
  $<$<CONFIG:Debug>: # Debug 4J libraries
    "${CMAKE_CURRENT_SOURCE_DIR}/${PLATFORM_FOLDER}/4JLibs/libs/4J_Input_d.lib"
    "${CMAKE_CURRENT_SOURCE_DIR}/${PLATFORM_FOLDER}/4JLibs/libs/4J_Storage_d.lib"
    "${CMAKE_CURRENT_SOURCE_DIR}/${PLATFORM_FOLDER}/4JLibs/libs/4J_Render_PC_d.lib"
  >
  $<$<NOT:$<CONFIG:Debug>>: # Release 4J libraries
    "${CMAKE_CURRENT_SOURCE_DIR}/${PLATFORM_FOLDER}/4JLibs/libs/4J_Input.lib"
    "${CMAKE_CURRENT_SOURCE_DIR}/${PLATFORM_FOLDER}/4JLibs/libs/4J_Storage.lib"
    "${CMAKE_CURRENT_SOURCE_DIR}/${PLATFORM_FOLDER}/4JLibs/libs/4J_Render_PC.lib"
  >
)

# ---
# Asset / redist copy
# ---
include("${CMAKE_CURRENT_SOURCE_DIR}/cmake/AssetCopyTargets.cmake")
setup_asset_copy_targets()

# Copy redist files
add_custom_target(CopyRedist ALL
  COMMAND ${CMAKE_COMMAND}
    "-DCOPY_SOURCE=${CMAKE_SOURCE_DIR}/x64/Release"
    "-DCOPY_DEST=${CMAKE_CURRENT_BINARY_DIR}/$<CONFIG>"
    -P "${COPY_SCRIPT}"
  COMMENT "Copying redist files..."
  VERBATIM
)

add_dependencies(Minecraft.Client CopyRedist)
set_property(TARGET CopyRedist PROPERTY FOLDER "Build")

# Make sure GameHDD exists on Windows
if(CMAKE_CONFIGURE_PRESET STREQUAL "windows64")
  add_custom_target(EnsureGameHDD ALL
    COMMAND ${CMAKE_COMMAND}
      -E make_directory "${CMAKE_CURRENT_BINARY_DIR}/$<CONFIG>/Windows64/GameHDD"
    COMMENT "Ensuring GameHDD directory exists..."
    VERBATIM
  )

  add_dependencies(Minecraft.Client EnsureGameHDD)
  set_property(TARGET EnsureGameHDD PROPERTY FOLDER "Build")
endif()
