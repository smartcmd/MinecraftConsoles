cmake_minimum_required(VERSION 3.24)

project(MinecraftConsoles LANGUAGES C CXX RC ASM_MASM)

if(NOT WIN32)
  message(FATAL_ERROR "This CMake build currently supports Windows only.")
endif()

if(NOT CMAKE_SIZEOF_VOID_P EQUAL 8)
  message(FATAL_ERROR "Use a 64-bit generator/toolchain (x64).")
endif()

set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

function(configure_msvc_target target)
  target_compile_options(${target} PRIVATE
    $<$<AND:$<NOT:$<CONFIG:Release>>,$<COMPILE_LANGUAGE:C,CXX>>:/W3>
    $<$<AND:$<CONFIG:Release>,$<COMPILE_LANGUAGE:C,CXX>>:/W0>
    $<$<COMPILE_LANGUAGE:C,CXX>:/MP>
    $<$<COMPILE_LANGUAGE:C,CXX>:/FS>
    $<$<COMPILE_LANGUAGE:CXX>:/EHsc>
    $<$<AND:$<CONFIG:Release>,$<COMPILE_LANGUAGE:C,CXX>>:/GL /O2 /Oi /GT /GF>
  )
endfunction()

include("${CMAKE_CURRENT_SOURCE_DIR}/cmake/WorldSources.cmake")
include("${CMAKE_CURRENT_SOURCE_DIR}/cmake/ClientSources.cmake")

list(TRANSFORM MINECRAFT_WORLD_SOURCES PREPEND "${CMAKE_CURRENT_SOURCE_DIR}/Minecraft.World/")
list(TRANSFORM MINECRAFT_CLIENT_SOURCES PREPEND "${CMAKE_CURRENT_SOURCE_DIR}/Minecraft.Client/")
list(APPEND MINECRAFT_CLIENT_SOURCES
  "${CMAKE_CURRENT_SOURCE_DIR}/Minecraft.Client/Xbox/MinecraftWindows.rc"
)

add_library(MinecraftWorld STATIC ${MINECRAFT_WORLD_SOURCES})
target_include_directories(MinecraftWorld PRIVATE
  "${CMAKE_CURRENT_SOURCE_DIR}/Minecraft.World"
  "${CMAKE_CURRENT_SOURCE_DIR}/Minecraft.World/x64headers"
)
target_compile_definitions(MinecraftWorld PRIVATE
  $<$<CONFIG:Debug>:_LARGE_WORLDS;_DEBUG_MENUS_ENABLED;_DEBUG;_LIB;_CRT_NON_CONFORMING_SWPRINTFS;_CRT_SECURE_NO_WARNINGS;_WINDOWS64>
  $<$<NOT:$<CONFIG:Debug>>:_LARGE_WORLDS;_DEBUG_MENUS_ENABLED;_LIB;_CRT_NON_CONFORMING_SWPRINTFS;_CRT_SECURE_NO_WARNINGS;_WINDOWS64>
)
if(MSVC)
  configure_msvc_target(MinecraftWorld)
endif()

add_executable(MinecraftClient WIN32 ${MINECRAFT_CLIENT_SOURCES})
target_include_directories(MinecraftClient PRIVATE
  "${CMAKE_CURRENT_SOURCE_DIR}/Minecraft.Client"
  "${CMAKE_CURRENT_SOURCE_DIR}/Minecraft.Client/Windows64/Iggy/include"
  "${CMAKE_CURRENT_SOURCE_DIR}/Minecraft.Client/Xbox/Sentient/Include"
  "${CMAKE_CURRENT_SOURCE_DIR}/Minecraft.World/x64headers"
)
target_compile_definitions(MinecraftClient PRIVATE
  $<$<CONFIG:Debug>:_LARGE_WORLDS;_DEBUG_MENUS_ENABLED;_DEBUG;_CRT_NON_CONFORMING_SWPRINTFS;_CRT_SECURE_NO_WARNINGS;_WINDOWS64>
  $<$<NOT:$<CONFIG:Debug>>:_LARGE_WORLDS;_DEBUG_MENUS_ENABLED;_CRT_NON_CONFORMING_SWPRINTFS;_CRT_SECURE_NO_WARNINGS;_WINDOWS64>
)
if(MSVC)
  configure_msvc_target(MinecraftClient)
  target_link_options(MinecraftClient PRIVATE
    $<$<CONFIG:Release>:/LTCG /INCREMENTAL:NO>
  )
endif()

set_target_properties(MinecraftClient PROPERTIES
  VS_DEBUGGER_WORKING_DIRECTORY "$<TARGET_FILE_DIR:MinecraftClient>"
)

