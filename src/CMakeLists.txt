add_executable(${PROJECT_NAME}
    game/GameManager.cpp
    gamepad/AllySystemControl.cpp
    steam/SteamIntegration.cpp
    ui/LauncherWindow.cpp
)

target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${STEAM_SDK_PATH}
)

target_link_libraries(${PROJECT_NAME} PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    Qt6::Network
    Qt6::WebEngineWidgets
    Qt6::Gamepad
    SDL3::SDL3
    OpenGL::GL
    ${STEAM_API_LIB}
)