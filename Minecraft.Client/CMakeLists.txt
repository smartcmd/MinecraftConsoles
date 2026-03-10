include("${CMAKE_CURRENT_LIST_DIR}/cmake/Common.cmake")
include("${CMAKE_CURRENT_LIST_DIR}/cmake/Durango.cmake")
include("${CMAKE_CURRENT_LIST_DIR}/cmake/ORBIS.cmake")
include("${CMAKE_CURRENT_LIST_DIR}/cmake/PS3.cmake")
include("${CMAKE_CURRENT_LIST_DIR}/cmake/PSVita.cmake")
include("${CMAKE_CURRENT_LIST_DIR}/cmake/Windows.cmake")
include("${CMAKE_CURRENT_LIST_DIR}/cmake/Xbox360.cmake")

# Combine all source files into a single variable for the target
# TODO Add conditions to include sources for the current platform
set(MINECRAFT_CLIENT_SOURCES
  ${MINECRAFT_CLIENT_COMMON}
  # ${MINECRAFT_CLIENT_DURANGO}
  # ${MINECRAFT_CLIENT_ORBIS}
  # ${MINECRAFT_CLIENT_PS3}
  # ${MINECRAFT_CLIENT_PSVITA}
  ${MINECRAFT_CLIENT_WINDOWS}
  # ${MINECRAFT_CLIENT_XBOX360}
  PARENT_SCOPE
)
