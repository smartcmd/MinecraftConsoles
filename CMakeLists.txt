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
  "${CMAKE_CURRENT_SOURCE_DIR}/include/"
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

set(MINECRAFT_SERVER_SOURCES ${MINECRAFT_CLIENT_SOURCES})
list(APPEND MINECRAFT_SERVER_SOURCES
  "${CMAKE_CURRENT_SOURCE_DIR}/Minecraft.Server/Windows64/ServerMain.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/Minecraft.Server/Access/Access.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/Minecraft.Server/Access/BanManager.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/Minecraft.Server/Access/WhitelistManager.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/Minecraft.Server/Console/ServerCli.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/Minecraft.Server/Console/ServerCliInput.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/Minecraft.Server/Console/ServerCliParser.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/Minecraft.Server/Console/ServerCliEngine.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/Minecraft.Server/Console/ServerCliRegistry.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/Minecraft.Server/Console/commands/ban/CliCommandBan.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/Minecraft.Server/Console/commands/ban-ip/CliCommandBanIp.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/Minecraft.Server/Console/commands/ban-list/CliCommandBanList.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/Minecraft.Server/Console/commands/help/CliCommandHelp.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/Minecraft.Server/Console/commands/pardon/CliCommandPardon.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/Minecraft.Server/Console/commands/pardon-ip/CliCommandPardonIp.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/Minecraft.Server/Console/commands/stop/CliCommandStop.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/Minecraft.Server/Console/commands/list/CliCommandList.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/Minecraft.Server/Console/commands/tp/CliCommandTp.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/Minecraft.Server/Console/commands/whitelist/CliCommandWhitelist.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/Minecraft.Server/Console/commands/gamemode/CliCommandGamemode.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/Minecraft.Server/Console/commands/time/CliCommandTime.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/Minecraft.Server/Console/commands/weather/CliCommandWeather.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/Minecraft.Server/Console/commands/give/CliCommandGive.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/Minecraft.Server/Console/commands/enchant/CliCommandEnchant.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/Minecraft.Server/Console/commands/kill/CliCommandKill.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/Minecraft.Server/Console/commands/defaultgamemode/CliCommandDefaultGamemode.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/Minecraft.Server/Console/commands/experience/CliCommandExperience.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/Minecraft.Server/Common/FileUtils.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/Minecraft.Server/Common/StringUtils.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/Minecraft.Server/ServerLogger.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/Minecraft.Server/ServerLogManager.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/Minecraft.Server/ServerProperties.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/Minecraft.Server/vendor/linenoise/linenoise.c"
  "${CMAKE_CURRENT_SOURCE_DIR}/Minecraft.Server/WorldManager.cpp"
)

add_executable(MinecraftServer ${MINECRAFT_SERVER_SOURCES})
target_include_directories(MinecraftServer PRIVATE
  "${CMAKE_CURRENT_SOURCE_DIR}/Minecraft.Client"
  "${CMAKE_CURRENT_SOURCE_DIR}/Minecraft.Client/Windows64/Iggy/include"
  "${CMAKE_CURRENT_SOURCE_DIR}/Minecraft.Client/Xbox/Sentient/Include"
  "${CMAKE_CURRENT_SOURCE_DIR}/Minecraft.World/x64headers"
  "${CMAKE_CURRENT_SOURCE_DIR}/include/"
  "${CMAKE_CURRENT_SOURCE_DIR}/Minecraft.Server/Windows64"
)
target_compile_definitions(MinecraftServer PRIVATE
  $<$<CONFIG:Debug>:_LARGE_WORLDS;_DEBUG_MENUS_ENABLED;_DEBUG;_CRT_NON_CONFORMING_SWPRINTFS;_CRT_SECURE_NO_WARNINGS;_WINDOWS64;MINECRAFT_SERVER_BUILD>
  $<$<NOT:$<CONFIG:Debug>>:_LARGE_WORLDS;_DEBUG_MENUS_ENABLED;_CRT_NON_CONFORMING_SWPRINTFS;_CRT_SECURE_NO_WARNINGS;_WINDOWS64;MINECRAFT_SERVER_BUILD>
)
if(MSVC)
  configure_msvc_target(MinecraftServer)
  target_link_options(MinecraftServer PRIVATE
    $<$<CONFIG:Release>:/LTCG /INCREMENTAL:NO>
  )
endif()

set_target_properties(MinecraftServer PROPERTIES
  OUTPUT_NAME "Minecraft.Server"
  VS_DEBUGGER_WORKING_DIRECTORY "$<TARGET_FILE_DIR:MinecraftServer>"
  VS_DEBUGGER_COMMAND_ARGUMENTS "-port 25565 -bind 0.0.0.0 -name DedicatedServer"
)

target_link_libraries(MinecraftServer PRIVATE
  MinecraftWorld
  d3d11
  XInput9_1_0
  wsock32
  legacy_stdio_definitions
  "${CMAKE_CURRENT_SOURCE_DIR}/Minecraft.Client/Windows64/Iggy/lib/iggy_w64.lib"
  "${CMAKE_CURRENT_SOURCE_DIR}/Minecraft.Client/Windows64/Iggy/lib/iggyperfmon_w64.lib"
  "${CMAKE_CURRENT_SOURCE_DIR}/Minecraft.Client/Windows64/Iggy/lib/iggyexpruntime_w64.lib"
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

if(CMAKE_HOST_WIN32)
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
elseif(CMAKE_HOST_UNIX)
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

add_custom_command(TARGET MinecraftServer POST_BUILD
  COMMAND "${CMAKE_COMMAND}"
    -DPROJECT_SOURCE_DIR="${CMAKE_CURRENT_SOURCE_DIR}"
    -DOUTPUT_DIR="$<TARGET_FILE_DIR:MinecraftServer>"
    -DCONFIGURATION=$<CONFIG>
    -P "${CMAKE_CURRENT_SOURCE_DIR}/cmake/CopyServerAssets.cmake"
  VERBATIM
)

set_property(DIRECTORY PROPERTY VS_STARTUP_PROJECT MinecraftServer)
