#pragma once

#include <QDialog>
#include <QListWidget>
#include <QProgressBar>
#include <QLabel>
#include <QPushButton>
#include "../auth/GooglePlayAPI.hpp"
#include "../gamepad/AllyGamepad.hpp"

class VersionInstallDialog : public QDialog {
    Q_OBJECT

public:
    explicit VersionInstallDialog(QWidget* parent = nullptr);
    ~VersionInstallDialog();

protected:
    bool eventFilter(QObject* obj, QEvent* event) override;
    void showEvent(QShowEvent* event) override;

private slots:
    void onGamepadButton(AllyGamepad::Button button);
    void onVersionSelected(QListWidgetItem* item);
    void onInstallClicked();
    void onInstallProgress(const QString& status, int percent);
    void onInstallComplete(const QVersionNumber& version, bool success);
    void onStorageLocationChanged(const QString& path);

private:
    void setupUI();
    void setupGamepad();
    void updateVersionList();
    void updateControlHints();
    void navigateList(int direction);
    void checkStorage();
    
    QListWidget* m_versionList;
    QProgressBar* m_progressBar;
    QLabel* m_statusLabel;
    QLabel* m_storageLabel;
    QLabel* m_controlHints;
    QPushButton* m_installButton;
    QPushButton* m_cancelButton;
    
    AllyGamepad* m_gamepad;
    GooglePlayAPI* m_playApi;
    QString m_selectedVersion;
    bool m_installing;
};