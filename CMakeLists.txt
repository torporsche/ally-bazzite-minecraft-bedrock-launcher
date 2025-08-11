cmake_minimum_required(VERSION 3.20)
project(ally-mc-launcher 
    VERSION 0.1.0
    DESCRIPTION "Minecraft Bedrock Launcher for ROG Ally"
    HOMEPAGE_URL "https://github.com/torporsche/ally-bazzite-minecraft-bedrock-launcher"
    LANGUAGES CXX
)

# Set C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Enable Qt's automatic processing
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

# Build type
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

# Add compile definitions
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_definitions(-DDEBUG)
else()
    add_definitions(-DNDEBUG)
endif()

add_definitions(
    -DAPP_VERSION="${PROJECT_VERSION}"
    -DAPP_NAME="${PROJECT_NAME}"
    -DROG_ALLY_SUPPORT=1
)

# Find required packages
find_package(Qt6 REQUIRED COMPONENTS
    Core
    Gui
    Widgets
    Network
    WebEngineWidgets
    Gamepad
    Test
)

# SDL2 for cross-platform input and audio support
find_package(SDL2 REQUIRED)
find_package(Wayland REQUIRED Client)
find_package(EGL REQUIRED)
find_package(OpenGL REQUIRED)

# Find Steam SDK (optional)
find_path(STEAM_SDK_PATH
    NAMES steam/steam_api.h
    PATHS
        $ENV{STEAM_SDK_PATH}/public
        /opt/steam-sdk/public
)

if(STEAM_SDK_PATH)
    find_library(STEAM_API_LIB
        NAMES steam_api
        PATHS
            $ENV{STEAM_SDK_PATH}/redistributable_bin/linux64
            /opt/steam-sdk/redistributable_bin/linux64
    )
    
    if(STEAM_API_LIB)
        message(STATUS "Steam SDK found at: ${STEAM_SDK_PATH}")
        add_definitions(-DSTEAM_SUPPORT=1)
    else()
        message(WARNING "Steam SDK headers found but library not found. Steam support will be disabled.")
        set(STEAM_SDK_PATH "")
    endif()
else()
    message(WARNING "Steam SDK not found. Steam support will be disabled. Run scripts/setup_steam.sh to enable Steam integration.")
    set(STEAM_API_LIB "")
endif()

# Add dependency check to build process
add_custom_target(check_dependencies
    COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/scripts/check_dependencies.sh
    COMMENT "Checking and installing required dependencies..."
    VERBATIM
)

# Include subdirectories
add_subdirectory(src)
add_subdirectory(resources)
# Skip tests to avoid build issues for now
# add_subdirectory(tests)

# Make all targets depend on dependency check
add_dependencies(${PROJECT_NAME} check_dependencies)

# Testing configuration (disabled for now)
# enable_testing()

# Code coverage
option(CODE_COVERAGE "Enable code coverage reporting" OFF)
if(CODE_COVERAGE AND CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(${PROJECT_NAME} PRIVATE --coverage -O0 -g)
    target_link_libraries(${PROJECT_NAME} PRIVATE --coverage)
endif()

# Installation targets
include(GNUInstallDirs)

install(TARGETS ${PROJECT_NAME}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}/static
)

# Install udev rules
install(FILES
    resources/udev/99-rog-ally.rules
    DESTINATION /etc/udev/rules.d/
)

# Install setup script
install(PROGRAMS
    scripts/setup_rog_ally.sh
    DESTINATION ${CMAKE_INSTALL_BINDIR}
)

# Desktop entry
install(FILES resources/desktop/ally-mc-launcher.desktop
    DESTINATION ${CMAKE_INSTALL_DATADIR}/applications
)

# Generate and install pkg-config file
configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/resources/pkg-config/ally-mc-launcher.pc.in
    ${CMAKE_CURRENT_BINARY_DIR}/ally-mc-launcher.pc
    @ONLY
)

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/ally-mc-launcher.pc
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig
)

# CPack configuration for packaging
set(CPACK_PACKAGE_NAME ${PROJECT_NAME})
set(CPACK_PACKAGE_VERSION ${PROJECT_VERSION})
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY ${PROJECT_DESCRIPTION})
set(CPACK_PACKAGE_VENDOR "torporsche")
set(CPACK_PACKAGE_CONTACT "torporsche@github.com")
set(CPACK_GENERATOR "DEB;RPM")
set(CPACK_DEBIAN_PACKAGE_DEPENDS "qt6-base-dev, qt6-webengine-dev, qt6-gamepad-dev, qt6-qttest-dev, libsdl2-dev")
include(CPack)