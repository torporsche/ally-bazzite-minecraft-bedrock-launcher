#include "SteamIntegration.hpp"
#include <QDebug>
#include <QDir>
#include <QSettings>

SteamIntegration* SteamIntegration::s_instance = nullptr;

SteamIntegration* SteamIntegration::instance() {
    if (!s_instance) {
        s_instance = new SteamIntegration();
    }
    return s_instance;
}

SteamIntegration::SteamIntegration(QObject* parent) 
    : QObject(parent)
    , m_steamRunning(false)
    , m_bigPictureMode(false)
    , m_gameModeActive(false)
    , m_overlayEnabled(false)
    , m_cloudSyncEnabled(false) {
    detectSteamInstallation();
}

bool SteamIntegration::initialize() {
    if (m_steamRunning) {
        return true;
    }

    if (!SteamAPI_Init()) {
        qWarning() << "Failed to initialize Steam API";
        return false;
    }

    m_steamRunning = true;
    setupGamemodeEnvironment();
    configureControllerLayout();
    
    // Register callback
    m_callbackSteamOverlay = new STEAM_CALLBACK(
        SteamIntegration, 
        onGameOverlayActivated, 
        GameOverlayActivated_t
    );

    emit steamStatusChanged(true);
    return true;
}

bool SteamIntegration::setupGamemodeEnvironment() {
    // Set required environment variables for Steam Gamemode
    qputenv("SDL_VIDEODRIVER", "wayland");
    qputenv("STEAM_RUNTIME_PREFER_HOST_LIBRARIES", "1");
    qputenv("STEAM_GAMEPAD_CONFIG", "1");
    qputenv("STEAM_USE_MANGOAPP", "1");
    
    // Enable FSR if available
    qputenv("STEAM_GAMESCOPE_FSR", "1");
    
    return true;
}

bool SteamIntegration::configureControllerLayout() {
    if (!m_steamRunning) {
        return false;
    }

    // Initialize Steam Input
    if (!SteamInput()->Init(true)) {
        qWarning() << "Failed to initialize Steam Input";
        return false;
    }

    // Load default controller config
    const QString configPath = QDir(QCoreApplication::applicationDirPath())
        .filePath("gamepad/ally_default.vdf");
    
    if (QFile::exists(configPath)) {
        SteamInput()->LoadControllerConfig(configPath.toStdString().c_str());
    }

    return true;
}

bool SteamIntegration::loadSteamInputConfig(const QString& configPath) {
    if (!m_steamRunning || !QFile::exists(configPath)) {
        return false;
    }

    return SteamInput()->LoadControllerConfig(configPath.toStdString().c_str());
}

bool SteamIntegration::launchInBigPicture() {
    if (!m_steamRunning) {
        return false;
    }

    SteamAPI_RunCallbacks();
    m_bigPictureMode = true;
    emit bigPictureModeChanged(true);
    return true;
}

void SteamIntegration::onGameOverlayActivated(GameOverlayActivated_t* callback) {
    m_overlayEnabled = callback->m_bActive;
    emit overlayStatusChanged(m_overlayEnabled);
}

void SteamIntegration::detectSteamInstallation() {
    // Check for Steam installation
    #ifdef Q_OS_LINUX
    QString steamPath = QDir::homePath() + "/.local/share/Steam";
    #else
    QString steamPath = "C:/Program Files (x86)/Steam";
    #endif

    m_steamRunning = QDir(steamPath).exists();
}

SteamIntegration::~SteamIntegration() {
    if (m_steamRunning) {
        SteamAPI_Shutdown();
    }
}