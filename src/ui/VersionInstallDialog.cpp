#include "VersionInstallDialog.hpp"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QMessageBox>
#include <QApplication>
#include <QStyle>
#include <QScreen>
#include "../game/GameManager.hpp"

VersionInstallDialog::VersionInstallDialog(QWidget* parent)
    : QDialog(parent)
    , m_installing(false) {
    
    setupUI();
    setupGamepad();
    
    // Initialize API
    m_playApi = new GooglePlayAPI(this);
    
    // Load available versions
    updateVersionList();
    
    // Install event filter for keyboard navigation
    qApp->installEventFilter(this);
}

void VersionInstallDialog::setupUI() {
    setWindowTitle("Install Minecraft Version");
    setWindowFlags(Qt::Dialog | Qt::CustomizeWindowHint | Qt::WindowTitleHint);
    setModal(true);
    
    // Set size based on screen
    QScreen* screen = QApplication::primaryScreen();
    QSize screenSize = screen->size();
    resize(screenSize.width() * 0.8, screenSize.height() * 0.8);
    
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(20);
    mainLayout->setContentsMargins(20, 20, 20, 20);
    
    // Storage information
    m_storageLabel = new QLabel(this);
    m_storageLabel->setStyleSheet(
        "QLabel {"
        "   color: white;"
        "   font-size: 14px;"
        "}"
    );
    mainLayout->addWidget(m_storageLabel);
    
    // Version list
    m_versionList = new QListWidget(this);
    m_versionList->setFocusPolicy(Qt::NoFocus);
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
            this, &VersionInstallDialog::onVersionSelected);
    
    mainLayout->addWidget(m_versionList, 1);
    
    // Progress information
    m_statusLabel = new QLabel(this);
    m_statusLabel->setStyleSheet(
        "QLabel {"
        "   color: white;"
        "   font-size: 14px;"
        "}"
    );
    mainLayout->addWidget(m_statusLabel);
    
    m_progressBar = new QProgressBar(this);
    m_progressBar->setStyleSheet(
        "QProgressBar {"
        "   border: 2px solid #2d2d2d;"
        "   border-radius: 5px;"
        "   text-align: center;"
        "}"
        "QProgressBar::chunk {"
        "   background-color: #4CAF50;"
        "   border-radius: 3px;"
        "}"
    );
    m_progressBar->hide();
    mainLayout->addWidget(m_progressBar);
    
    // Buttons
    QHBoxLayout* buttonLayout = new QHBoxLayout();
    buttonLayout->setSpacing(10);
    
    m_installButton = new QPushButton("Install", this);
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
    
    m_installButton->setStyleSheet(buttonStyle);
    m_cancelButton->setStyleSheet(buttonStyle);
    
    m_installButton->setEnabled(false);
    
    connect(m_installButton, &QPushButton::clicked,
            this, &VersionInstallDialog::onInstallClicked);
    connect(m_cancelButton, &QPushButton::clicked,
            this, &QDialog::reject);
    
    buttonLayout->addWidget(m_installButton);
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
    
    checkStorage();
}

void VersionInstallDialog::setupGamepad() {
    m_gamepad = AllyGamepad::instance();
    connect(m_gamepad, &AllyGamepad::buttonPressed,
            this, &VersionInstallDialog::onGamepadButton);
}

void VersionInstallDialog::updateVersionList() {
    m_versionList->clear();
    m_statusLabel->setText("Loading available versions...");
    m_installButton->setEnabled(false);
    
    // Get available versions asynchronously
    QFuture<QList<MinecraftVersion>> future = m_playApi->getAvailableVersions();
    QFutureWatcher<QList<MinecraftVersion>>* watcher = 
        new QFutureWatcher<QList<MinecraftVersion>>(this);
    
    connect(watcher, &QFutureWatcher<QList<MinecraftVersion>>::finished,
            this, [this, watcher]() {
        QList<MinecraftVersion> versions = watcher->result();
        
        for (const auto& version : versions) {
            QListWidgetItem* item = new QListWidgetItem(m_versionList);
            item->setText(QString("%1 (%2)")
                .arg(version.versionString)
                .arg(version.isBeta ? "Beta" : "Release"));
            item->setData(Qt::UserRole, QVariant::fromValue(version));
        }
        
        m_statusLabel->setText("Select a version to install");
        watcher->deleteLater();
    });
    
    watcher->setFuture(future);
}

void VersionInstallDialog::checkStorage() {
    QString dataPath = GameManager::instance()->getCurrentDataPath();
    QStorageInfo storage(dataPath);
    
    qint64 freeSpace = storage.bytesAvailable();
    QString freeSpaceStr = QString::number(freeSpace / 1024 / 1024 / 1024) + " GB";
    
    m_storageLabel->setText(QString("Available storage: %1").arg(freeSpaceStr));
}

