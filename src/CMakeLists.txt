
cmake_minimum_required(VERSION 3.14)

if (POLICY CMP0087)
  cmake_policy(SET CMP0087 NEW)
endif()

project(desktopCapture LANGUAGES CXX)

# output directory
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
# output compile_commands.json
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
add_definitions(-DWIN32_LEAN_AND_MEAN -DNOMINMAX -DUNICODE -D_UNICODE)

# Enable Link Time Code Generation to Visual Studio
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /GL")
set(CMAKE_EXE_LINKER_FLAGS_RELEASE "${CMAKE_EXE_LINKER_FLAGS_RELEASE} /LTCG")

# Enable Visual Studio filtering
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

# Enable separate link
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /INCREMENTAL")

# Keep the auto-generated files together in the Visual Studio source tree.
# Because by default it it added to Source Files in the Visual Studio source tree.
# This is very hard to see.
set_property(GLOBAL PROPERTY AUTOGEN_TARGETS_FOLDER AutoGenFiles)
set_property(GLOBAL PROPERTY AUTOGEN_SOURCE_GROUP AutoGenFiles)

if (WIN32)
  # Set wil include directory
  include_directories(
    main/videoendecode
    lib/wil/include
  )
else()
  # Linux

endif(WIN32)

find_package(Git REQUIRED)

# Define the version numbers
set(SOFTWARE_VERSION_MAJOR 0)
set(SOFTWARE_VERSION_MINOR 0)
set(SOFTWARE_VERSION_PATCH 0)

# Output version.h
execute_process(
  COMMAND ${GIT_EXECUTABLE} rev-parse --short HEAD
  WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
  OUTPUT_VARIABLE GIT_SHA1
  OUTPUT_STRIP_TRAILING_WHITESPACE
)

# Configure version.h.in to create version.h
configure_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/version.h.in
  ${CMAKE_CURRENT_SOURCE_DIR}/version.h
)

if (WIN32)
  INCLUDE_DIRECTORIES(
    ${CMAKE_CURRENT_SOURCE_DIR} # For version.h
    lib/wil/include
    main/core/win/capture
    main/core/win/renderer
    main/core/win/callback
    main/core/win/common
    main/nis
    main/setupwizard
  )
  # Precompiled header
  set_source_files_properties(main/core/win/common/pch.h PROPERTIES HEADER_FILE_ONLY TRUE)
  add_library(PCH_WIN OBJECT main/core/win/common/pch.cpp)
  target_precompile_headers(PCH_WIN PRIVATE main/core/win/common/pch.h)
else()
  INCLUDE_DIRECTORIES(
    ${CMAKE_CURRENT_SOURCE_DIR} # For version.h
  )
endif()

# CaptureCoreWin is lib name by core/win/capture directory.
set(CORE_CAPTURE_WIN_LIB "CoreCaptureWin")
# CoreRendererWin is lib name by core/win/renderer directory.
set(CORE_RENDERER_WIN_LIB "CoreRendererWin")
# NIS_Win is custom target name by nis directory.
#set(NIS_WIN "NIS_Win")

# Set up the copy nis files
set(NIS_COPY_FILES
  ${CMAKE_CURRENT_SOURCE_DIR}/main/nis/NIS_Config.h
  ${CMAKE_CURRENT_SOURCE_DIR}/main/nis/NIS_Main.hlsl
  ${CMAKE_CURRENT_SOURCE_DIR}/main/nis/NIS_Scaler.h
)

add_subdirectory(main)

