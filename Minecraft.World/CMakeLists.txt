include("${CMAKE_CURRENT_LIST_DIR}/cmake/sources/Common.cmake")
include("${CMAKE_CURRENT_LIST_DIR}/cmake/sources/Durango.cmake")

include("${CMAKE_SOURCE_DIR}/cmake/CommonSources.cmake")

# Combine all source files into a single variable for the target
# We cant use CMAKE_CONFIGURE_PRESET here as VS doesn't set it, so just rely on the folder
set(MINECRAFT_WORLD_SOURCES
  ${MINECRAFT_WORLD_COMMON}
  $<$<STREQUAL:${PLATFORM_NAME},Durango>:${MINECRAFT_WORLD_DURANGO}>
  ${SOURCES_COMMON}
)

add_library(Minecraft.World STATIC ${MINECRAFT_WORLD_SOURCES})

target_include_directories(Minecraft.World
  PRIVATE
  "${CMAKE_BINARY_DIR}/generated/"
  "${CMAKE_CURRENT_SOURCE_DIR}"
  "${CMAKE_SOURCE_DIR}/include/"
  PUBLIC
  "${CMAKE_CURRENT_SOURCE_DIR}/x64headers"
)
target_compile_definitions(Minecraft.World PRIVATE
  ${MINECRAFT_SHARED_DEFINES}
  _LIB
)
target_precompile_headers(Minecraft.World PRIVATE "$<$<COMPILE_LANGUAGE:CXX>:stdafx.h>")

configure_compiler_target(Minecraft.World)
