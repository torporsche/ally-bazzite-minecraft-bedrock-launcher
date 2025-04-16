#include "LauncherWindow.hpp"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QScreen>
#include <QApplication>
#include <QStyle>
#include <QStyleOption>
#include <QPainter>
#include <QSettings>
#include <QMessageBox>

LauncherWindow::LauncherWindow(QWidget* parent)
    : QMainWindow(parent)
    , m_isFullscreen(false)
    , m_uiScale(100) {
    
    setupUI();
    setupGamepad();
    
    // Connect to Steam integration
    m_steam = SteamIntegration::instance();
    connect(m_steam, &SteamIntegration::steamStatusChanged,
            this, &LauncherWindow::onSteamStatusChanged);
    connect(m_steam, &SteamIntegration::bigPictureModeChanged,
            this, &LauncherWindow::onBigPictureModeChanged);
    connect(m_steam, &SteamIntegration::gameModeChanged,
            this, &LauncherWindow::onGameModeChanged);
    
    // Load settings
    QSettings settings;
    m_uiScale = settings.value("ui/scale", 100).toInt();
    updateUIScale();
    
    // Install event filter for keyboard navigation
    qApp->installEventFilter(this);
}

LauncherWindow::~LauncherWindow() {
    QSettings settings;
    settings.setValue("ui/scale", m_uiScale);
}

void LauncherWindow::setupUI() {
    // Set window properties
    setWindowTitle("Minecraft Bedrock Launcher");
    setMinimumSize(800, 600);
    
    // Create central widget
    QWidget* centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);
    
    QVBoxLayout* mainLayout = new QVBoxLayout(centralWidget);
    mainLayout->setSpacing(20);
    mainLayout->setContentsMargins(20, 20, 20, 20);
    
    // Create version list
    m_versionList = new QListWidget(this);
    m_versionList->setFocusPolicy(Qt::NoFocus);
    m_versionList->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_versionList->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_versionList->setStyleSheet(
        "QListWidget {"
        "   background-color: #2d2d2d;"
        "   border-radius: 10px;"
        "   padding: 10px;"
        "}"
        "QListWidget::item {"
        "   height: 60px;"
        "   color: white;"
        "   background-color: #3d3d3d;"
        "   border-radius: 5px;"
        "   margin: 5px;"
        "   padding: 10px;"
        "}"
        "QListWidget::item:selected {"
        "   background-color: #5d5d5d;"
        "}"
    );
    
    connect(m_versionList, &QListWidget::itemClicked,
            this, &LauncherWindow::onVersionSelected);
    
    mainLayout->addWidget(m_versionList, 1);
    
    // Create button container
    QHBoxLayout* buttonLayout = new QHBoxLayout();
    buttonLayout->setSpacing(10);
    
    // Create buttons
    m_playButton = new QPushButton("Play", this);
    m_installButton = new QPushButton("Install", this);
    m_settingsButton = new QPushButton("Settings", this);
    
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
    
    m_playButton->setStyleSheet(buttonStyle);
    m_installButton->setStyleSheet(buttonStyle);
    m_settingsButton->setStyleSheet(buttonStyle);
    
    connect(m_playButton, &QPushButton::clicked,
            this, &LauncherWindow::onPlayClicked);
    connect(m_installButton, &QPushButton::clicked,
            this, &LauncherWindow::onInstallClicked);
    connect(m_settingsButton, &QPushButton::clicked,
            this, &LauncherWindow::onSettingsClicked);
    
    buttonLayout->addWidget(m_playButton);
    buttonLayout->addWidget(m_installButton);
    buttonLayout->addWidget(m_settingsButton);
    
    mainLayout->addLayout(buttonLayout);
    
    // Create control hints
    m_controlHints = new QLabel(this);
    m_controlHints->setStyleSheet(
        "QLabel {"
        "   color: #888888;"
        "   font-size: 14px;"
        "}"
    );
    updateControlHints();
    
    mainLayout->addWidget(m_controlHints);
    
    // Update version list
    updateVersionList();
}

void LauncherWindow::setupGamepad() {
    m_gamepad = AllyGamepad::instance();
    connect(m_gamepad, &AllyGamepad::buttonPressed,
            this, &LauncherWindow::onGamepadButton);
}

void LauncherWindow::updateUIScale() {
    // Calculate base font size based on screen DPI and scale
    int baseFontSize = (QApplication::desktop()->logicalDpiY() * m_uiScale) / 96;
    
    // Update font sizes
    QFont font = QApplication::font();
    font.setPointSize(baseFontSize);
    QApplication::setFont(font);
    
    // Update control hints font size
    QFont hintFont = m_controlHints->font();
    hintFont.setPointSize(baseFontSize - 2);
    m_controlHints->setFont(hintFont);
    
    // Update list item sizes
    int itemHeight = baseFontSize * 3;
    QString listStyle = m_versionList->styleSheet();
    listStyle.replace(QRegExp("height:\\s*\\d+px"),
                     QString("height: %1px").arg(itemHeight));
    m_versionList->setStyleSheet(listStyle);
}

