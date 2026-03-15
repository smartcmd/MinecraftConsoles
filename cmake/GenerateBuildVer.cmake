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
  OUTPUT_VARIABLE GIT_BRANCH
  OUTPUT_STRIP_TRAILING_WHITESPACE
  RESULT_VARIABLE rc
)
if(NOT rc EQUAL 0)
  set(GIT_BRANCH "unknown")
endif()

# Get GitHub repository from environment variable (if available) or git remote
if(DEFINED ENV{GITHUB_REPOSITORY})
  set(GIT_REF "$ENV{GITHUB_REPOSITORY}/${GIT_BRANCH}")
else()
  execute_process(
    COMMAND git remote get-url origin
    OUTPUT_VARIABLE GIT_REMOTE_URL
    OUTPUT_STRIP_TRAILING_WHITESPACE
    RESULT_VARIABLE rc
  )
  # Handle github urls only
  if(rc EQUAL 0 AND GIT_REMOTE_URL MATCHES "github\\.com[:/]([^/:]+/[^/.]+)(\\.git)?")
    set(GIT_REF "${CMAKE_MATCH_1}/${GIT_BRANCH}")
  else()
    set(GIT_REF "UNKNOWN/${GIT_BRANCH}")
  endif()
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

# Write to a temp file first, only overwrite if content changed.
# This prevents unnecessary rebuilds when the version info hasn't changed.
set(_content
  "#pragma once\n"
  "\n"
  "#define VER_PRODUCTBUILD ${BUILD_NUMBER}\n"
  "#define VER_PRODUCTVERSION_STR_W L\"${GIT_SHA}${SUFFIX}\"\n"
  "#define VER_FILEVERSION_STR_W VER_PRODUCTVERSION_STR_W\n"
  "#define VER_BRANCHVERSION_STR_W L\"${GIT_REF}\"\n"
  "#define VER_NETWORK VER_PRODUCTBUILD\n"
)

set(_tmp "${OUTPUT_FILE}.tmp")
file(WRITE "${_tmp}" ${_content})

if(EXISTS "${OUTPUT_FILE}")
  execute_process(
    COMMAND ${CMAKE_COMMAND} -E compare_files "${OUTPUT_FILE}" "${_tmp}"
    RESULT_VARIABLE _changed
  )
else()
  set(_changed 1)
endif()

if(_changed)
  file(RENAME "${_tmp}" "${OUTPUT_FILE}")
else()
  file(REMOVE "${_tmp}")
endif()