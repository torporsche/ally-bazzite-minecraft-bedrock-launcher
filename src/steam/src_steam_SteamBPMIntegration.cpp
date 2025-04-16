#include "SteamBPMIntegration.hpp"
#include <QFile>
#include <QDir>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QCryptographicHash>
#include <QProcess>
#include <QStandardPaths>

SteamBPMIntegration* SteamBPMIntegration::s_instance = nullptr;

SteamBPMIntegration* SteamBPMIntegration::instance() {
    if (!s_instance) {
        s_instance = new SteamBPMIntegration();
    }
    return s_instance;
}

SteamBPMIntegration::SteamBPMIntegration(QObject* parent)
    : QObject(parent) {
    
    // Find Steam user configuration
    m_steamConfigPath = findSteamUserConfig();
    
    if (!m_steamConfigPath.isEmpty()) {
        // Extract Steam User ID from path
        QRegExp rx("/userdata/(\\d+)/");
        if (rx.indexIn(m_steamConfigPath) != -1) {
            m_steamUserId = rx.cap(1);
        }
    }
    
    // Load existing shortcuts
    loadExistingShortcuts();
}

bool SteamBPMIntegration::registerWithSteam(
    const InstalledVersion& version
) {
    if (m_steamConfigPath.isEmpty() || m_steamUserId.isEmpty()) {
        return false;
    }
    
    // Create Steam shortcut
    if (!createSteamShortcut(version)) {
        return false;
    }
    
    // Setup Steam Input configuration
    QString shortcutId = getSteamShortcutId(version.versionInfo.version);
    if (!setupSteamInputConfig(shortcutId)) {
        return false;
    }
    
    // Setup grid art
    if (!setupSteamGridArt(shortcutId)) {
        return false;
    }
    
    emit registrationComplete(version.versionInfo.version, true);
    return true;
}

bool SteamBPMIntegration::createSteamShortcut(
    const InstalledVersion& version
) {
    QMap<QString, QString> properties;
    
    // Set basic properties
    properties["AppName"] = "Minecraft Bedrock " + 
                          version.versionInfo.versionString;
    properties["Exe"] = version.installPath + "/minecraft-launcher";
    properties["StartDir"] = version.installPath;
    properties["LaunchOptions"] = "--fullscreen --gamemode";
    properties["IsHidden"] = "0";
    properties["AllowDesktopConfig"] = "1";
    properties["AllowOverlay"] = "1";
    properties["LastPlayTime"] = "0";
    
    // Generate shortcut ID
    QString shortcutId = generateShortcutId(version);
    m_shortcutIds[version.versionInfo.version] = shortcutId;
    
    return modifySteamShortcut(shortcutId, properties);
}

bool SteamBPMIntegration::setupSteamInputConfig(const QString& shortcutId) {
    // Create Steam Input configuration directory
    QString configDir = m_steamConfigPath + "/config/controller_configs/" + 
                       shortcutId;
    QDir().mkpath(configDir);
    
    // Copy default ROG Ally configuration
    QString defaultConfig = ":/steam/configs/rog_ally_minecraft.vdf";
    QString targetConfig = configDir + "/controller_config.vdf";
    
    return QFile::copy(defaultConfig, targetConfig);
}

bool SteamBPMIntegration::setupSteamGridArt(const QString& shortcutId) {
    // Create grid art directory
    QString gridDir = m_steamConfigPath + "/grid";
    QDir().mkpath(gridDir);
    
    // Copy grid artwork
    QStringList artTypes = {
        "p" // Box art (460x215)
        "v" // Hero art (1920x620)
        "l" // Logo (640x360)
    };
    
    bool success = true;
    for (const QString& type : artTypes) {
        QString artFile = ":/steam/grid/minecraft_" + type + ".jpg";
        QString targetFile = gridDir + "/" + shortcutId + "_" + type + ".jpg";
        success &= QFile::copy(artFile, targetFile);
    }
    
    return success;
}

QString SteamBPMIntegration::generateShortcutId(
    const InstalledVersion& version
) {
    // Generate unique ID based on version and install path
    QString unique = version.versionInfo.versionString + 
                    version.installPath;
    QByteArray hash = QCryptographicHash::hash(
        unique.toUtf8(), 
        QCryptographicHash::Sha1
    );
    return hash.toHex();
}

bool SteamBPMIntegration::modifySteamShortcut(
    const QString& shortcutId,
    const QMap<QString, QString>& properties
) {
    QString shortcutsFile = m_steamConfigPath + 
                           "/config/shortcuts.vdf";
    
    // Read existing shortcuts
    QFile file(shortcutsFile);
    if (!file.open(QIODevice::ReadWrite)) {
        return false;
    }
    
    QByteArray data = file.readAll();
    file.close();
    
    // Parse VDF format (Steam's binary format)
    // This is a simplified version, production code should use
    // a proper VDF parser
    QJsonObject shortcuts;
    // ... Parse VDF format ...
    
    // Modify or add shortcut
    shortcuts[shortcutId] = QJsonObject::fromVariantMap(properties);
    
    // Write back to file
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        return false;
    }
    
    // Convert back to VDF format
    // ... Convert to VDF format ...
    
    file.write(data);
    file.close();
    
    return true;
}

QString SteamBPMIntegration::findSteamUserConfig() const {
    // Check common Steam paths
    QStringList steamPaths = {
        QDir::homePath() + "/.local/share/Steam",
        QDir::homePath() + "/.steam/steam",
        "/usr/share/steam"
    };
    
    for (const QString& basePath : steamPaths) {
        QDir userDataDir(basePath + "/userdata");
        if (userDataDir.exists()) {
            // Get first user directory (assuming single user)
            QStringList users = userDataDir.entryList(
                QDir::Dirs | QDir::NoDotAndDotDot
            );
            if (!users.isEmpty()) {
                return userDataDir.filePath(users.first());
            }
        }
    }
    
    return QString();
}

bool SteamBPMIntegration::enableOverlay(const InstalledVersion& version) {
    QString shortcutId = getSteamShortcutId(version.versionInfo.version);
    if (shortcutId.isEmpty()) return false;
    
    QMap<QString, QString> properties;
    properties["AllowOverlay"] = "1";
    
    bool success = modifySteamShortcut(shortcutId, properties);
    if (success) {
        emit overlayStatusChanged(version.versionInfo.version, true);
    }
    return success;
}

bool SteamBPMIntegration::enableCloudSync(const InstalledVersion& version) {
    QString shortcutId = getSteamShortcutId(version.versionInfo.version);
    if (shortcutId.isEmpty()) return false;
    
    // Create cloud config directory
    QString cloudConfigDir = m_steamConfigPath + 
                           "/config/cloudsync/" + 
                           shortcutId;
    QDir().mkpath(cloudConfigDir);
    
    // Create cloud sync configuration
    QJsonObject config;
    config["enabled"] = true;
    config["paths"] = QJsonArray{
        version.dataPath + "/worlds",
        version.dataPath + "/resource_packs"
    };
    
    QFile configFile(cloudConfigDir + "/config.json");
    if (!configFile.open(QIODevice::WriteOnly)) {
        return false;
    }
    
    configFile.write(QJsonDocument(config).toJson());
    configFile.close();
    
    emit cloudSyncStatusChanged(version.versionInfo.version, true);
    return true;
}

QString SteamBPMIntegration::getSteamShortcutId(
    const QVersionNumber& version
) const {
    return m_shortcutIds.value(version);
}