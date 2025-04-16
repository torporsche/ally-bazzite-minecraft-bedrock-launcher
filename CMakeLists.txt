cmake_minimum_required(VERSION 3.20)
project(ally-mc-launcher VERSION 0.1.0 LANGUAGES CXX)

# C++ standard requirements
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Find required packages
find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets Network WebEngineWidgets Gamepad)
find_package(SDL3 REQUIRED)
find_package(Wayland REQUIRED)
find_package(EGL REQUIRED)
find_package(OpenGL REQUIRED)

# Add subdirectories
add_subdirectory(src)

# Package configuration
set(CPACK_GENERATOR "DEB")
set(CPACK_PACKAGE_VERSION_MAJOR "0")
set(CPACK_PACKAGE_VERSION_MINOR "1")
set(CPACK_PACKAGE_VERSION_PATCH "0")
set(CPACK_PACKAGE_CONTACT "torporsche")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Minecraft Bedrock Launcher for ROG Ally/Bazzite OS")
include(CPack)