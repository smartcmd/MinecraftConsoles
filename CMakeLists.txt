cmake_minimum_required(VERSION 3.24)

project(MinecraftConsoles)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(BACKEND "Windows64" CACHE STRING "Backend to compile. Windows64 or Cross64")
set_property(CACHE BACKEND PROPERTY STRINGS "Windows64" "Cross64")

# assuming what the user wants, probably not best practice.
if(NOT DEFINED CACHE{BACKEND})
  if(WIN32)
    set(BACKEND "Windows64")
  else()
    set(BACKEND "Cross64")
  endif()
endif()

message(STATUS "MinecraftConsoles: Generating ${BACKEND}.")

if(BACKEND STREQUAL "Windows64")
  enable_language(C CXX ASM_MASM)
else()
  enable_language(C CXX)
endif()

if(NOT CMAKE_SIZEOF_VOID_P EQUAL 8)
  message(FATAL_ERROR "Use a 64-bit generator/toolchain (x64).")
endif()

if(MSVC)
  set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
endif()

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

# minecraftworld is shared with every backend. 
include("${CMAKE_CURRENT_SOURCE_DIR}/cmake/WorldSources.cmake")
list(TRANSFORM MINECRAFT_WORLD_SOURCES PREPEND "${CMAKE_CURRENT_SOURCE_DIR}/Minecraft.World/")

add_library(MinecraftWorld STATIC ${MINECRAFT_WORLD_SOURCES})
target_include_directories(MinecraftWorld PUBLIC
  "${CMAKE_CURRENT_SOURCE_DIR}"
  "${CMAKE_CURRENT_SOURCE_DIR}/Minecraft.World"
  "${CMAKE_CURRENT_SOURCE_DIR}/Minecraft.World/x64headers"
)
target_compile_definitions(MinecraftWorld PRIVATE
  $<$<CONFIG:Debug>:_LARGE_WORLDS;_DEBUG_MENUS_ENABLED;_DEBUG;_LIB;_CRT_NON_CONFORMING_SWPRINTFS;_CRT_SECURE_NO_WARNINGS;>
  $<$<NOT:$<CONFIG:Debug>>:_LARGE_WORLDS;_DEBUG_MENUS_ENABLED;_LIB;_CRT_NON_CONFORMING_SWPRINTFS;_CRT_SECURE_NO_WARNINGS;>
  $<$<STREQUAL:${BACKEND},Windows64>:_WINDOWS64>
  $<$<STREQUAL:${BACKEND},Cross64>:_CROSS64>
)
if(MSVC)
  configure_msvc_target(MinecraftWorld)
endif()

include("${CMAKE_CURRENT_SOURCE_DIR}/cmake/ClientSources.cmake")

# each backend is assumed to change these in some way.
set(BACKEND_SOURCES
  "glWrapper.cpp"
  "stubs.cpp"
  "stdafx.cpp"
  "iob_shim.asm"
  "Windows64/Iggy/gdraw/gdraw_d3d11.cpp"
  "Windows64/KeyboardMouseInput.cpp"
  "Windows64/Leaderboards/WindowsLeaderboardManager.cpp"
  "Windows64/Windows64_App.cpp"
  "Windows64/Windows64_Minecraft.cpp"
  "Windows64/Windows64_UIController.cpp"
  "Windows64/Network/WinsockNetLayer.cpp"
)

# remove backend specific stuff
set(CLIENT_SHARED_SOURCES ${MINECRAFT_CLIENT_SOURCES})
list(REMOVE_ITEM CLIENT_SHARED_SOURCES ${BACKEND_SOURCES})
list(TRANSFORM CLIENT_SHARED_SOURCES PREPEND "${CMAKE_CURRENT_SOURCE_DIR}/Minecraft.Client/")

