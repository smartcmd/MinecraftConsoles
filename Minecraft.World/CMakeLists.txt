include("${CMAKE_CURRENT_LIST_DIR}/cmake/Common.cmake")
include("${CMAKE_CURRENT_LIST_DIR}/cmake/Durango.cmake")

include("${CMAKE_SOURCE_DIR}/cmake/CommonSources.cmake")

# Combine all source files into a single variable for the target
# TODO Add conditions to include sources for the current platform
set(MINECRAFT_WORLD_SOURCES
  ${MINECRAFT_WORLD_COMMON}
  # ${MINECRAFT_WORLD_DURANGO}
  ${SOURCES_COMMON}
  PARENT_SCOPE
)