void VersionInstallDialog::onGamepadButton(AllyGamepad::Button button) {
    if (m_installing) return;
    
    switch (button) {
        case AllyGamepad::Button::A:
            if (m_versionList->currentItem()) {
                onInstallClicked();
            }
            break;
            
        case AllyGamepad::Button::B:
            reject();
            break;
            
        case AllyGamepad::Button::DPAD_UP:
            navigateList(-1);
            break;
            
        case AllyGamepad::Button::DPAD_DOWN:
            navigateList(1);
            break;
            
        default:
            break;
    }
}

void VersionInstallDialog::navigateList(int direction) {
    int row = m_versionList->currentRow();
    int newRow = row + direction;
    
    if (newRow >= 0 && newRow < m_versionList->count()) {
        m_versionList->setCurrentRow(newRow);
        onVersionSelected(m_versionList->currentItem());
    }
}

void VersionInstallDialog::onVersionSelected(QListWidgetItem* item) {
    if (!item) return;
    
    MinecraftVersion version = item->data(Qt::UserRole)
        .value<MinecraftVersion>();
    
    // Check if version is already installed
    QList<InstalledVersion> installed = 
        GameManager::instance()->getInstalledVersions();
    
    bool isInstalled = false;
    for (const auto& inst : installed) {
        if (inst.versionInfo.version == version.version) {
            isInstalled = true;
            break;
        }
    }
    
    m_installButton->setEnabled(!isInstalled);
    m_selectedVersion = version.versionString;
    
    // Show version details
    QString details = QString("%1 (%2)\nSize: %3 MB")
        .arg(version.versionString)
        .arg(version.isBeta ? "Beta" : "Release")
        .arg(version.size / 1024 / 1024);
    
    m_statusLabel->setText(details);
}

void VersionInstallDialog::onInstallClicked() {
    if (m_selectedVersion.isEmpty() || m_installing) return;
    
    QListWidgetItem* item = m_versionList->currentItem();
    if (!item) return;
    
    MinecraftVersion version = item->data(Qt::UserRole)
        .value<MinecraftVersion>();
    
    // Check storage space
    if (GameManager::instance()->getAvailableSpace(
            GameManager::instance()->getCurrentDataPath()) < 
        version.size * 2) {
        QMessageBox::warning(this, "Insufficient Storage",
            "Not enough storage space available for installation.");
        return;
    }
    
    m_installing = true;
    m_progressBar->show();
    m_progressBar->setValue(0);
    m_installButton->setEnabled(false);
    m_versionList->setEnabled(false);
    
    // Start installation
    QFuture<bool> future = GameManager::instance()->installVersion(version);
    QFutureWatcher<bool>* watcher = new QFutureWatcher<bool>(this);
    
    connect(watcher, &QFutureWatcher<bool>::finished,
            this, [this, version, watcher]() {
        bool success = watcher->result();
        onInstallComplete(version.version, success);
        watcher->deleteLater();
    });
    
    watcher->setFuture(future);
}

void VersionInstallDialog::onInstallProgress(
    const QString& status, 
    int percent
) {
    m_statusLabel->setText(status);
    m_progressBar->setValue(percent);
}

void VersionInstallDialog::onInstallComplete(
    const QVersionNumber& version,
    bool success
) {
    m_installing = false;
    m_progressBar->hide();
    m_versionList->setEnabled(true);
    
    if (success) {
        accept();
    } else {
        m_statusLabel->setText("Installation failed");
        m_installButton->setEnabled(true);
    }
}

void VersionInstallDialog::updateControlHints() {
    QString hints;
    if (m_gamepad->isConnected()) {
        hints = "A: Install   B: Cancel   ↑↓: Navigate";
    } else {
        hints = "Enter: Install   Esc: Cancel   ↑↓: Navigate";
    }
    m_controlHints->setText(hints);
}

bool VersionInstallDialog::eventFilter(QObject* obj, QEvent* event) {
    if (event->type() == QEvent::KeyPress) {
        QKeyEvent* keyEvent = static_cast<QKeyEvent*>(event);
        switch (keyEvent->key()) {
            case Qt::Key_Return:
            case Qt::Key_Enter:
                if (m_installButton->isEnabled()) {
                    onInstallClicked();
                }
                return true;
                
            case Qt::Key_Escape:
                reject();
                return true;
                
            case Qt::Key_Up:
                navigateList(-1);
                return true;
                
            case Qt::Key_Down:
                navigateList(1);
                return true;
        }
    }
    return QDialog::eventFilter(obj, event);
}