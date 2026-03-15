# Cross-platform single file copy using fast native tools
#
# Required:
#   COPY_SOURCE – pipe-separated list of source file paths
#   COPY_DEST  – destination directory

if(NOT COPY_SOURCE OR NOT COPY_DEST)
  message(FATAL_ERROR "COPY_SOURCE and COPY_DEST must be set.")
endif()

string(REPLACE "|" ";" COPY_SOURCE "${COPY_SOURCE}")

if(CMAKE_HOST_WIN32)
  # Group files by parent directory so we can batch them into single robocopy calls
  set(dir_file_map "")
  set(unique_dirs "")

  foreach(filepath IN LISTS COPY_SOURCE)
    get_filename_component(dir "${filepath}" DIRECTORY)
    get_filename_component(name "${filepath}" NAME)

    if(NOT dir IN_LIST unique_dirs)
      list(APPEND unique_dirs "${dir}")
    endif()

    list(APPEND "FILES_IN_${dir}" "${name}")
  endforeach()

  foreach(dir IN LISTS unique_dirs)
    execute_process(
      COMMAND robocopy.exe "${dir}" "${COPY_DEST}" ${FILES_IN_${dir}} /MT /R:0 /W:0 /NP 
      RESULT_VARIABLE rc
    )

    if(rc GREATER 7) # Allows for "files copied" and "no files copied" cases, but treats actual errors as failures
      message(FATAL_ERROR "robocopy failed (exit code ${rc})")
    endif()
  endforeach()

elseif(CMAKE_HOST_UNIX)
  execute_process(
    COMMAND rsync -av ${COPY_SOURCE} "${COPY_DEST}/"
    RESULT_VARIABLE rs
  )

  if(rs GREATER 0) # Any non-zero exit code indicates an error
    message(FATAL_ERROR "rsync failed (exit code ${rs})")
  endif()
else()
  message(FATAL_ERROR "Unsupported host platform for asset copying.")
endif()