target_link_libraries(MinecraftClient PRIVATE
  MinecraftWorld
  d3d11
  XInput9_1_0
  wsock32
  legacy_stdio_definitions
  "${CMAKE_CURRENT_SOURCE_DIR}/Minecraft.Client/Windows64/Iggy/lib/iggy_w64.lib"
  "${CMAKE_CURRENT_SOURCE_DIR}/Minecraft.Client/Windows64/Iggy/lib/iggyperfmon_w64.lib"
  "${CMAKE_CURRENT_SOURCE_DIR}/Minecraft.Client/Windows64/Iggy/lib/iggyexpruntime_w64.lib"
  "${CMAKE_CURRENT_SOURCE_DIR}/Minecraft.Client/Windows64/Miles/lib/mss64.lib"
  $<$<CONFIG:Debug>:
    "${CMAKE_CURRENT_SOURCE_DIR}/Minecraft.Client/Windows64/4JLibs/libs/4J_Input_d.lib"
    "${CMAKE_CURRENT_SOURCE_DIR}/Minecraft.Client/Windows64/4JLibs/libs/4J_Storage_d.lib"
    "${CMAKE_CURRENT_SOURCE_DIR}/Minecraft.Client/Windows64/4JLibs/libs/4J_Render_PC_d.lib"
  >
  $<$<NOT:$<CONFIG:Debug>>:
    "${CMAKE_CURRENT_SOURCE_DIR}/Minecraft.Client/Windows64/4JLibs/libs/4J_Input.lib"
    "${CMAKE_CURRENT_SOURCE_DIR}/Minecraft.Client/Windows64/4JLibs/libs/4J_Storage.lib"
    "${CMAKE_CURRENT_SOURCE_DIR}/Minecraft.Client/Windows64/4JLibs/libs/4J_Render_PC.lib"
  >
)

if(WIN32)
  message(STATUS "Starting redist copy...")
  execute_process(
    COMMAND robocopy.exe
            "${CMAKE_CURRENT_SOURCE_DIR}/x64/Release"
            "${CMAKE_CURRENT_BINARY_DIR}"
            /S /MT /R:0 /W:0 /NP
  )
  message(STATUS "Starting asset copy...")
  execute_process(
    COMMAND robocopy.exe
            "${CMAKE_CURRENT_SOURCE_DIR}/Minecraft.Client"
            "${CMAKE_CURRENT_BINARY_DIR}"
            /S /MT /R:0 /W:0 /NP
            /XF "*.cpp" "*.c" "*.h" "*.hpp" "*.asm"
                "*.xml" "*.lang" "*.vcxproj" "*.vcxproj.*" "*.sln"
                "*.docx" "*.xls"
                "*.bat" "*.cmd" "*.ps1" "*.py"
                "*Test*"
            /XD "Durango*" "Orbis*" "PS*" "Xbox"
  )
  message(STATUS "Patching Windows64Media...")
  execute_process(
    COMMAND robocopy.exe
            "${CMAKE_CURRENT_SOURCE_DIR}/Minecraft.Client/DurangoMedia"
            "${CMAKE_CURRENT_BINARY_DIR}/Windows64Media"
            /S /MT /R:0 /W:0 /NP
            /XF "*.h" "*.xml" "*.lang" "*.bat"
  )
elseif(UNIX)
  message(STATUS "Starting redist copy...")
  execute_process(
    COMMAND rsync -av "${CMAKE_CURRENT_SOURCE_DIR}/x64/Release/" "${CMAKE_CURRENT_BINARY_DIR}/"
  )
  message(STATUS "Starting asset copy...")
  execute_process(
    COMMAND rsync -av
            "--exclude=*.cpp" "--exclude=*.c" "--exclude=*.h" "--exclude=*.hpp" "--exclude=*.asm"
            "--exclude=*.xml" "--exclude=*.lang" "--exclude=*.vcxproj" "--exclude=*.vcxproj.*" "--exclude=*.sln"
            "--exclude=*.docx" "--exclude=*.xls"
            "--exclude=*.bat" "--exclude=*.cmd" "--exclude=*.ps1" "--exclude=*.py"
            "--exclude=*Test*"
            "--exclude=Durango*" "--exclude=Orbis*" "--exclude=PS*" "--exclude=Xbox"
            "${CMAKE_CURRENT_SOURCE_DIR}/Minecraft.Client/" "${CMAKE_CURRENT_BINARY_DIR}/"
  )
  message(STATUS "Patching Windows64Media...")
  execute_process(
    COMMAND rsync -av
            "--exclude=*.h" "--exclude=*.xml" "--exclude=*.lang" "--exclude=*.bat"
            "${CMAKE_CURRENT_SOURCE_DIR}/Minecraft.Client/DurangoMedia/" "${CMAKE_CURRENT_BINARY_DIR}/Windows64Media/"
  )
else()
  message(FATAL_ERROR "Redist and asset copying is only supported on Windows (Robocopy) and Unix systems (rsync).")
endif()

set_property(DIRECTORY PROPERTY VS_STARTUP_PROJECT MinecraftClient)
