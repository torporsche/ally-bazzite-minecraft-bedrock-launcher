#include "TestSuite.hpp"
#include "../src/steam/SteamIntegration.hpp"
#include "../src/gamepad/AllySystemControl.hpp"
#include "../src/game/GameManager.hpp"
#include "../src/ui/LauncherWindow.hpp"
#include <QSignalSpy>

// Steam Integration Tests
void TestSuite::testSteamInitialization() {
    auto* steam = SteamIntegration::instance();
    QVERIFY(steam != nullptr);
    QVERIFY(steam->initialize());
    QVERIFY(steam->isSteamRunning());
    
    // Test Steam environment variables
    QCOMPARE(qgetenv("SDL_VIDEODRIVER"), QByteArray("wayland"));
    QCOMPARE(qgetenv("STEAM_RUNTIME_PREFER_HOST_LIBRARIES"), QByteArray("1"));
}

void TestSuite::testSteamGamepadConfig() {
    auto* steam = SteamIntegration::instance();
    QString configPath = QDir::currentPath() + "/resources/gamepad/ally_default.vdf";
    
    QVERIFY(QFile::exists(configPath));
    QVERIFY(steam->loadSteamInputConfig(configPath));
}

void TestSuite::testSteamBigPictureMode() {
    auto* steam = SteamIntegration::instance();
    QSignalSpy spy(steam, SIGNAL(bigPictureModeChanged(bool)));
    
    QVERIFY(steam->launchInBigPicture());
    QVERIFY(spy.wait(5000));
    QCOMPARE(spy.count(), 1);
    QVERIFY(steam->isBigPictureMode());
}

// ROG Ally Hardware Tests
void TestSuite::testPerformanceProfiles() {
    auto* control = AllySystemControl::instance();
    QSignalSpy spy(control, SIGNAL(performanceProfileChanged(PerformanceProfile)));
    
    QVERIFY(control->setPerformanceProfile(AllySystemControl::PerformanceProfile::BALANCED));
    QVERIFY(spy.wait());
    QCOMPARE(control->currentProfile(), AllySystemControl::PerformanceProfile::BALANCED);
    
    // Test profile-specific settings
    QCOMPARE(control->currentTDP(), 15);
    QVERIFY(control->setPerformanceProfile(AllySystemControl::PerformanceProfile::TURBO));
    QCOMPARE(control->currentTDP(), 25);
}

void TestSuite::testTDPControl() {
    auto* control = AllySystemControl::instance();
    QSignalSpy spy(control, SIGNAL(tdpChanged(int)));
    
    // Test valid TDP range
    QVERIFY(control->setTDP(15));
    QVERIFY(spy.wait());
    QCOMPARE(control->currentTDP(), 15);
    
    // Test invalid TDP values
    QVERIFY(!control->setTDP(4));  // Below minimum
    QVERIFY(!control->setTDP(31)); // Above maximum
}

void TestSuite::testTemperatureMonitoring() {
    auto* control = AllySystemControl::instance();
    QSignalSpy spy(control, SIGNAL(temperatureChanged(float)));
    
    // Wait for temperature update
    QVERIFY(spy.wait(2000));
    QVERIFY(spy.count() > 0);
    
    float temp = control->getCurrentTemperature();
    QVERIFY(temp >= 0.0f && temp <= 100.0f);
}

// Game Optimization Tests
void TestSuite::testGraphicsPresets() {
    auto* manager = GameManager::instance();
    QSignalSpy spy(manager, SIGNAL(graphicsPresetChanged(GraphicsPreset)));
    
    QVERIFY(manager->setGraphicsPreset(GameManager::GraphicsPreset::PERFORMANCE));
    QVERIFY(spy.wait());
    
    // Verify performance settings
    auto* control = AllySystemControl::instance();
    QCOMPARE(control->currentTDP(), 25);
    QVERIFY(manager->enableFSR(true));
}

void TestSuite::testShaderCache() {
    auto* manager = GameManager::instance();
    QString cachePath = QDir::homePath() + "/.local/share/minecraft/shader_cache";
    
    manager->optimizeShaderCache();
    QVERIFY(QDir(cachePath).exists());
    QCOMPARE(qgetenv("MESA_GLSL_CACHE_DIR"), cachePath.toUtf8());
}

void TestSuite::testVulkanLayers() {
    auto* manager = GameManager::instance();
    manager->setupVulkanLayers();
    
    QString layersPath = QDir::homePath() + "/.local/share/vulkan/implicit_layer.d/";
    QVERIFY(QDir(layersPath).exists());
    QVERIFY(qgetenv("VK_LAYER_MESA_overlay") == "1");
}

// UI Tests
void TestSuite::testTouchInput() {
    LauncherWindow window;
    QSignalSpy spy(&window, SIGNAL(touchGestureDetected()));
    
    // Simulate touch event
    QTouchEvent touchEvent(QEvent::TouchBegin,
                          nullptr,
                          Qt::NoModifier,
                          Qt::TouchPointPressed,
                          {QTouchEvent::TouchPoint(1)});
    
    QCoreApplication::sendEvent(&window, &touchEvent);
    QVERIFY(spy.wait());
}

void TestSuite::testGestures() {
    LauncherWindow window;
    
    // Test pinch gesture
    QPinchGesture pinch;
    pinch.setScaleFactor(1.5);
    pinch.setState(Qt::GestureFinished);
    
    QGestureEvent gestureEvent({&pinch});
    QCoreApplication::sendEvent(&window, &gestureEvent);
    
    // Verify UI scale changed
    QCOMPARE(window.property("scaling").toDouble(), 1.5);
}

void TestSuite::testUIScaling() {
    LauncherWindow window;
    window.show();
    
    // Test Big Picture Mode scaling
    window.toggleBigPictureMode(true);
    QVERIFY(window.isFullScreen());
    
    QFont font = QApplication::font();
    QCOMPARE(font.pointSizeF(), QApplication::font().pointSizeF() * 1.5);
}

// Build System Tests
void TestSuite::testInstallationPaths() {
    QString prefix = CMAKE_INSTALL_PREFIX;
    
    // Test binary installation
    QString binPath = prefix + "/bin/ally-mc-launcher";
    QVERIFY(QFile::exists(binPath));
    
    // Test desktop file installation
    QString desktopPath = prefix + "/share/applications/ally-mc-launcher.desktop";
    QVERIFY(QFile::exists(desktopPath));
    
    // Test configuration installation
    QString configPath = prefix + "/etc/ally-mc-launcher/default_config.json";
    QVERIFY(QFile::exists(configPath));
}

QTEST_MAIN(TestSuite)
#include "TestSuite.moc"