#include "AllySystemControl.hpp"
#include <QFile>
#include <QDir>
#include <QDebug>

AllySystemControl* AllySystemControl::s_instance = nullptr;

AllySystemControl* AllySystemControl::instance() {
    if (!s_instance) {
        s_instance = new AllySystemControl();
    }
    return s_instance;
}

AllySystemControl::AllySystemControl(QObject* parent)
    : QObject(parent)
    , m_vrrSupported(false)
    , m_vrrEnabled(false)
    , m_currentRefreshRate(60)
    , m_currentMode(PerformanceMode::BALANCED)
    , m_currentTDP(15)
    , m_currentFanSpeed(50)
    , m_vrrSysfsPath("/sys/class/drm/card0/device/vrr_enabled")
    , m_perfModePath("/sys/devices/platform/asus-nb-wmi/performance_mode")
    , m_tdpPath("/sys/devices/platform/asus-nb-wmi/power_boost")
    , m_fanPath("/sys/devices/platform/asus-nb-wmi/fan_boost_mode") {

    initializeVRRControl();
    initializePerformanceControl();
}

AllySystemControl::~AllySystemControl() {
}

bool AllySystemControl::initializeVRRControl() {
    // Check if VRR is supported
    if (QFile::exists(m_vrrSysfsPath)) {
        m_vrrSupported = true;
        m_vrrEnabled = (readFromSysfs(m_vrrSysfsPath) == "1");
        return true;
    }
    return false;
}

bool AllySystemControl::initializePerformanceControl() {
    if (!QFile::exists(m_perfModePath)) {
        qWarning() << "Performance control not available";
        return false;
    }

    // Read current performance mode
    QString mode = readFromSysfs(m_perfModePath);
    if (mode == "0") m_currentMode = PerformanceMode::SILENT;
    else if (mode == "1") m_currentMode = PerformanceMode::BALANCED;
    else if (mode == "2") m_currentMode = PerformanceMode::TURBO;
    
    // Read current TDP and fan speed
    m_currentTDP = readFromSysfs(m_tdpPath).toInt();
    m_currentFanSpeed = readFromSysfs(m_fanPath).toInt();
    
    return true;
}

bool AllySystemControl::writeToSysfs(const QString& path, const QString& value) {
    QFile file(path);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        qWarning() << "Failed to open" << path << "for writing";
        return false;
    }
    
    if (file.write(value.toUtf8()) == -1) {
        qWarning() << "Failed to write to" << path;
        return false;
    }
    
    return true;
}

QString AllySystemControl::readFromSysfs(const QString& path) {
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << "Failed to open" << path << "for reading";
        return QString();
    }
    
    return QString::fromUtf8(file.readAll()).trimmed();
}

bool AllySystemControl::isVRRSupported() const {
    return m_vrrSupported;
}

bool AllySystemControl::isVRREnabled() const {
    return m_vrrEnabled;
}

bool AllySystemControl::setVRREnabled(bool enabled) {
    if (!m_vrrSupported) return false;
    
    if (writeToSysfs(m_vrrSysfsPath, enabled ? "1" : "0")) {
        m_vrrEnabled = enabled;
        emit vrrStatusChanged(enabled);
        return true;
    }
    return false;
}

int AllySystemControl::getCurrentRefreshRate() const {
    return m_currentRefreshRate;
}

QList<int> AllySystemControl::getSupportedRefreshRates() const {
    // ROG Ally supported refresh rates
    return {60, 120};
}

bool AllySystemControl::setRefreshRate(int rate) {
    if (!getSupportedRefreshRates().contains(rate)) {
        return false;
    }

    // Set refresh rate using DRM interface
    QString cmd = QString("xrandr --output eDP --rate %1").arg(rate);
    if (system(cmd.toUtf8().constData()) == 0) {
        m_currentRefreshRate = rate;
        emit refreshRateChanged(rate);
        return true;
    }
    return false;
}

AllySystemControl::PerformanceMode AllySystemControl::getCurrentMode() const {
    return m_currentMode;
}

bool AllySystemControl::setPerformanceMode(PerformanceMode mode) {
    QString value;
    switch (mode) {
        case PerformanceMode::SILENT:
            value = "0";
            break;
        case PerformanceMode::BALANCED:
            value = "1";
            break;
        case PerformanceMode::TURBO:
            value = "2";
            break;
        case PerformanceMode::CUSTOM:
            // Custom mode uses current TDP and fan settings
            return true;
    }
    
    if (writeToSysfs(m_perfModePath, value)) {
        m_currentMode = mode;
        emit performanceModeChanged(mode);
        return true;
    }
    return false;
}

bool AllySystemControl::setCustomTDP(int watts) {
    if (watts < 5 || watts > 30) { // Safe TDP range for ROG Ally
        return false;
    }
    
    if (writeToSysfs(m_tdpPath, QString::number(watts))) {
        m_currentTDP = watts;
        emit tdpChanged(watts);
        return true;
    }
    return false;
}

bool AllySystemControl::setCustomFanSpeed(int percentage) {
    if (percentage < 0 || percentage > 100) {
        return false;
    }
    
    if (writeToSysfs(m_fanPath, QString::number(percentage))) {
        m_currentFanSpeed = percentage;
        emit fanSpeedChanged(percentage);
        return true;
    }
    return false;
}

int AllySystemControl::getCurrentTDP() const {
    return m_currentTDP;
}

int AllySystemControl::getCurrentFanSpeed() const {
    return m_currentFanSpeed;
}