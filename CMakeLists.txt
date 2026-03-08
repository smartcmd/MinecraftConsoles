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

if(CMAKE_HOST_WIN32)
  message(STATUS "Starting redist copy...")
  execute_process(
    COMMAND robocopy.exe
            "${CMAKE_CURRENT_SOURCE_DIR}/x64/Release"
            "${CMAKE_CURRENT_BINARY_DIR}"
            /S /MT /R:0 /W:0 /NP
            /XF "*.msscmp"
  )
  message(STATUS "Starting asset copy...")
  execute_process(
    COMMAND robocopy.exe
            "${CMAKE_CURRENT_SOURCE_DIR}/Minecraft.Client"
            "${CMAKE_CURRENT_BINARY_DIR}"
            /S /MT /R:0 /W:0 /NP
            /XF
                "*.c" "*.cpp" "*.h" "*.hpp" "*.xui" "*.inl" "*.asm"  # Source code.
                "*.vcxproj" "*.vcxproj.*" "*.sln" "*.cd"             # Project files.
                "*.xml" "*.lang" "*.resx" "*.loc"                    # Language files.
                "*.docx" "*.xls"                                     # Office documents.
                "*.bat" "*.cmd" "*.ps1" "*.py"                       # Scripts.
                "*.lib" "*.lib.*"                                    # Static lib.
                "*.swf" "*.msscmp" "*.wav"                           # Uncompiled/unused res.
                "*Test*"                                             # Test files. (who could have guessed)
                "MediaPS*" "MediaOrbis.arc" "MediaDurango.arc"       # Console media.
                "ReadMe.txt" "Network Implementation Notes.txt"      # Extra.
            /XD
                "Durango*" "Orbis*" "PS*" "Xbox"                     # Console data.
                "sce_sys" "TROPDIR"                                  # Playstation data.
                "Graphics" "DummyTexturePack" "GameConfig"           # Extra.
  )
  file(MAKE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/Windows64/GameHDD")
elseif(CMAKE_HOST_UNIX)
  message(STATUS "Starting redist copy...")
  execute_process(
    COMMAND cp -av "${CMAKE_CURRENT_SOURCE_DIR}/x64/Release/" "${CMAKE_CURRENT_BINARY_DIR}/"
  )
  message(STATUS "Starting asset copy...")
  execute_process(
    COMMAND tar
          "--exclude=*.c" "--exclude=*.cpp" "--exclude=*.h" "--exclude=*.hpp" # Source code.
          "--exclude=*.xui" "--exclude=*.inl" "--exclude=*.asm"
          "--exclude=*.vcxproj" "--exclude=*.vcxproj.*"                       # Project files.
          "--exclude=*.sln" "--exclude=*.cd"
          "--exclude=*.xml" "--exclude=*.lang"                                # Language files.
          "--exclude=*.resx" "--exclude=*.loc"
          "--exclude=*.docx" "--exclude=*.xls"                                # Office documents.
          "--exclude=*.bat" "--exclude=*.cmd"                                 # Scripts.
          "--exclude=*.ps1" "--exclude=*.py"
          "--exclude=*.lib" "--exclude=*.lib.*"                               # Static lib.
          "--exclude=*.swf" "--exclude=*.msscmp" "--exclude=*.wav"            # Uncompiled/unused res.
          "--exclude=*Test*"                                                  # Test files. (who could have guessed)
          "--exclude=MediaPS*" "--exclude=MediaOrbis.arc"                     # Console media.
          "--exclude=MediaDurango.arc"
          "--exclude=ReadMe.txt" "--exclude=Network Implementation Notes.txt" # Extra.

          "--exclude=Durango*" "--exclude=Orbis*"            # Console data.
          "--exclude=PS*" "--exclude=Xbox"
          "--exclude=sce_sys" "--exclude=TROPDIR"            # Playstation data.
          "--exclude=Graphics" "--exclude=DummyTexturePack"  # Extra.
          "--exclude=GameConfig"
            -cf - -C "${CMAKE_CURRENT_SOURCE_DIR}/Minecraft.Client/" .
    COMMAND tar -xvf - -C "${CMAKE_CURRENT_BINARY_DIR}/"
  )
  file(MAKE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/Windows64/GameHDD")
else()
  message(FATAL_ERROR "Redist and asset copying is only supported on Windows (Robocopy) and Unix systems (cp, tar).")
endif()

set_property(DIRECTORY PROPERTY VS_STARTUP_PROJECT MinecraftClient)
