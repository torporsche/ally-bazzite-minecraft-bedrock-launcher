#include "GameManager.hpp"
#include <QFile>
#include <QDir>
#include <QProcess>
#include <QSettings>
#include <QDebug>
#include <QJsonDocument>
#include <QJsonObject>
#include "../gamepad/AllySystemControl.hpp"

GameManager* GameManager::s_instance = nullptr;

GameManager* GameManager::instance() {
    if (!s_instance) {
        s_instance = new GameManager();
    }
    return s_instance;
}

GameManager::GameManager(QObject* parent)
    : QObject(parent)
    , m_currentPreset(GraphicsPreset::BALANCED)
    , m_currentAPI("vulkan")
    , m_targetFPS(60)
    , m_fsrEnabled(true) {
    
    // Initialize Vulkan layers map
    m_vulkanLayers = {
        {"VK_LAYER_MESA_overlay", "1"},
        {"VK_LAYER_VALVE_steam_fossilize", "1"},
        {"VK_LAYER_MESA_device_select", "1"}
    };
}

bool GameManager::applyROGAllyOptimizations() {
    setupVulkanLayers();
    configureGameScope();
    setupControllerHints();
    optimizeShaderCache();
    
    // Set environment variables for optimal performance
    qputenv("MESA_VK_WSI_PRESENT_MODE", "mailbox");
    qputenv("PROTON_FORCE_LARGE_ADDRESS_AWARE", "1");
    qputenv("PROTON_HIDE_NVIDIA_GPU", "1");
    
    return true;
}

void GameManager::setupVulkanLayers() {
    QString layersPath = QDir::homePath() + "/.local/share/vulkan/implicit_layer.d/";
    QDir().mkpath(layersPath);
    
    // Enable required Vulkan layers
    for (auto it = m_vulkanLayers.begin(); it != m_vulkanLayers.end(); ++it) {
        qputenv(it.key().toUtf8(), it.value().toUtf8());
    }
}

void GameManager::configureGameScope() {
    QProcess gamescope;
    QStringList args;
    
    args << "--force-grab-cursor"
         << "--adaptive-sync"
         << "--expose-wayland"
         << "--output-width" << "1920"
         << "--output-height" << "1080"
         << "--fps-limit" << QString::number(m_targetFPS);
         
    if (m_fsrEnabled) {
        args << "--fsr";
    }
    
    gamescope.startDetached("gamescope", args);
}

bool GameManager::setGraphicsPreset(GraphicsPreset preset) {
    auto* systemControl = AllySystemControl::instance();
    
    switch (preset) {
        case GraphicsPreset::BATTERY_SAVER:
            systemControl->setTDP(10);
            systemControl->setGPUFreq(1200);
            m_targetFPS = 30;
            break;
            
        case GraphicsPreset::BALANCED:
            systemControl->setTDP(15);
            systemControl->setGPUFreq(1600);
            m_targetFPS = 60;
            break;
            
        case GraphicsPreset::PERFORMANCE:
            systemControl->setTDP(25);
            systemControl->setGPUFreq(2000);
            m_targetFPS = 90;
            break;
    }
    
    m_currentPreset = preset;
    emit graphicsPresetChanged(preset);
    emit fpsLimitChanged(m_targetFPS);
    
    configureGameScope();
    return true;
}

void GameManager::setupControllerHints() {
    // Create controller hint file
    QString hintPath = QDir::homePath() + "/.local/share/minecraft/controller_hints.json";
    QFile hintFile(hintPath);
    
    if (hintFile.open(QIODevice::WriteOnly)) {
        QJsonObject hints;
        hints["controller_type"] = "xbox";
        hints["show_button_prompts"] = true;
        hints["vibration_enabled"] = true;
        
        QJsonDocument doc(hints);
        hintFile.write(doc.toJson());
        hintFile.close();
    }
}

void GameManager::optimizeShaderCache() {
    QString cachePath = QDir::homePath() + "/.local/share/minecraft/shader_cache";
    QDir().mkpath(cachePath);
    
    // Set environment variables for shader cache
    qputenv("MESA_GLSL_CACHE_DIR", cachePath.toUtf8());
    qputenv("__GL_SHADER_DISK_CACHE_PATH", cachePath.toUtf8());
    qputenv("__GL_SHADER_DISK_CACHE_SKIP_CLEANUP", "1");
}

bool GameManager::enableFSR(bool enabled) {
    m_fsrEnabled = enabled;
    configureGameScope();
    return true;
}

GameManager::~GameManager() {
    // Cleanup
}