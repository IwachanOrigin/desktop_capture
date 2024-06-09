
set(main_src
  app.cpp
  app.h
  capturecontroller.h
  capturecontroller.cpp
  capturehelper.h
  composition.interop.h
  d3dHelpers.h
  direct3d11.interop.h
  hwndcapture.cpp
  hwndcapture.h
  monitorlist.cpp
  monitorlist.h
)

add_library(
  ${CORE_CAPTURE_WIN_LIB}
  ${main_src}
)

