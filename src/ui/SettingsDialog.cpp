#include "SettingsDialog.hpp"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFileDialog>
#include <QMessageBox>
#include <QSettings>
#include "../game/GameManager.hpp"
#include "../steam/SteamIntegration.hpp"

SettingsDialog::SettingsDialog(QWidget* parent)
    : QDialog(parent)
    , m_currentControl(0) {
    
    setupUI();
    setupGamepad();
    loadSettings();
    
    // Install event filter for keyboard navigation
    qApp->installEventFilter(this);
}

void SettingsDialog::setupUI() {
    setWindowTitle("Settings");
    setWindowFlags(Qt::Dialog | Qt::CustomizeWindowHint | Qt::WindowTitleHint);
    setModal(true);
    
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(20);
    mainLayout->setContentsMargins(20, 20, 20, 20);
    
    // Storage Location
    QHBoxLayout* storageLayout = new QHBoxLayout();
    QLabel* storageLabel = new QLabel("Storage Location:", this);
    m_storageLocation = new QComboBox(this);
    QPushButton* browseButton = new QPushButton("Browse", this);
    
    storageLayout->addWidget(storageLabel);
    storageLayout->addWidget(m_storageLocation, 1);
    storageLayout->addWidget(browseButton);
    
    mainLayout->addLayout(storageLayout);
    
    // Performance Settings
    QGroupBox* perfGroup = new QGroupBox("Performance", this);
    QVBoxLayout* perfLayout = new QVBoxLayout(perfGroup);
    
    // Performance Mode
    QHBoxLayout* modeLayout = new QHBoxLayout();
    QLabel* modeLabel = new QLabel("Performance Mode:", this);
    m_performanceMode = new QComboBox(this);
    m_performanceMode->addItems({"Silent", "Balanced", "Turbo", "Custom"});
    
    modeLayout->addWidget(modeLabel);
    modeLayout->addWidget(m_performanceMode, 1);
    perfLayout->addLayout(modeLayout);
    
    // Custom TDP
    QHBoxLayout* tdpLayout = new QHBoxLayout();
    QLabel* tdpLabel = new QLabel("Custom TDP (W):", this);
    m_customTDP = new QSpinBox(this);
    m_customTDP->setRange(5, 30);
    m_customTDP->setSingleStep(1);
    
    tdpLayout->addWidget(tdpLabel);
    tdpLayout->addWidget(m_customTDP, 1);
    perfLayout->addLayout(tdpLayout);
    
    mainLayout->addWidget(perfGroup);
    
    // Display Settings
    QGroupBox* displayGroup = new QGroupBox("Display", this);
    QVBoxLayout* displayLayout = new QVBoxLayout(displayGroup);
    
    // VRR
    m_vrrEnabled = new QCheckBox("Enable Variable Refresh Rate", this);
    displayLayout->addWidget(m_vrrEnabled);
    
    // Refresh Rate
    QHBoxLayout* refreshLayout = new QHBoxLayout();
    QLabel* refreshLabel = new QLabel("Refresh Rate:", this);
    m_refreshRate = new QComboBox(this);
    m_refreshRate->addItems({"60 Hz", "120 Hz"});
    
    refreshLayout->addWidget(refreshLabel);
    refreshLayout->addWidget(m_refreshRate, 1);
    displayLayout->addLayout(refreshLayout);
    
    mainLayout->addWidget(displayGroup);
    
    // Steam Integration
    m_steamIntegration = new QCheckBox("Enable Steam Integration", this);
    mainLayout->addWidget(m_steamIntegration);
    
    // Buttons
    QHBoxLayout* buttonLayout = new QHBoxLayout();
    m_saveButton = new QPushButton("Save", this);
    m_cancelButton = new QPushButton("Cancel", this);
    
    QString buttonStyle = 
        "QPushButton {"
        "   background-color: #4d4d4d;"
        "   color: white;"
        "   border: none;"
        "   border-radius: 5px;"
        "   padding: 15px 30px;"
        "   font-size: 16px;"
        "}"
        "QPushButton:hover {"
        "   background-color: #5d5d5d;"
        "}"
        "QPushButton:pressed {"
        "   background-color: #3d3d3d;"
        "}";
    
    m_saveButton->setStyleSheet(buttonStyle);
    m_cancelButton->setStyleSheet(buttonStyle);
    
    connect(m_saveButton, &QPushButton::clicked,
            this, &SettingsDialog::onSaveClicked);
    connect(m_cancelButton, &QPushButton::clicked,
            this, &QDialog::reject);
    
    buttonLayout->addWidget(m_saveButton);
    buttonLayout->addWidget(m_cancelButton);
    
    mainLayout->addLayout(buttonLayout);
    
    // Control hints
    m_controlHints = new QLabel(this);
    m_controlHints->setStyleSheet(
        "QLabel {"
        "   color: #888888;"
        "   font-size: 14px;"
        "}"
    );
    updateControlHints();
    mainLayout->addWidget(m_controlHints);
    
    // Build controls list for navigation
    m_controls = {
        m_storageLocation,
        m_performanceMode,
        m_customTDP,
        m_vrrEnabled,
        m_refreshRate,
        m_steamIntegration,
        m_saveButton,
        m_cancelButton
    };
    
    // Connect signals
    connect(browseButton, &QPushButton::clicked,
            this, &SettingsDialog::onStorageLocationChanged);
    connect(m_performanceMode, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &SettingsDialog::onPerformanceModeChanged);
    connect(m_customTDP, QOverload<int>::of(&QSpinBox::valueChanged),
            this, &SettingsDialog::onCustomTDPChanged);
    connect(m_vrrEnabled, &QCheckBox::toggled,
            this, &SettingsDialog::onVRRToggled);
    connect(m_refreshRate, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &SettingsDialog::onRefreshRateChanged);
}

