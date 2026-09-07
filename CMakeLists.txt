cmake_minimum_required(VERSION 3.20.0)

if (CONFIG_ZMK_DISPLAY_STATUS_SCREEN_CUSTOM)
  target_sources(app PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src/custom_status_screen.c)
  if (DEFINED ZMK_CONFIG)
    target_include_directories(app BEFORE PRIVATE
      ${ZMK_CONFIG}/include
      ${ZMK_CONFIG}
      ${ZMK_CONFIG}/../include
    )
  endif()
  target_include_directories(app PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/src
  )
endif()

