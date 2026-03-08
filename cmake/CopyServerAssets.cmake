if(NOT DEFINED PROJECT_SOURCE_DIR OR NOT DEFINED OUTPUT_DIR OR NOT DEFINED CONFIGURATION)
  message(FATAL_ERROR "CopyServerAssets.cmake requires PROJECT_SOURCE_DIR, OUTPUT_DIR, and CONFIGURATION.")
endif()

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

    foreach(_file IN LISTS _files)
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
  foreach(_candidate IN LISTS ARGN)
    if(EXISTS "${PROJECT_SOURCE_DIR}/${_candidate}")
      copy_file_if_exists("${_candidate}" "${dst_rel}")
      return()
    endif()
  endforeach()
endfunction()

# Dedicated server runtime assets (minimal set validated in startup tests).
copy_file_if_exists("Minecraft.Client/Common/Media/MediaWindows64.arc" "Common/Media/MediaWindows64.arc")
copy_tree_if_exists("Common/res" "Common/res")
copy_tree_if_exists("Windows64/GameHDD" "Windows64/GameHDD")

copy_first_existing("iggy_w64.dll"
  "Minecraft.Client/Windows64/Iggy/lib/redist64/iggy_w64.dll"
  "x64/${CONFIGURATION}/iggy_w64.dll"
)

