
set(hlsl_srcs
  common.fxh
  videorgb32ps.hlsl
  videorgb32vs.hlsl
)

# options for shader debug mode:  /Zi /Zss /Od /FdColorConvPS.pdb
set_source_files_properties(videorgb32vs.hlsl
  PROPERTIES VS_SHADER_FLAGS "/Vn g_videoRGB32VS /Fh \"${CMAKE_CURRENT_SOURCE_DIR}/videoRGB32VS.h\""
  VS_SHADER_TYPE Vertex
  VS_SHADER_MODEL 5.0
  VS_SHADER_ENTRYPOINT VS )
set_source_files_properties(videorgb32ps.hlsl
  PROPERTIES VS_SHADER_FLAGS "/Vng_videoRGB32PS /Fh \"${CMAKE_CURRENT_SOURCE_DIR}/videoRGB32PS.h\""
  VS_SHADER_TYPE Pixel
  VS_SHADER_MODEL 5.0
  VS_SHADER_ENTRYPOINT PS )

add_custom_target(hlsl SOURCES ${hlsl_srcs})

set(main_src
  deviceresources.cpp
  deviceresources.h
  dx11baserenderer.cpp
  dx11baserenderer.h
  dx11rgb32renderer.cpp
  dx11rgb32renderer.h
  dxhelper.h
  DXUtilities.h
  gpuhelper.cpp
  gpuhelper.h
  mesh.cpp
  mesh.h
  nvscaler.cpp
  nvscaler.h
  pipeline.cpp
  pipeline.h
  SimpleMath.cpp
  SimpleMath.h
  SimpleMath.inl
  Utilities.h
  win32messagehandler.cpp
  win32messagehandler.h
)

add_library(${CORE_RENDERER_WIN_LIB} ${main_src})
add_dependencies(${CORE_RENDERER_WIN_LIB} hlsl ${NIS_WIN})

