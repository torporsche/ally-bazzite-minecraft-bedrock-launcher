#pragma once

#include <QObject>
#include <QHash>
#include <QVersionNumber>
#include <QFuture>
#include <QDir>
#include "../auth/GooglePlayAPI.hpp"

struct InstalledVersion {
    MinecraftVersion versionInfo;
    QString installPath;
    QDateTime installDate;
    QString dataPath;
    bool isPlayable;
};

class GameManager : public QObject {
    Q_OBJECT

public:
    explicit GameManager(QObject* parent = nullptr);
    
    // Version management
    QList<InstalledVersion> getInstalledVersions() const;
    QFuture<bool> installVersion(const MinecraftVersion& version);
    bool uninstallVersion(const QVersionNumber& version);
    bool launchVersion(const QVersionNumber& version);
    
    // Game data management
    QString getDefaultDataPath() const;
    bool setCustomDataPath(const QString& path);
    QString getCurrentDataPath() const;
    qint64 getRequiredSpace(const MinecraftVersion& version) const;
    qint64 getAvailableSpace(const QString& path) const;

signals:
    void installProgress(const QString& status, int percent);
    void installComplete(const QVersionNumber& version, bool success);
    void versionLaunched(const QVersionNumber& version);
    void versionCrashed(const QVersionNumber& version, const QString& error);
    void storageSpaceChanged(qint64 available);

private:
    QHash<QVersionNumber, InstalledVersion> m_installedVersions;
    GooglePlayAPI m_playApi;
    QString m_customDataPath;
    QDir m_baseInstallDir;
    
    void loadInstalledVersions();
    bool extractApk(const QString& apkPath, const QString& extractPath);
    bool setupVersionData(const InstalledVersion& version);
    void saveVersionConfig();
    QString getVersionInstallPath(const QVersionNumber& version) const;
    bool validateInstallation(const InstalledVersion& version);
};