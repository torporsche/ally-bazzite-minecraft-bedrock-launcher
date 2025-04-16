#pragma once

#include <QMainWindow>
#include <QStackedWidget>
#include <QListWidget>
#include <QPushButton>
#include <QLabel>
#include "../gamepad/AllyGamepad.hpp"
#include "../steam/SteamIntegration.hpp"

class LauncherWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit LauncherWindow(QWidget* parent = nullptr);
    ~LauncherWindow();

protected:
    bool eventFilter(QObject* obj, QEvent* event) override;
    void showEvent(QShowEvent* event) override;
    void closeEvent(QCloseEvent* event) override;

private slots:
    void onGamepadButton(AllyGamepad::Button button);
    void onSteamStatusChanged(bool running);
    void onBigPictureModeChanged(bool active);
    void onGameModeChanged(bool active);
    void onVersionSelected(QListWidgetItem* item);
    void onInstallClicked();
    void onPlayClicked();
    void onSettingsClicked();

private:
    void setupUI();
    void setupGamepad();
    void updateUIScale();
    void updateVersionList();
    void updateControlHints();
    void navigateList(int direction);
    void toggleFullscreen();
    
    // UI Elements
    QStackedWidget* m_stackedWidget;
    QListWidget* m_versionList;
    QPushButton* m_playButton;
    QPushButton* m_installButton;
    QPushButton* m_settingsButton;
    QLabel* m_controlHints;
    
    // State
    bool m_isFullscreen;
    int m_uiScale;
    QString m_selectedVersion;
    
    // Steam integration
    SteamIntegration* m_steam;
    AllyGamepad* m_gamepad;
};