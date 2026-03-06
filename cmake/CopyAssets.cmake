include("${CMAKE_CURRENT_SOURCE_DIR}/cmake/AssetSources.cmake")

set(_project_dir "${CMAKE_CURRENT_SOURCE_DIR}/Minecraft.Client")

function(copy_tree_if_exists src_rel dst_rel)
  set(_src "${_project_dir}/${src_rel}")
  set(_dst "${CMAKE_CURRENT_BINARY_DIR}/${dst_rel}")

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
          configure_file(${_full_src} ${_full_dst} COPYONLY)
        endif()
      endif()
    endforeach()
  endif()
endfunction()

function(make_media_archive arc_name)
  set(_dst "${CMAKE_CURRENT_BINARY_DIR}/Common/Media/${arc_name}")
  set(_src_dir "${CMAKE_CURRENT_SOURCE_DIR}/Minecraft.Client/Common/Media")
  set(_src)
  foreach(file IN LISTS ARGN)
    list(APPEND _src "${_src_dir}/${file}")
  endforeach()

  get_filename_component(_dst_dir "${_dst}" DIRECTORY)
  file(MAKE_DIRECTORY "${_dst_dir}")

  add_custom_command(
    OUTPUT "${_dst}.zip"
    DEPENDS ${_src}
    WORKING_DIRECTORY ${_src_dir}
    COMMAND ${CMAKE_COMMAND} -E tar "cf" "${_dst}.zip" --format=zip -- ${ARGN}
    COMMENT "Creating archive ${arc_name}.zip"
    VERBATIM
  )

  add_custom_command(
    OUTPUT "${_dst}.arc"
    DEPENDS "${_dst}.zip"
    COMMAND python "${CMAKE_CURRENT_SOURCE_DIR}/arcconv.py" "zip2arc" "${_dst}.zip" "${_dst}.arc"
    COMMENT "Creating archive ${arc_name}.arc"
    VERBATIM
  )

  add_custom_target("arc_${arc_name}" DEPENDS "${_dst}.arc")
  add_dependencies(MinecraftClient "arc_${arc_name}")
endfunction()

copy_tree_if_exists("Durango/Sound" "Windows64/Sound")
copy_tree_if_exists("music" "music")
copy_tree_if_exists("Windows64/GameHDD" "Windows64/GameHDD")
copy_tree_if_exists("Common/res" "Common/res")
copy_tree_if_exists("Common/Trial" "Common/Trial")
copy_tree_if_exists("Common/Tutorial" "Common/Tutorial")
copy_tree_if_exists("Windows64Media" "Windows64Media")

make_media_archive("MediaWindows64" ${ASSETS_WINDOWS64})
make_media_archive("MediaDurango" ${ASSETS_DURANGO})
make_media_archive("MediaOrbis" ${ASSETS_ORBIS})
make_media_archive("MediaPSVita" ${ASSETS_PSVITA})
make_media_archive("MediaPS3" ${ASSETS_PS3})

# Some runtime code asserts if this directory tree is missing.
file(MAKE_DIRECTORY "Windows64/GameHDD")

# Keep legacy runtime redistributables in a familiar location.
copy_tree_if_exists("Windows64/Miles/lib/redist64" "redist64")
copy_tree_if_exists("Windows64/Iggy/lib/redist64" "redist64")

set(CONFIGURATION $<$<CONFIG:Release>:Release>)

# Runtime DLLs required at launch.
configure_file("Minecraft.Client/Windows64/Iggy/lib/redist64/iggy_w64.dll" "iggy_w64.dll" COPYONLY)
configure_file("Minecraft.Client/Windows64/Miles/lib/redist64/mss64.dll" "mss64.dll" COPYONLY)

