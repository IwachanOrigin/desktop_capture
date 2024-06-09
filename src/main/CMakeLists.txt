
#add_subdirectory(nis)
add_subdirectory(core/win/capture)
add_subdirectory(core/win/renderer)

set(PROJECT_SOURCES
  main.cpp
  setupwizard/areaselecter.cpp
  setupwizard/areaselecter.h
  setupwizard/capturesetupwizard.cpp
  setupwizard/capturesetupwizard.h
)

if(WIN32)
  add_executable(
    ${PROJECT_NAME}
    #WIN32
    ${PROJECT_SOURCES}
  )
  # Link
  target_link_libraries(
    ${PROJECT_NAME}
    PRIVATE
    ${CORE_CAPTURE_WIN_LIB}
    ${CORE_RENDERER_WIN_LIB}
    windowsapp.lib
    d3d11.lib
    d3dcompiler.lib
    dxgi.lib
    msimg32.lib
    uxtheme.lib
  )
else()
  add_executable(
    ${PROJECT_NAME}
    ${PROJECT_SOURCES}
  )
  # Link
  target_link_libraries(
    ${PROJECT_NAME}
    PRIVATE
    ${CORE_CAPTURE_WIN_LIB}
    ${CORE_RENDERER_WIN_LIB}
  )
endif()

################################################################
# NIS
###############################################################

set(NIS_OUTPUT_DIR ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/$<CONFIG>/Shader)
if (NOT EXISTS ${NIS_OUTPUT_DIR})
  add_custom_command(
    TARGET ${PROJECT_NAME}
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory "${NIS_OUTPUT_DIR}"
    COMMENT "Create NIS file to output directory"
  )
endif()

foreach(FILE IN LISTS NIS_COPY_FILES)
  # Get the file name of the DLL
  get_filename_component(NIS_FILENAME "${FILE}" NAME)

  # Command to copy the nis file
  add_custom_command(
    TARGET ${PROJECT_NAME}
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy "${FILE}" "${NIS_OUTPUT_DIR}/${NIS_FILENAME}"
    COMMENT "Copying NIS file to output directory"
  )
endforeach()


