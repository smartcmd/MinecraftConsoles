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
    file(GLOB_RECURSE _files RELATIVE "${_src}" "${_src}/*")

    foreach(_file IN LISTS _files) # if not a source file 
      if(NOT _file MATCHES "\\.(cpp|c|h|hpp|xml|lang)$")
        set(_full_src "${_src}/${_file}")
        set(_full_dst "${_dst}/${_file}")

        if(IS_DIRECTORY "${_full_src}")
          file(MAKE_DIRECTORY "${_full_dst}")
        else()
          get_filename_component(_dst_dir "${_full_dst}" DIRECTORY)
          file(MAKE_DIRECTORY "${_dst_dir}")
          execute_process(
            COMMAND "${CMAKE_COMMAND}" -E copy_if_different
            "${_full_src}" "${_full_dst}"
          )
        endif()
      endif()
    endforeach()
  endif()
endfunction()

function(ensure_dir rel_path)
  file(MAKE_DIRECTORY "${OUTPUT_DIR}/${rel_path}")
endfunction()

function(copy_file_if_exists src_rel dst_rel)
  set(_src "${PROJECT_SOURCE_DIR}/${src_rel}")
  set(_dst "${OUTPUT_DIR}/${dst_rel}")

  get_filename_component(_dst_dir "${_dst}" DIRECTORY)
  file(MAKE_DIRECTORY "${_dst_dir}")

  if(EXISTS "${_src}")
    execute_process(
      COMMAND "${CMAKE_COMMAND}" -E copy_if_different
      "${_src}" "${_dst}"
    )
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

function(remove_directory_if_exists rel_path)
  set(_dir "${OUTPUT_DIR}/${rel_path}")
  if(EXISTS "${_dir}")
    file(REMOVE_RECURSE "${_dir}")
  endif()
endfunction()

copy_tree_if_exists("Durango/Sound" "Windows64/Sound")
copy_tree_if_exists("music" "music")
copy_tree_if_exists("Windows64/GameHDD" "Windows64/GameHDD")
copy_file_if_exists("Minecraft.Client/Common/Media/MediaWindows64.arc" "Common/Media/MediaWindows64.arc")
copy_tree_if_exists("Common/res" "Common/res")
copy_tree_if_exists("Common/Trial" "Common/Trial")
copy_tree_if_exists("Common/Tutorial" "Common/Tutorial")
copy_tree_if_exists("DurangoMedia" "Windows64Media")
copy_tree_if_exists("Windows64Media" "Windows64Media")

remove_directory_if_exists("Windows64Media/Layout")

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
