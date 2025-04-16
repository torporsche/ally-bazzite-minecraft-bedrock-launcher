#pragma once

#include <QDialog>
#include <QComboBox>
#include <QCheckBox>
#include <QSpinBox>
#include <QLabel>
#include <QPushButton>
#include "../gamepad/AllyGamepad.hpp"
#include "../gamepad/AllySystemControl.hpp"

class SettingsDialog : public QDialog {
    Q_OBJECT

public:
    explicit SettingsDialog(QWidget* parent = nullptr);
    ~SettingsDialog();

protected:
    bool eventFilter(QObject* obj, QEvent* event) override;

private slots:
    void onGamepadButton(AllyGamepad::Button button);
    void onStorageLocationChanged();
    void onPerformanceModeChanged(int index);
    void onCustomTDPChanged(int value);
    void onVRRToggled(bool enabled);
    void onRefreshRateChanged(int index);
    void onSaveClicked();

private:
    void setupUI();
    void setupGamepad();
    void loadSettings();
    void saveSettings();
    void updateControlHints();
    void navigateControls(int direction);
    
    QComboBox* m_storageLocation;
    QComboBox* m_performanceMode;
    QSpinBox* m_customTDP;
    QCheckBox* m_vrrEnabled;
    QComboBox* m_refreshRate;
    QCheckBox* m_steamIntegration;
    QLabel* m_controlHints;
    QPushButton* m_saveButton;
    QPushButton* m_cancelButton;
    
    AllyGamepad* m_gamepad;
    AllySystemControl* m_systemControl;
    
    int m_currentControl;
    QList<QWidget*> m_controls;
};