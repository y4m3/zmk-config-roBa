zephyr_library()
zephyr_library_sources_ifdef(CONFIG_ROBA_PROFILE_SETTINGS
                             src/behavior_profile_settings.c)
zephyr_include_directories(${APPLICATION_SOURCE_DIR}/include)
