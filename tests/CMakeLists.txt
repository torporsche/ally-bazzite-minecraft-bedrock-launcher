add_executable(TestSuite
    TestSuite.cpp
    # Link directly to source files for testing
    ${CMAKE_SOURCE_DIR}/src/core/Config.cpp
    ${CMAKE_SOURCE_DIR}/src/game/GameManager.cpp
    ${CMAKE_SOURCE_DIR}/src/gamepad/AllySystemControl.cpp
    ${CMAKE_SOURCE_DIR}/src/steam/SteamIntegration.cpp
    ${CMAKE_SOURCE_DIR}/src/ui/LauncherWindow.cpp
)

target_link_libraries(TestSuite PRIVATE
    Qt6::Test
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    Qt6::Network
    Qt6::WebEngineWidgets
    SDL2::SDL2
    OpenGL::GL
)

# Link Steam API if available
if(STEAM_API_LIB)
    target_link_libraries(TestSuite PRIVATE ${STEAM_API_LIB})
endif()

target_include_directories(TestSuite PRIVATE
    ${CMAKE_SOURCE_DIR}/src
)

# Add Steam SDK include if available
if(STEAM_SDK_PATH)
    target_include_directories(TestSuite PRIVATE ${STEAM_SDK_PATH})
endif()

add_test(NAME TestSuite COMMAND TestSuite)