if(BACKEND STREQUAL "Windows64")
  if(NOT WIN32)
    message(FATAL_ERROR "The Windows64 backend can only be built on Windows.")
  endif()

  list(TRANSFORM MINECRAFT_CLIENT_SOURCES PREPEND "${CMAKE_CURRENT_SOURCE_DIR}/Minecraft.Client/")

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
    target_compile_options(MinecraftClient PRIVATE
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

  add_custom_command(TARGET MinecraftClient POST_BUILD
    COMMAND "${CMAKE_COMMAND}"
      -DPROJECT_SOURCE_DIR="${CMAKE_CURRENT_SOURCE_DIR}"
      -DOUTPUT_DIR="$<TARGET_FILE_DIR:MinecraftClient>"
      -DCONFIGURATION=$<CONFIG>
      -P "${CMAKE_CURRENT_SOURCE_DIR}/cmake/CopyAssets.cmake"
    VERBATIM
  )

  set_property(DIRECTORY PROPERTY VS_STARTUP_PROJECT MinecraftClient)
elseif(BACKEND STREQUAL "Cross64")
  include(FetchContent)

  FetchContent_Declare(glm GIT_REPOSITORY https://github.com/g-truc/glm.git GIT_TAG 1.0.3)
  FetchContent_MakeAvailable(glm)

  FetchContent_Declare(SDL3 GIT_REPOSITORY https://github.com/libsdl-org/SDL GIT_TAG release-3.4.2)
  FetchContent_MakeAvailable(SDL3)

  set(CROSS64_DIR "${CMAKE_CURRENT_SOURCE_DIR}/Minecraft.Client/Cross64")
  include("${CMAKE_CURRENT_SOURCE_DIR}/cmake/Cross64Sources.cmake")
  list(TRANSFORM CROSS64_SOURCES PREPEND "${CROSS64_DIR}")

  add_executable(MinecraftClientCross64 
    ${CLIENT_SHARED_SOURCES}
    ${CROSS64_SOURCES}
  )

  target_include_directories(MinecraftWorld PRIVATE "${CROSS64_DIR}/Cross64Extras")

  target_include_directories(MinecraftClientCross64 PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}"
    "${CROSS64_DIR}/Cross64Extras/"
    
    "${CROSS64_DIR}"
    "${CROSS64_DIR}/4JLibs/inc"
    "${CROSS64_DIR}/Glad/include"

    "${CMAKE_CURRENT_SOURCE_DIR}/Minecraft.Client"
    "${CMAKE_CURRENT_SOURCE_DIR}/Minecraft.World/x64headers"
  )

  target_compile_definitions(MinecraftClientCross64 PRIVATE
    $<$<CONFIG:Debug>:_LARGE_WORLDS;_DEBUG_MENUS_ENABLED;_DEBUG;_CRT_NON_CONFORMING_SWPRINTFS;_CRT_SECURE_NO_WARNINGS;_CROSS64>
    $<$<NOT:$<CONFIG:Debug>>:_LARGE_WORLDS;_DEBUG_MENUS_ENABLED;_CRT_NON_CONFORMING_SWPRINTFS;_CRT_SECURE_NO_WARNINGS;_CROSS64>
  )

  if(MSVC)
      target_compile_options(MinecraftClientCross64 PRIVATE
        $<$<COMPILE_LANGUAGE:C,CXX>:/W3 /MP>
        $<$<COMPILE_LANGUAGE:CXX>:/EHsc>
      )
  else()
      target_compile_options(MinecraftClientCross64 PRIVATE
        -Wall -Wextra
      )
  endif()

  set_target_properties(MinecraftClientCross64 PROPERTIES
    VS_DEBUGGER_WORKING_DIRECTORY "$<TARGET_FILE_DIR:MinecraftClientCross64>"
  )

  target_link_libraries(MinecraftClientCross64 PRIVATE
    MinecraftWorld
    glm::glm
    SDL3::SDL3
    OpenGL:GL
  )

  find_package(OpenGL REQUIRED)

  if(APPLE)
    target_link_libraries(MinecraftClientCross64 PRIVATE
      "-framework Cocoa" 
      "-framework IOKit" 
      "-framework CoreVideo"
    )
  elseif(UNIX)
    target_link_libraries(MinecraftClientCross64 PRIVATE 
      pthread dl
    )
  elseif(WIN32)
    target_link_libraries(MinecraftClientCross64 PRIVATE 
      wsock32
    )
  endif()

  add_custom_command(TARGET MinecraftClientCross64 POST_BUILD
  COMMAND "${CMAKE_COMMAND}"
    -DPROJECT_SOURCE_DIR="${CMAKE_CURRENT_SOURCE_DIR}"
    -DOUTPUT_DIR="$<TARGET_FILE_DIR:MinecraftClientCross64>"
    -DCONFIGURATION=$<CONFIG>
    -P "${CMAKE_CURRENT_SOURCE_DIR}/cmake/CopyAssets.cmake"
    VERBATIM
  )

  set_property(DIRECTORY PROPERTY VS_STARTUP_PROJECT MinecraftClientCross64)
else()
  message(FATAL_ERROR "Invalid Backend.")
endif()