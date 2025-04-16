#pragma once

#include <QObject>
#include <QString>
#include <QProcess>
#include <memory>

class SteamIntegration : public QObject {
    Q_OBJECT

public:
    static SteamIntegration* instance();

    bool isSteamRunning() const;
    bool isBigPictureMode() const;
    bool isGameModeActive() const;
    
    // Steam integration methods
    bool launchInBigPicture(const QString& appId = QString());
    bool addNonSteamGame(const QString& execPath, const QString& name);
    bool removeNonSteamGame(const QString& appId);
    QString getLastActiveUser() const;

signals:
    void steamStatusChanged(bool running);
    void bigPictureModeChanged(bool active);
    void gameModeChanged(bool active);

private:
    explicit SteamIntegration(QObject* parent = nullptr);
    ~SteamIntegration();

    static SteamIntegration* s_instance;
    
    bool m_steamRunning;
    bool m_bigPictureMode;
    bool m_gameModeActive;
    QString m_steamPath;
    QString m_activeUser;
    
    void detectSteamInstallation();
    void monitorSteamProcess();
    void checkGameMode();
    QString findSteamConfig() const;
};