#pragma once

#include <QMainWindow>
#include <QGamepad>
#include <memory>
#include "../auth/GooglePlayAuth.hpp"
#include "../game/GameManager.hpp"

class LauncherWindow : public QMainWindow {
    Q_OBJECT
    
public:
    explicit LauncherWindow(QWidget *parent = nullptr);
    ~LauncherWindow() override;

private slots:
    void onLoginRequired();
    void onVersionSelected(const QString& version);
    void onGamepadButtonPressed(int button);
    void onStorageLocationChanged(const QString& path);
    
private:
    void setupUI();
    void setupGamepad();
    void loadInstalledVersions();
    void checkStorageLocation();
    
    std::unique_ptr<GooglePlayAuth> m_auth;
    std::unique_ptr<GameManager> m_gameManager;
    QGamepad* m_gamepad;
    
    // UI elements will be added here
};