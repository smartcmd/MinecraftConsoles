if(NOT DEFINED PROJECT_SOURCE_DIR OR NOT DEFINED OUTPUT_DIR OR NOT DEFINED CONFIGURATION)
  message(FATAL_ERROR "CopyAssets.cmake requires PROJECT_SOURCE_DIR, OUTPUT_DIR, and CONFIGURATION.")
endif()

# Some generators may pass quoted values (e.g. "Debug"); normalize that.
string(REPLACE "\"" "" PROJECT_SOURCE_DIR "${PROJECT_SOURCE_DIR}")
string(REPLACE "\"" "" OUTPUT_DIR "${OUTPUT_DIR}")
string(REPLACE "\"" "" CONFIGURATION "${CONFIGURATION}")

set(_project_dir "${PROJECT_SOURCE_DIR}/Minecraft.Client")

function(copy_tree_if_exists src_rel dst_rel)
  set(_src "${_project_dir}/${src_rel}")
  set(_dst "${OUTPUT_DIR}/${dst_rel}")
  if(EXISTS "${_src}")
    file(MAKE_DIRECTORY "${_dst}")
    execute_process(COMMAND "${CMAKE_COMMAND}" -E copy_directory "${_src}" "${_dst}")
  endif()
endfunction()

function(ensure_dir rel_path)
  file(MAKE_DIRECTORY "${OUTPUT_DIR}/${rel_path}")
endfunction()

function(copy_file_if_exists src_rel dst_rel)
  set(_src "${PROJECT_SOURCE_DIR}/${src_rel}")
  set(_dst "${OUTPUT_DIR}/${dst_rel}")
  if(EXISTS "${_src}")
    get_filename_component(_dst_dir "${_dst}" DIRECTORY)
    file(MAKE_DIRECTORY "${_dst_dir}")
    execute_process(COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${_src}" "${_dst}")
  endif()
endfunction()

function(copy_first_existing dst_rel)
  set(_copied FALSE)
  foreach(_candidate IN LISTS ARGN)
    if(EXISTS "${PROJECT_SOURCE_DIR}/${_candidate}")
      copy_file_if_exists("${_candidate}" "${dst_rel}")
      set(_copied TRUE)
      break()
    endif()
  endforeach()
  if(NOT _copied)
    message(WARNING "Runtime file not found for ${dst_rel}. Checked: ${ARGN}")
  endif()
endfunction()

if(CONFIGURATION STREQUAL "Debug")
  copy_tree_if_exists("Durango/Sound" "Durango/Sound")
  copy_tree_if_exists("music" "music")
  copy_tree_if_exists("Windows64/GameHDD" "Windows64/GameHDD")
  copy_tree_if_exists("Common/Media" "Common/Media")
  copy_tree_if_exists("Common/res" "Common/res")
  copy_tree_if_exists("Common/Trial" "Common/Trial")
  copy_tree_if_exists("Common/Tutorial" "Common/Tutorial")
else()
  copy_tree_if_exists("music" "music")
  copy_tree_if_exists("Windows64/GameHDD" "Windows64/GameHDD")
  copy_tree_if_exists("Common/Media" "Common/Media")
  copy_tree_if_exists("Common/res" "Common/res")
  copy_tree_if_exists("Common/Trial" "Common/Trial")
  copy_tree_if_exists("Common/Tutorial" "Common/Tutorial")
  copy_tree_if_exists("DurangoMedia" "Windows64Media")
  copy_tree_if_exists("Windows64Media" "Windows64Media")
endif()

# Some runtime code asserts if this directory tree is missing.
ensure_dir("Windows64/GameHDD")

# Keep legacy runtime redistributables in a familiar location.
copy_tree_if_exists("Windows64/Miles/lib/redist64" "redist64")
copy_tree_if_exists("Windows64/Iggy/lib/redist64" "redist64")

# Runtime DLLs required at launch.
copy_first_existing("iggy_w64.dll"
  "Minecraft.Client/Windows64/Iggy/lib/redist64/iggy_w64.dll"
  "x64/${CONFIGURATION}/iggy_w64.dll"
)
copy_first_existing("mss64.dll"
  "Minecraft.Client/Windows64/Miles/lib/redist64/mss64.dll"
  "x64/${CONFIGURATION}/mss64.dll"
)
