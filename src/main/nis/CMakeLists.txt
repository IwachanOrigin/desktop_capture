
set(nis_srcs
  NIS_Main.hlsl
  NIS_Config.h
  NIS_Scaler.h
)

add_custom_target(${NIS_WIN} SOURCES ${nis_srcs})

