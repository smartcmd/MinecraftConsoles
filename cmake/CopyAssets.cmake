function(setup_asset_folder_copy TARGET_NAME ASSET_FOLDER_PAIRS)
  set(COPY_FOLDER_SCRIPT "${CMAKE_SOURCE_DIR}/cmake/CopyFolderScript.cmake")

  # Global exclusions applied to every folder copy
  set(ASSET_EXCLUDE_FILES
    "*.cpp" "*.h"
    "*.xml" "*.lang" 
    "*.bat" "*.cmd"
    "*.msscmp" "*.binka" # Old audio formats
    "*.swf" # These are built into the .arc
    "*.resx" "*.loc"
    "*.wav" # Unsupported audio format
    "*.xui"
  )

  # Global folder exclusions applied to every folder copy
  set(ASSET_EXCLUDE_FOLDERS
    "Graphics"
  )

  # Exclude platform-specific arc media files
  set(PLATFORM_MEDIA_FILES
    "MediaWindows64.arc"
    "MediaDurango.arc"
    "MediaOrbis.arc"
    "MediaPS3.arc"
    "MediaPSVita.arc"
    "MediaXbox.arc" # Seems to be missing?
  )

  # Exclude all platform media files except the one for the current platform
  foreach(media_file IN LISTS PLATFORM_MEDIA_FILES)
    if(NOT media_file MATCHES "Media${PLATFORM_NAME}\\.arc")
      list(APPEND ASSET_EXCLUDE_FILES "${media_file}")
    endif()
  endforeach()

  # Join the exclusion patterns into a single string for passing to the copy script
  list(JOIN ASSET_EXCLUDE_FILES "|" ASSET_EXCLUDE_FILES_STR)
  list(JOIN ASSET_EXCLUDE_FOLDERS "|" ASSET_EXCLUDE_FOLDERS_STR)

  set(copy_commands "")
  list(LENGTH ASSET_FOLDER_PAIRS pair_count)
  math(EXPR last "${pair_count} - 1")

  # Loop through the source;dest pairs and create a copy command for each
  foreach(i RANGE 0 ${last} 2)
    math(EXPR j "${i} + 1")
    list(GET ASSET_FOLDER_PAIRS ${i} src)
    list(GET ASSET_FOLDER_PAIRS ${j} dest)

    list(APPEND copy_commands
      COMMAND ${CMAKE_COMMAND}
        "-DCOPY_SOURCE=${src}"
        "-DCOPY_DEST=$<TARGET_FILE_DIR:${TARGET_NAME}>/${dest}"
        "-DEXCLUDE_FILES=${ASSET_EXCLUDE_FILES_STR}"
        "-DEXCLUDE_FOLDERS=${ASSET_EXCLUDE_FOLDERS_STR}"
        -P "${COPY_FOLDER_SCRIPT}"
    )
  endforeach()

  add_custom_target(AssetFolderCopy_${TARGET_NAME} ALL
    ${copy_commands}
    COMMENT "Copying assets (folders) for ${TARGET_NAME}..."
    VERBATIM
  )

  add_dependencies(${TARGET_NAME} AssetFolderCopy_${TARGET_NAME})
  set_property(TARGET AssetFolderCopy_${TARGET_NAME} PROPERTY FOLDER "Build")
endfunction()

function(setup_asset_file_copy TARGET_NAME ASSET_FILE_PAIRS)
  set(COPY_FILE_SCRIPT "${CMAKE_SOURCE_DIR}/cmake/CopyFileScript.cmake")

  set(copy_commands "")
  list(LENGTH ASSET_FILE_PAIRS pair_count)
  math(EXPR last "${pair_count} - 1")

  # Loop through the source;dest pairs and create a copy command for each
  foreach(i RANGE 0 ${last} 2)
    math(EXPR j "${i} + 1")
    list(GET ASSET_FILE_PAIRS ${i} src)
    list(GET ASSET_FILE_PAIRS ${j} dest)

    list(APPEND copy_commands
      COMMAND ${CMAKE_COMMAND}
        "-DCOPY_SOURCE=${src}"
        "-DCOPY_DEST=$<TARGET_FILE_DIR:${TARGET_NAME}>/${dest}"
        -P "${COPY_FILE_SCRIPT}"
    )
  endforeach()

  add_custom_target(AssetFileCopy_${TARGET_NAME} ALL
    ${copy_commands}
    COMMENT "Copying assets (files) for ${TARGET_NAME}..."
    VERBATIM
  )

  add_dependencies(${TARGET_NAME} AssetFileCopy_${TARGET_NAME})
  set_property(TARGET AssetFileCopy_${TARGET_NAME} PROPERTY FOLDER "Build")
endfunction()
