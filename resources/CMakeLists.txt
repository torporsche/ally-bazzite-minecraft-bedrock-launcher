# Install configuration files
install(FILES
    config/default_config.json
    config/error_messages.json
    config/input_config.json
    config/performance_profiles.yml
    config/shader_optimization.yml
    config/ui_layout.yml
    DESTINATION ${CMAKE_INSTALL_SYSCONFDIR}/${PROJECT_NAME}/config
)

# Install gamepad configurations
install(FILES
    gamepad/ally_default.vdf
    DESTINATION ${CMAKE_INSTALL_DATADIR}/${PROJECT_NAME}/gamepad
)

# Install icons
install(FILES
    icons/ally-mc-launcher.png
    DESTINATION ${CMAKE_INSTALL_DATADIR}/icons/hicolor/256x256/apps
)