void SettingsDialog::loadSettings() {
    QSettings settings;
    
    // Load storage locations
    m_storageLocation->clear();
    m_storageLocation->addItem(GameManager::instance()->getDefaultDataPath());
    QString customPath = settings.value("storage/customPath").toString();
    if (!customPath.isEmpty()) {
        m_storageLocation->addItem(customPath);
        m_storageLocation->setCurrentIndex(1);
    }
    
    // Load performance settings
    m_systemControl = AllySystemControl::instance();
    m_performanceMode->setCurrentIndex(
        static_cast<int>(m_systemControl->getCurrentMode()));
    m_customTDP->setValue(m_systemControl->getCurrentTDP());
    m_customTDP->setEnabled(
        m_systemControl->getCurrentMode() == 
        AllySystemControl::PerformanceMode::CUSTOM);
    
    // Load display settings
    m_vrrEnabled->setChecked(m_systemControl->isVRREnabled());
    m_vrrEnabled->setEnabled(m_systemControl->isVRRSupported());
    
    int refreshRate = m_systemControl->getCurrentRefreshRate();
    m_refreshRate->setCurrentText(QString::number(refreshRate) + " Hz");
    
    // Load Steam integration settings
    m_steamIntegration->setChecked(
        settings.value("steam/enabled", true).toBool());
}

void SettingsDialog::saveSettings() {
    QSettings settings;
    
    // Save storage location
    if (m_storageLocation->currentIndex() > 0) {
        QString path = m_storageLocation->currentText();
        settings.setValue("storage/customPath", path);
        GameManager::instance()->setCustomDataPath(path);
    } else {
        settings.remove("storage/customPath");
        GameManager::instance()->setCustomDataPath(QString());
    }
    
    // Save performance settings
    AllySystemControl::PerformanceMode mode = 
        static_cast<AllySystemControl::PerformanceMode>(
            m_performanceMode->currentIndex());
    m_systemControl->setPerformanceMode(mode);
    
    if (mode == AllySystemControl::PerformanceMode::CUSTOM) {
        m_systemControl->setCustomTDP(m_customTDP->value());
    }
    
    // Save display settings
    m_systemControl->setVRREnabled(m_vrrEnabled->isChecked());
    m_systemControl->setRefreshRate(
        m_refreshRate->currentText().split(" ")[0].toInt());
    
    // Save Steam integration settings
    settings.setValue("steam/enabled", m_steamIntegration->isChecked());
    
    accept();
}

void SettingsDialog::setupGamepad() {
    m_gamepad = AllyGamepad::instance();
    connect(m_gamepad, &AllyGamepad::buttonPressed,
            this, &SettingsDialog::onGamepadButton);
}

