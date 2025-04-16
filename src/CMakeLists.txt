# Source CMakeLists.txt
# Updated: 2025-04-16 23:19:51 UTC

set(SOURCES
    main.cpp
    auth/GooglePlayAuth.cpp
    auth/GooglePlayAPI.cpp
    game/GameManager.cpp
    gamepad/AllyGamepad.cpp
    gamepad/AllySystemControl.cpp
    steam/SteamIntegration.cpp
    steam/SteamBPMIntegration.cpp
    ui/LauncherWindow.cpp
    ui/SettingsDialog.cpp
    ui/VersionInstallDialog.cpp
    core/Config.cpp
)

set(HEADERS
    auth/GooglePlayAuth.hpp
    auth/GooglePlayAPI.hpp
    game/GameManager.hpp
    gamepad/AllyGamepad.hpp
    gamepad/AllySystemControl.hpp
    steam/SteamIntegration.hpp
    steam/SteamBPMIntegration.hpp
    ui/LauncherWindow.hpp
    ui/SettingsDialog.hpp
    ui/VersionInstallDialog.hpp
    core/Config.hpp
)

add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS})

target_link_libraries(${PROJECT_NAME} PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    Qt6::Network
    Qt6::WebEngineWidgets
    Qt6::Gamepad
    SDL3::SDL3
    Wayland::Client
    EGL::EGL
    OpenGL::GL
)

target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)
