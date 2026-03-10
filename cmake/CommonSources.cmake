set(_INCLUDE_LCE_FILESYSTEM
  "${CMAKE_SOURCE_DIR}/include/lce_filesystem/lce_filesystem.cpp"
  "${CMAKE_SOURCE_DIR}/include/lce_filesystem/lce_filesystem.h"
)
source_group("include/lce_filesystem" FILES ${_INCLUDE_LCE_FILESYSTEM})

set(SOURCES_COMMON
	${_INCLUDE_LCE_FILESYSTEM}
)