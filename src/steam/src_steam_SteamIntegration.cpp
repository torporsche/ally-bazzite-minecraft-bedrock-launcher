#include "SteamIntegration.hpp"
#include <QDir>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QStandardPaths>
#include <QTimer>
#include <QProcess>
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
    , m_gameModeActive(false) {
    
    detectSteamInstallation();
    monitorSteamProcess();
    
    // Check status periodically
    QTimer* statusTimer = new QTimer(this);
    connect(statusTimer, &QTimer::timeout, this, [this]() {
        checkGameMode();
    });
    statusTimer->start(1000); // Check every second
}

SteamIntegration::~SteamIntegration() {
}

void SteamIntegration::detectSteamInstallation() {
    // Check common Steam installation paths for Bazzite OS
    QStringList searchPaths = {
        QDir::homePath() + "/.local/share/Steam",
        QDir::homePath() + "/.steam/steam",
        "/usr/share/steam"
    };
    
    for (const QString& path : searchPaths) {
        if (QFile::exists(path + "/steam")) {
            m_steamPath = path;
            break;
        }
    }
    
    if (m_steamPath.isEmpty()) {
        qWarning() << "Steam installation not found";
        return;
    }
    
    // Get active user from Steam config
    QString configPath = findSteamConfig();
    if (!configPath.isEmpty()) {
        QFile configFile(configPath);
        if (configFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
            QString content = configFile.readAll();
            QRegularExpression re("\"AutoLoginUser\"\\s+\"([^\"]+)\"");
            QRegularExpressionMatch match = re.match(content);
            if (match.hasMatch()) {
                m_activeUser = match.captured(1);
            }
        }
    }
}

void SteamIntegration::monitorSteamProcess() {
    QTimer* checkTimer = new QTimer(this);
    connect(checkTimer, &QTimer::timeout, this, [this]() {
        QProcess process;
        process.start("pidof", {"steam"});
        process.waitForFinished();
        
        bool running = (process.exitCode() == 0);
        if (running != m_steamRunning) {
            m_steamRunning = running;
            emit steamStatusChanged(running);
        }
        
        // Check Big Picture Mode
        if (m_steamRunning) {
            process.start("wmctrl", {"-l"});
            process.waitForFinished();
            QString output = process.readAll();
            bool bpm = output.contains("Steam Big Picture Mode");
            
            if (bpm != m_bigPictureMode) {
                m_bigPictureMode = bpm;
                emit bigPictureModeChanged(bpm);
            }
        }
    });
    checkTimer->start(2000); // Check every 2 seconds
}

void SteamIntegration::checkGameMode() {
    // Check if running in Steam GameMode
    QProcess process;
    process.start("ps", {"-A"});
    process.waitForFinished();
    QString output = process.readAll();
    
    bool gameMode = output.contains("gamescope") || 
                    output.contains("steamos-session") ||
                    QFile::exists("/run/gamescope");
    
    if (gameMode != m_gameModeActive) {
        m_gameModeActive = gameMode;
        emit gameModeChanged(gameMode);
    }
}

QString SteamIntegration::findSteamConfig() const {
    QStringList configPaths = {
        m_steamPath + "/config/loginusers.vdf",
        QDir::homePath() + "/.steam/steam/config/loginusers.vdf"
    };
    
    for (const QString& path : configPaths) {
        if (QFile::exists(path)) {
            return path;
        }
    }
    
    return QString();
}

bool SteamIntegration::isSteamRunning() const {
    return m_steamRunning;
}

bool SteamIntegration::isBigPictureMode() const {
    return m_bigPictureMode;
}

bool SteamIntegration::isGameModeActive() const {
    return m_gameModeActive;
}

bool SteamIntegration::launchInBigPicture(const QString& appId) {
    if (!m_steamRunning) {
        QProcess::startDetached("steam", {"-bigpicture"});
        return true;
    }
    
    if (!appId.isEmpty()) {
        QProcess::startDetached("steam", 
            {"steam://rungameid/" + appId});
        return true;
    }
    
    return false;
}

bool SteamIntegration::addNonSteamGame(
    const QString& execPath, 
    const QString& name
) {
    if (!m_steamRunning || m_activeUser.isEmpty()) {
        return false;
    }
    
    // Find shortcuts.vdf
    QString shortcutsPath = m_steamPath + "/userdata/" + 
                           m_activeUser + "/config/shortcuts.vdf";
    
    // Create shortcuts file if it doesn't exist
    if (!QFile::exists(shortcutsPath)) {
        QFile file(shortcutsPath);
        if (!file.open(QIODevice::WriteOnly)) {
            return false;
        }
        file.close();
    }
    
    // Add game to Steam
    QProcess process;
    process.start("steam", {
        "-silent", 
        "-addnonsteamgame", 
        execPath,
        "-name",
        name
    });
    return process.waitForFinished();
}

bool SteamIntegration::removeNonSteamGame(const QString& appId) {
    if (!m_steamRunning || m_activeUser.isEmpty()) {
        return false;
    }
    
    QString shortcutsPath = m_steamPath + "/userdata/" + 
                           m_activeUser + "/config/shortcuts.vdf";
    
    if (!QFile::exists(shortcutsPath)) {
        return false;
    }
    
    // Remove game from Steam
    QProcess process;
    process.start("steam", {
        "-silent", 
        "-removenonsteamgame", 
        appId
    });
    return process.waitForFinished();
}

QString SteamIntegration::getLastActiveUser() const {
    return m_activeUser;
}