void LauncherWindow::updateControlHints() {
    QString hints;
    if (m_gamepad->isConnected()) {
        hints = "A: Select   B: Back   Y: Settings   "
               "↑↓: Navigate   L1/R1: Change Scale";
    } else {
        hints = "Enter: Select   Esc: Back   S: Settings   "
               "↑↓: Navigate   +/-: Change Scale";
    }
    m_controlHints->setText(hints);
}

void LauncherWindow::onGamepadButton(AllyGamepad::Button button) {
    switch (button) {
        case AllyGamepad::Button::A:
            if (m_versionList->hasFocus()) {
                onVersionSelected(m_versionList->currentItem());
            }
            break;
            
        case AllyGamepad::Button::B:
            if (m_isFullscreen) {
                toggleFullscreen();
            }
            break;
            
        case AllyGamepad::Button::Y:
            onSettingsClicked();
            break;
            
        case AllyGamepad::Button::DPAD_UP:
            navigateList(-1);
            break;
            
        case AllyGamepad::Button::DPAD_DOWN:
            navigateList(1);
            break;
            
        case AllyGamepad::Button::L1:
            m_uiScale = qMax(50, m_uiScale - 10);
            updateUIScale();
            break;
            
        case AllyGamepad::Button::R1:
            m_uiScale = qMin(200, m_uiScale + 10);
            updateUIScale();
            break;
            
        default:
            break;
    }
}

void LauncherWindow::navigateList(int direction) {
    int row = m_versionList->currentRow();
    int newRow = row + direction;
    
    if (newRow >= 0 && newRow < m_versionList->count()) {
        m_versionList->setCurrentRow(newRow);
    }
}

void LauncherWindow::toggleFullscreen() {
    if (m_isFullscreen) {
        showNormal();
    } else {
        showFullScreen();
    }
    m_isFullscreen = !m_isFullscreen;
}

void LauncherWindow::onSteamStatusChanged(bool running) {
    if (!running && m_steam->isGameModeActive()) {
        // If Steam closes while in game mode, exit
        QApplication::quit();
    }
}

void LauncherWindow::onBigPictureModeChanged(bool active) {
    if (active && !m_isFullscreen) {
        toggleFullscreen();
    }
}

void LauncherWindow::onGameModeChanged(bool active) {
    if (active) {
        // Ensure proper scaling in game mode
        m_uiScale = 150; // Larger scale for TV mode
        updateUIScale();
        if (!m_isFullscreen) {
            toggleFullscreen();
        }
    }
}

bool LauncherWindow::eventFilter(QObject* obj, QEvent* event) {
    if (event->type() == QEvent::KeyPress) {
        QKeyEvent* keyEvent = static_cast<QKeyEvent*>(event);
        switch (keyEvent->key()) {
            case Qt::Key_Return:
            case Qt::Key_Enter:
                if (m_versionList->hasFocus()) {
                    onVersionSelected(m_versionList->currentItem());
                }
                return true;
                
            case Qt::Key_Escape:
                if (m_isFullscreen) {
                    toggleFullscreen();
                }
                return true;
                
            case Qt::Key_S:
                onSettingsClicked();
                return true;
                
            case Qt::Key_Plus:
                m_uiScale = qMin(200, m_uiScale + 10);
                updateUIScale();
                return true;
                
            case Qt::Key_Minus:
                m_uiScale = qMax(50, m_uiScale - 10);
                updateUIScale();
                return true;
        }
    }
    return QMainWindow::eventFilter(obj, event);
}

void LauncherWindow::showEvent(QShowEvent* event) {
    QMainWindow::showEvent(event);
    
    // Check if we should start in full screen
    if (m_steam->isBigPictureMode() || m_steam->isGameModeActive()) {
        toggleFullscreen();
    }
}

void LauncherWindow::updateVersionList() {
    m_versionList->clear();
    
    // Add installed versions
    QList<InstalledVersion> versions = GameManager::instance()->getInstalledVersions();
    for (const auto& version : versions) {
        QListWidgetItem* item = new QListWidgetItem(m_versionList);
        item->setText(version.versionInfo.versionString);
        item->setData(Qt::UserRole, version.versionInfo.version.toString());
        
        if (!version.isPlayable) {
            item->setFlags(item->flags() & ~Qt::ItemIsEnabled);
        }
    }
}

void LauncherWindow::onVersionSelected(QListWidgetItem* item) {
    if (!item) return;
    
    m_selectedVersion = item->data(Qt::UserRole).toString();
    m_playButton->setEnabled(true);
}

void LauncherWindow::onPlayClicked() {
    if (m_selectedVersion.isEmpty()) return;
    
    QVersionNumber version = QVersionNumber::fromString(m_selectedVersion);
    
    // Launch through Steam if in Big Picture Mode
    if (m_steam->isBigPictureMode() || m_steam->isGameModeActive()) {
        // Get the Steam App ID for the selected version
        QString appId = "minecraft_" + m_selectedVersion;
        m_steam->launchInBigPicture(appId);
    } else {
        // Launch directly
        GameManager::instance()->launchVersion(version);
    }
}

void LauncherWindow::onInstallClicked() {
    // Show available versions dialog
    // Implementation will be added in the next part
}

void LauncherWindow::onSettingsClicked() {
    // Show settings dialog
    // Implementation will be added in the next part
}