void SettingsDialog::onGamepadButton(AllyGamepad::Button button) {
    switch (button) {
        case AllyGamepad::Button::A:
            if (m_controls[m_currentControl] == m_saveButton) {
                onSaveClicked();
            } else if (m_controls[m_currentControl] == m_cancelButton) {
                reject();
            } else {
                m_controls[m_currentControl]->setFocus();
                if (auto checkbox = qobject_cast<QCheckBox*>(
                        m_controls[m_currentControl])) {
                    checkbox->setChecked(!checkbox->isChecked());
                }
            }
            break;
            
        case AllyGamepad::Button::B:
            reject();
            break;
            
        case AllyGamepad::Button::DPAD_UP:
            navigateControls(-1);
            break;
            
        case AllyGamepad::Button::DPAD_DOWN:
            navigateControls(1);
            break;
            
        case AllyGamepad::Button::DPAD_LEFT:
            if (auto spinBox = qobject_cast<QSpinBox*>(
                    m_controls[m_currentControl])) {
                spinBox->setValue(spinBox->value() - spinBox->singleStep());
            } else if (auto comboBox = qobject_cast<QComboBox*>(
                    m_controls[m_currentControl])) {
                int index = comboBox->currentIndex() - 1;
                if (index >= 0) {
                    comboBox->setCurrentIndex(index);
                }
            }
            break;
            
        case AllyGamepad::Button::DPAD_RIGHT:
            if (auto spinBox = qobject_cast<QSpinBox*>(
                    m_controls[m_currentControl])) {
                spinBox->setValue(spinBox->value() + spinBox->singleStep());
            } else if (auto comboBox = qobject_cast<QComboBox*>(
                    m_controls[m_currentControl])) {
                int index = comboBox->currentIndex() + 1;
                if (index < comboBox->count()) {
                    comboBox->setCurrentIndex(index);
                }
            }
            break;
            
        default:
            break;
    }
}

void SettingsDialog::navigateControls(int direction) {
    int newControl = m_currentControl + direction;
    
    if (newControl >= 0 && newControl < m_controls.size()) {
        m_currentControl = newControl;
        m_controls[m_currentControl]->setFocus();
        updateControlHints();
    }
}

void SettingsDialog::updateControlHints() {
    QString hints;
    if (m_gamepad->isConnected()) {
        hints = "A: Select   B: Cancel   ↑↓: Navigate   ←→: Adjust Values";
    } else {
        hints = "Enter: Select   Esc: Cancel   ↑↓: Navigate   ←→: Adjust Values";
    }
    m_controlHints->setText(hints);
}

void SettingsDialog::onStorageLocationChanged() {
    QString dir = QFileDialog::getExistingDirectory(this,
        "Select Storage Location",
        QDir::homePath(),
        QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
        
    if (!dir.isEmpty()) {
        if (m_storageLocation->count() > 1) {
            m_storageLocation->setItemText(1, dir);
        } else {
            m_storageLocation->addItem(dir);
        }
        m_storageLocation->setCurrentIndex(1);
    }
}

void SettingsDialog::onPerformanceModeChanged(int index) {
    m_customTDP->setEnabled(index == 3); // Custom mode
    if (index != 3) {
        m_systemControl->setPerformanceMode(
            static_cast<AllySystemControl::PerformanceMode>(index));
    }
}

void SettingsDialog::onCustomTDPChanged(int value) {
    if (m_performanceMode->currentIndex() == 3) {
        m_systemControl->setCustomTDP(value);
    }
}

void SettingsDialog::onVRRToggled(bool enabled) {
    if (m_systemControl->isVRRSupported()) {
        m_systemControl->setVRREnabled(enabled);
    }
}

void SettingsDialog::onRefreshRateChanged(int index) {
    int rate = m_refreshRate->currentText().split(" ")[0].toInt();
    m_systemControl->setRefreshRate(rate);
}

void SettingsDialog::onSaveClicked() {
    saveSettings();
}

bool SettingsDialog::eventFilter(QObject* obj, QEvent* event) {
    if (event->type() == QEvent::KeyPress) {
        QKeyEvent* keyEvent = static_cast<QKeyEvent*>(event);
        switch (keyEvent->key()) {
            case Qt::Key_Return:
            case Qt::Key_Enter:
                if (m_controls[m_currentControl] == m_saveButton) {
                    onSaveClicked();
                } else if (m_controls[m_currentControl] == m_cancelButton) {
                    reject();
                }
                return true;
                
            case Qt::Key_Escape:
                reject();
                return true;
                
            case Qt::Key_Up:
                navigateControls(-1);
                return true;
                
            case Qt::Key_Down:
                navigateControls(1);
                return true;
                
            case Qt::Key_Left:
            case Qt::Key_Right:
                if (auto spinBox = qobject_cast<QSpinBox*>(
                        m_controls[m_currentControl]
                if (auto spinBox = qobject_cast<QSpinBox*>(
                        m_controls[m_currentControl])) {
                    spinBox->setValue(spinBox->value() + 
                        (keyEvent->key() == Qt::Key_Left ? -1 : 1));
                } else if (auto comboBox = qobject_cast<QComboBox*>(
                        m_controls[m_currentControl])) {
                    int index = comboBox->currentIndex() + 
                        (keyEvent->key() == Qt::Key_Left ? -1 : 1);
                    if (index >= 0 && index < comboBox->count()) {
                        comboBox->setCurrentIndex(index);
                    }
                }
                return true;
        }
    }
    return QDialog::eventFilter(obj, event);
}
