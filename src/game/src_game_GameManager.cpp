#include "GameManager.hpp"
#include <QStandardPaths>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QProcess>
#include <QStorageInfo>
#include <QFile>
#include <QtConcurrent>
#include <sys/statvfs.h>

GameManager::GameManager(QObject* parent) 
    : QObject(parent) {
    
    // Setup base installation directory in user's home
    QString baseDir = QStandardPaths::writableLocation(QStandardPaths::HomeLocation);
    m_baseInstallDir = QDir(baseDir + "/.local/share/ally-mc-launcher");
    
    // Create necessary directories
    m_baseInstallDir.mkpath("versions");
    m_baseInstallDir.mkpath("data");
    
    // Load existing installations
    loadInstalledVersions();
}

void GameManager::loadInstalledVersions() {
    QFile configFile(m_baseInstallDir.filePath("versions.json"));
    if (!configFile.open(QIODevice::ReadOnly)) {
        return;
    }
    
    QJsonDocument doc = QJsonDocument::fromJson(configFile.readAll());
    QJsonObject versions = doc.object();
    
    for (const QString& key : versions.keys()) {
        QJsonObject verObj = versions[key].toObject();
        
        InstalledVersion version;
        version.versionInfo.version = QVersionNumber::fromString(key);
        version.versionInfo.versionString = verObj["versionString"].toString();
        version.versionInfo.versionCode = verObj["versionCode"].toString();
        version.versionInfo.isBeta = verObj["isBeta"].toBool();
        version.installPath = verObj["installPath"].toString();
        version.installDate = QDateTime::fromString(
            verObj["installDate"].toString(), 
            Qt::ISODate
        );
        version.dataPath = verObj["dataPath"].toString();
        
        // Validate installation
        version.isPlayable = validateInstallation(version);
        
        if (version.isPlayable) {
            m_installedVersions[version.versionInfo.version] = version;
        }
    }
}

void GameManager::saveVersionConfig() {
    QJsonObject versions;
    
    for (const auto& version : m_installedVersions) {
        QJsonObject verObj;
        verObj["versionString"] = version.versionInfo.versionString;
        verObj["versionCode"] = version.versionInfo.versionCode;
        verObj["isBeta"] = version.versionInfo.isBeta;
        verObj["installPath"] = version.installPath;
        verObj["installDate"] = version.installDate.toString(Qt::ISODate);
        verObj["dataPath"] = version.dataPath;
        
        versions[version.versionInfo.version.toString()] = verObj;
    }
    
    QFile configFile(m_baseInstallDir.filePath("versions.json"));
    if (configFile.open(QIODevice::WriteOnly)) {
        configFile.write(QJsonDocument(versions).toJson());
    }
}

QList<InstalledVersion> GameManager::getInstalledVersions() const {
    return m_installedVersions.values();
}

QFuture<bool> GameManager::installVersion(const MinecraftVersion& version) {
    return QtConcurrent::run([this, version]() {
        emit installProgress("Checking storage space...", 0);
        
        // Check available space
        QString installPath = getVersionInstallPath(version.version);
        if (getAvailableSpace(installPath) < version.size * 2) { // 2x for extraction
            emit installProgress("Insufficient storage space", 0);
            return false;
        }
        
        // Create version directory
        QDir versionDir(installPath);
        if (!versionDir.mkpath(".")) {
            emit installProgress("Failed to create installation directory", 0);
            return false;
        }
        
        // Download APK
        emit installProgress("Downloading...", 10);
        QString apkPath = versionDir.filePath("game.apk");
        
        connect(&m_playApi, &GooglePlayAPI::downloadProgress,
                this, [this](qint64 received, qint64 total) {
            int percent = (received * 80 / total) + 10;
            emit installProgress("Downloading...", percent);
        });
        
        QFuture<bool> downloadFuture = m_playApi.downloadVersion(
            version,
            apkPath
        );
        bool downloadSuccess = downloadFuture.result();
        
        if (!downloadSuccess) {
            versionDir.removeRecursively();
            emit installProgress("Download failed", 0);
            return false;
        }
        
        // Extract APK
        emit installProgress("Extracting...", 90);
        if (!extractApk(apkPath, versionDir.path())) {
            versionDir.removeRecursively();
            emit installProgress("Extraction failed", 0);
            return false;
        }
        
        // Setup version data
        InstalledVersion installed;
        installed.versionInfo = version;
        installed.installPath = installPath;
        installed.installDate = QDateTime::currentDateTime();
        installed.dataPath = getDefaultDataPath() + "/" + version.version.toString();
        
        if (!setupVersionData(installed)) {
            versionDir.removeRecursively();
            emit installProgress("Data setup failed", 0);
            return false;
        }
        
        // Validate installation
        installed.isPlayable = validateInstallation(installed);
        if (!installed.isPlayable) {
            versionDir.removeRecursively();
            emit installProgress("Validation failed", 0);
            return false;
        }
        
        // Save configuration
        m_installedVersions[version.version] = installed;
        saveVersionConfig();
        
        emit installProgress("Installation complete", 100);
        emit installComplete(version.version, true);
        return true;
    });
}

