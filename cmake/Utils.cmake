# Make sure the GameHDD directory exists, only supported on Windows
function(add_gamehdd_target TARGET_NAME)
  add_custom_target(EnsureGameHDD_${TARGET_NAME} ALL
    COMMAND ${CMAKE_COMMAND}
      -E make_directory "$<TARGET_FILE_DIR:${TARGET_NAME}>/Windows64/GameHDD"
    COMMENT "Ensuring GameHDD directory exists..."
    VERBATIM
  )
  add_dependencies(${TARGET_NAME} EnsureGameHDD_${TARGET_NAME})
  set_property(TARGET EnsureGameHDD_${TARGET_NAME} PROPERTY FOLDER "Build")
endfunction()

# Copy any needed redist files to the output directory
function(add_copyredist_target TARGET_NAME)
  set(COPY_SCRIPT "${CMAKE_SOURCE_DIR}/cmake/CopyFolderScript.cmake")

  add_custom_target(CopyRedist_${TARGET_NAME} ALL
    COMMAND ${CMAKE_COMMAND}
      "-DCOPY_SOURCE=${CMAKE_SOURCE_DIR}/Minecraft.Client/${PLATFORM_NAME}/Iggy/lib/redist64"
      "-DCOPY_DEST=$<TARGET_FILE_DIR:${TARGET_NAME}>"
      -P "${COPY_SCRIPT}"
    COMMENT "Copying redist files..."
    VERBATIM
  )

  add_dependencies(${TARGET_NAME} CopyRedist_${TARGET_NAME})
  set_property(TARGET CopyRedist_${TARGET_NAME} PROPERTY FOLDER "Build")
endfunction()
