# Generates BuildVer.h with git version info.
#
# Required:
#   OUTPUT_FILE - path to write BuildVer.h

if(NOT OUTPUT_FILE)
  message(FATAL_ERROR "OUTPUT_FILE must be set.")
endif()

set(BUILD_NUMBER 560) # Note: Build/network has to stay static for now, as without it builds wont be able to play together. We can change it later when we have a better versioning scheme in place.
set(SUFFIX "")

# Get short SHA
execute_process(
  COMMAND git rev-parse --short HEAD
  OUTPUT_VARIABLE GIT_SHA
  OUTPUT_STRIP_TRAILING_WHITESPACE
  RESULT_VARIABLE rc
)
if(NOT rc EQUAL 0)
  set(GIT_SHA "unknown")
endif()

# Get branch name
execute_process(
  COMMAND git symbolic-ref --short HEAD
  OUTPUT_VARIABLE GIT_REF
  OUTPUT_STRIP_TRAILING_WHITESPACE
  RESULT_VARIABLE rc
)
if(NOT rc EQUAL 0)
  set(GIT_REF "unknown")
endif()

# If we have uncommitted changes, add a suffix to the version string
execute_process(
  COMMAND git status --porcelain
  OUTPUT_VARIABLE GIT_STATUS
  OUTPUT_STRIP_TRAILING_WHITESPACE
)
if(GIT_STATUS)
  set(SUFFIX "-dev")
endif()

file(WRITE "${OUTPUT_FILE}"
  "#pragma once\n"
  "\n"
  "#define VER_PRODUCTBUILD ${BUILD_NUMBER}\n"
  "#define VER_PRODUCTVERSION_STR_W L\"${GIT_SHA}${SUFFIX} (${GIT_REF})\"\n"
  "#define VER_FILEVERSION_STR_W VER_PRODUCTVERSION_STR_W\n"
  "#define VER_NETWORK VER_PRODUCTBUILD\n"
)