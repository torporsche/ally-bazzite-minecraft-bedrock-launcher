add_executable(${PROJECT_NAME}
    main.cpp
    core/Config.cpp
    game/GameManager.cpp
    gamepad/AllySystemControl.cpp
    steam/SteamIntegration.cpp
    ui/LauncherWindow.cpp
)

# Enable Qt's automatic MOC, UIC, and RCC processing
set_target_properties(${PROJECT_NAME} PROPERTIES
    AUTOMOC ON
    AUTOUIC ON
    AUTORCC ON
)

target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)

# Add Steam SDK include if available
if(STEAM_SDK_PATH)
    target_include_directories(${PROJECT_NAME} PRIVATE ${STEAM_SDK_PATH})
endif()

target_link_libraries(${PROJECT_NAME} PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    Qt6::Network
    Qt6::WebEngineWidgets
    Qt6::Gamepad
    SDL2::SDL2
    OpenGL::GL
)

# Link Steam API if available
if(STEAM_API_LIB)
    target_link_libraries(${PROJECT_NAME} PRIVATE ${STEAM_API_LIB})
endif()