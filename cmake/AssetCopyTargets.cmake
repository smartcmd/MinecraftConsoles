# Creates the CopyAssets build target

set(COPY_SCRIPT "${CMAKE_CURRENT_SOURCE_DIR}/cmake/CopyAssets.cmake")

# Source;Dest pairs relative to Minecraft.Client/ and the build output.
# Order matters — later entries overwrite earlier ones on conflict.
set(ASSET_FOLDER_PAIRS
  "music"              "music"
  "Common/Media"       "Common/Media"
  "Common/res"         "Common/res"
  "Common/Trial"       "Common/Trial"
  "Common/Tutorial"    "Common/Tutorial"
#   "Windows64/GameHDD"  "Windows64/GameHDD"
  "DurangoMedia"       "Windows64Media" # Use Durango as a base for the Windows64 media
  "Windows64Media"     "Windows64Media"
)

# Global exclusions applied to every folder copy
set(ASSET_EXCLUDE_FILES
  "*.cpp" "*.h"
  "*.xml" "*.lang" 
  "*.bat" "*.cmd"
  "*.msscmp" "*.binka"
)

# Join the exclusion patterns into a single string for passing to the copy script
list(JOIN ASSET_EXCLUDE_FILES "|" ASSET_EXCLUDE_FILES_STR)

function(setup_asset_copy_targets)
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
        "-DCOPY_SOURCE=${CMAKE_CURRENT_SOURCE_DIR}/Minecraft.Client/${src}"
        "-DCOPY_DEST=${CMAKE_CURRENT_BINARY_DIR}/$<CONFIG>/${dest}"
        "-DEXCLUDE_FILES=${ASSET_EXCLUDE_FILES_STR}"
        -P "${COPY_SCRIPT}"
    )
  endforeach()

  add_custom_target(CopyAssets ALL
    ${copy_commands}
    COMMENT "Copying assets..."
    VERBATIM
  )

  add_dependencies(MinecraftClient CopyAssets)

  set_property(TARGET CopyAssets PROPERTY FOLDER "Build")
endfunction()