bool GameManager::extractApk(const QString& apkPath, const QString& extractPath) {
    // Use unzip command for APK extraction
    QProcess unzip;
    unzip.setProgram("unzip");
    unzip.setArguments({
        "-o",           // overwrite files without prompting
        "-q",           // quiet mode
        apkPath,        // source APK
        "-d",          // destination directory
        extractPath    // extract path
    });
    
    unzip.start();
    if (!unzip.waitForFinished(300000)) { // 5 minute timeout
        return false;
    }
    
    return unzip.exitCode() == 0;
}

bool GameManager::setupVersionData(const InstalledVersion& version) {
    QDir dataDir(version.dataPath);
    if (!dataDir.mkpath(".")) {
        return false;
    }
    
    // Create symbolic links for shared data
    QString sharedDataPath = getDefaultDataPath() + "/shared";
    QDir sharedDir(sharedDataPath);
    if (!sharedDir.exists()) {
        sharedDir.mkpath(".");
    }
    
    // Link shared resources
    QStringList sharedDirs = {"resource_packs", "behavior_packs", "worlds"};
    for (const QString& dir : sharedDirs) {
        QString target = sharedDataPath + "/" + dir;
        QString link = version.dataPath + "/" + dir;
        
        if (!QFile::exists(target)) {
            QDir().mkpath(target);
        }
        if (QFile::exists(link)) {
            QFile::remove(link);
        }
        QFile::link(target, link);
    }
    
    return true;
}

bool GameManager::launchVersion(const QVersionNumber& version) {
    if (!m_installedVersions.contains(version)) {
        return false;
    }
    
    const InstalledVersion& installed = m_installedVersions[version];
    if (!installed.isPlayable) {
        return false;
    }
    
    // Prepare launch environment
    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
    env.insert("MCPE_DATA_PATH", installed.dataPath);
    env.insert("MCPE_VERSION", installed.versionInfo.versionString);
    env.insert("DISPLAY", ":0");
    env.insert("WAYLAND_DISPLAY", "wayland-0");
    env.insert("XDG_RUNTIME_DIR", "/run/user/1000");
    
    // Launch process
    QProcess* gameProcess = new QProcess(this);
    gameProcess->setProcessEnvironment(env);
    gameProcess->setWorkingDirectory(installed.installPath);
    
    // Connect signals for monitoring
    connect(gameProcess, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            this, [this, version, gameProcess](int exitCode, QProcess::ExitStatus exitStatus) {
        if (exitCode != 0 || exitStatus != QProcess::NormalExit) {
            emit versionCrashed(version, gameProcess->errorString());
        }
        gameProcess->deleteLater();
    });
    
    // Start the game
    gameProcess->start("./minecraft-launcher", QStringList());
    
    if (gameProcess->waitForStarted()) {
        emit versionLaunched(version);
        return true;
    }
    
    gameProcess->deleteLater();
    return false;
}

QString GameManager::getDefaultDataPath() const {
    return m_baseInstallDir.filePath("data");
}

bool GameManager::setCustomDataPath(const QString& path) {
    QDir dir(path);
    if (!dir.exists() && !dir.mkpath(".")) {
        return false;
    }
    
    m_customDataPath = path;
    
    // Update all version configurations to use new path
    for (auto& version : m_installedVersions) {
        QString oldPath = version.dataPath;
        version.dataPath = m_customDataPath + "/" + 
                          version.versionInfo.version.toString();
        
        // Move existing data if necessary
        if (QDir(oldPath).exists()) {
            QDir().rename(oldPath, version.dataPath);
        }
    }
    
    saveVersionConfig();
    return true;
}

QString GameManager::getCurrentDataPath() const {
    return m_customDataPath.isEmpty() ? getDefaultDataPath() : m_customDataPath;
}

qint64 GameManager::getAvailableSpace(const QString& path) const {
    QStorageInfo storage(path);
    return storage.bytesAvailable();
}

bool GameManager::validateInstallation(const InstalledVersion& version) {
    QDir dir(version.installPath);
    
    // Check for required files
    QStringList requiredFiles = {
        "minecraft-launcher",
        "libminecraft.so",
        "libc++_shared.so"
    };
    
    for (const QString& file : requiredFiles) {
        if (!QFile::exists(dir.filePath(file))) {
            return false;
        }
    }
    
    // Ensure launcher is executable
    QFile launcher(dir.filePath("minecraft-launcher"));
    return launcher.permissions().testFlag(QFile::ExeUser);
}

bool GameManager::uninstallVersion(const QVersionNumber& version) {
    if (!m_installedVersions.contains(version)) {
        return false;
    }
    
    const InstalledVersion& installed = m_installedVersions[version];
    
    // Remove installation directory
    QDir installDir(installed.installPath);
    if (installDir.exists() && !installDir.removeRecursively()) {
        return false;
    }
    
    // Remove version-specific data (but keep shared data)
    QDir dataDir(installed.dataPath);
    if (dataDir.exists() && !dataDir.removeRecursively()) {
        return false;
    }
    
    // Update configuration
    m_installedVersions.remove(version);
    saveVersionConfig();
    
    return true;
}