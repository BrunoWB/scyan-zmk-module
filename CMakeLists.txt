cmake_minimum_required(VERSION 3.20.0)

if (CONFIG_ZMK_DISPLAY_STATUS_SCREEN_CUSTOM)
  target_sources(app PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src/status_screen.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/engine.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/canvas.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/transform.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/font_renderer.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/events.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/widgets/widget_output.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/widgets/widget_battery.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/widgets/widget_layer.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/widgets/widget_wpm.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/widgets/widget_branding.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/widgets/widget_split.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/widgets/widget_screensaver.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/widgets/widget_caps.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/widgets/widget_bongo.c
  )

  # Prioritize user config directory BEFORE any internal search paths
  # to guarantee scyan_assets.h exported by Scyan ZMK Studio is loaded.
  if (DEFINED ZMK_CONFIG)
    target_include_directories(app BEFORE PRIVATE
      ${ZMK_CONFIG}
      ${ZMK_CONFIG}/include
      ${ZMK_CONFIG}/../config
    )
  endif()

  target_include_directories(app PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/src
  )
endif()

