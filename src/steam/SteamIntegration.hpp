#pragma once

#include <QObject>
#include <QString>

#ifdef STEAM_SUPPORT
#include <steam/steam_api.h>
class GameOverlayActivated_t;
#endif

class SteamIntegration : public QObject {
    Q_OBJECT

public:
    static SteamIntegration* instance();
    
    bool initialize();
    bool isSteamRunning() const { return m_steamRunning; }
    bool isBigPictureMode() const { return m_bigPictureMode; }
    bool isGameModeActive() const { return m_gameModeActive; }
    bool isOverlayEnabled() const { return m_overlayEnabled; }
    bool isCloudSyncEnabled() const { return m_cloudSyncEnabled; }

    bool loadSteamInputConfig(const QString& configPath);
    bool launchInBigPicture();
    bool setupGamemodeEnvironment();
    bool configureControllerLayout();

signals:
    void steamStatusChanged(bool running);
    void bigPictureModeChanged(bool enabled);
    void gameModeChanged(bool active);
    void overlayStatusChanged(bool enabled);
    void cloudSyncStatusChanged(bool enabled);
    void controllerConfigChanged();

private:
    explicit SteamIntegration(QObject* parent = nullptr);
    ~SteamIntegration();

    static SteamIntegration* s_instance;

    void detectSteamInstallation();
    
#ifdef STEAM_SUPPORT
    void STEAM_CALLBACK(onGameOverlayActivated, GameOverlayActivated_t*);
#endif

    bool m_steamRunning;
    bool m_bigPictureMode;
    bool m_gameModeActive;
    bool m_overlayEnabled;
    bool m_cloudSyncEnabled;
};