#pragma once

#include <QObject>
#include <QString>
#include <QMap>
#include "../game/GameManager.hpp"

class SteamBPMIntegration : public QObject {
    Q_OBJECT

public:
    static SteamBPMIntegration* instance();

    // Steam Big Picture Mode integration
    bool registerWithSteam(const InstalledVersion& version);
    bool unregisterFromSteam(const InstalledVersion& version);
    bool updateSteamGridArt(const InstalledVersion& version);
    bool configureSteamInput(const InstalledVersion& version);
    
    // Steam Overlay features
    bool enableOverlay(const InstalledVersion& version);
    bool disableOverlay(const InstalledVersion& version);
    
    // Steam Cloud integration
    bool enableCloudSync(const InstalledVersion& version);
    bool disableCloudSync(const InstalledVersion& version);
    
    // Steam shortcuts management
    QString getSteamShortcutId(const QVersionNumber& version) const;
    bool updateSteamShortcut(const InstalledVersion& version);

signals:
    void registrationComplete(const QVersionNumber& version, bool success);
    void overlayStatusChanged(const QVersionNumber& version, bool enabled);
    void cloudSyncStatusChanged(const QVersionNumber& version, bool enabled);

private:
    explicit SteamBPMIntegration(QObject* parent = nullptr);
    ~SteamBPMIntegration();

    static SteamBPMIntegration* s_instance;
    
    QString m_steamUserId;
    QString m_steamConfigPath;
    QMap<QVersionNumber, QString> m_shortcutIds;
    
    bool createSteamShortcut(const InstalledVersion& version);
    bool modifySteamShortcut(const QString& shortcutId, 
                            const QMap<QString, QString>& properties);
    QString generateShortcutId(const InstalledVersion& version);
    bool setupSteamInputConfig(const QString& shortcutId);
    bool setupSteamGridArt(const QString& shortcutId);
    QString findSteamUserConfig() const;
};