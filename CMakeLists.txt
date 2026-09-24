cmake_minimum_required(VERSION 3.20.0)

if (CONFIG_ZMK_DISPLAY_STATUS_SCREEN_CUSTOM)
  target_sources(app PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src/status_screen.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/engine.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/canvas.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/transform.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/font_renderer.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/events.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/widgets/dispatch.c
  )

  target_sources_ifdef(CONFIG_SCYAN_WIDGET_OUTPUT app PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src/widgets/widget_output.c)
  target_sources_ifdef(CONFIG_SCYAN_WIDGET_BATTERY app PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src/widgets/widget_battery.c)
  target_sources_ifdef(CONFIG_SCYAN_WIDGET_LAYER app PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src/widgets/widget_layer.c)
  target_sources_ifdef(CONFIG_SCYAN_WIDGET_WPM app PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src/widgets/widget_wpm.c)
  target_sources_ifdef(CONFIG_SCYAN_WIDGET_BRANDING app PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src/widgets/widget_branding.c)
  target_sources_ifdef(CONFIG_SCYAN_WIDGET_SPLIT app PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src/widgets/widget_split.c)
  target_sources_ifdef(CONFIG_SCYAN_WIDGET_SCREENSAVER app PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src/widgets/widget_screensaver.c)
  target_sources_ifdef(CONFIG_SCYAN_WIDGET_CAPS app PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src/widgets/widget_caps.c)
  target_sources_ifdef(CONFIG_SCYAN_WIDGET_BONGO app PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src/widgets/widget_bongo.c)
  target_sources_ifdef(CONFIG_SCYAN_WIDGET_LOOP app PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src/widgets/widget_loop.c)
  target_sources_ifdef(CONFIG_SCYAN_WIDGET_TYPEWRITER app PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src/widgets/widget_typewriter.c)
  target_sources_ifdef(CONFIG_SCYAN_WIDGET_KEYPRESS app PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src/widgets/widget_keypress.c)

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
