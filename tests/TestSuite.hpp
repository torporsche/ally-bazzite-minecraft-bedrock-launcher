#pragma once

#include <QObject>
#include <QtTest>
#include <memory>

class TestSuite : public QObject {
    Q_OBJECT

private slots:
    // Steam Integration Tests
    void testSteamInitialization();
    void testSteamGamepadConfig();
    void testSteamBigPictureMode();
    void testSteamOverlay();

    // ROG Ally Hardware Tests
    void testPerformanceProfiles();
    void testTDPControl();
    void testGPUFrequency();
    void testFanControl();
    void testTemperatureMonitoring();
    void testBatteryMonitoring();

    // Build System Tests
    void testInstallationPaths();
    void testConfigurationFiles();
    void testDesktopIntegration();

    // Game Optimization Tests
    void testGraphicsPresets();
    void testShaderCache();
    void testVulkanLayers();
    void testGameScope();

    // UI Tests
    void testTouchInput();
    void testGestures();
    void testBigPictureMode();
    void testUIScaling();
};