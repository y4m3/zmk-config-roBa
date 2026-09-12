zephyr_library()
zephyr_library_sources_ifdef(CONFIG_ZMK_BLE src/behavior_profile_settings.c)
zephyr_include_directories(${APPLICATION_SOURCE_DIR}/include)
