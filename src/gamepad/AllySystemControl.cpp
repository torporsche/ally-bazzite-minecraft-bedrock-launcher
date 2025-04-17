#include "AllySystemControl.hpp"
#include <QFile>
#include <QDebug>
#include <QProcess>

// System file paths for ROG Ally
const QString AllySystemControl::POWER_PROFILE_PATH = "/sys/devices/platform/asus-nb-wmi/profile";
const QString AllySystemControl::TDP_PATH = "/sys/class/powercap/powercap0/tdp";
const QString AllySystemControl::GPU_FREQ_PATH = "/sys/class/drm/card0/device/pp_dpm_sclk";
const QString AllySystemControl::TEMP_PATH = "/sys/class/hwmon/hwmon*/temp1_input";
const QString AllySystemControl::FAN_PATH = "/sys/devices/platform/asus-nb-wmi/fan_speed";
const QString AllySystemControl::BATTERY_PATH = "/sys/class/power_supply/BAT1/";

AllySystemControl* AllySystemControl::s_instance = nullptr;

AllySystemControl* AllySystemControl::instance() {
    if (!s_instance) {
        s_instance = new AllySystemControl();
    }
    return s_instance;
}

AllySystemControl::AllySystemControl(QObject* parent)
    : QObject(parent)
    , m_currentProfile(PerformanceProfile::BALANCED)
    , m_currentTDP(15)
    , m_currentGPUFreq(1600)
    , m_freeSyncEnabled(true)
    , m_currentTemp(0.0f)
    , m_batteryLevel(100)
    , m_isCharging(false)
    , m_fanSpeed(0) {
    
    // Set up monitoring timer
    connect(&m_monitorTimer, &QTimer::timeout, this, [this]() {
        monitorTemperature();
        monitorBattery();
        adjustFanCurve();
    });
    m_monitorTimer.start(2000); // Check every 2 seconds
}

bool AllySystemControl::setPerformanceProfile(PerformanceProfile profile) {
    QString profileValue;
    switch (profile) {
        case PerformanceProfile::SILENT:
            profileValue = "0";
            setTDP(10);  // Lower TDP for battery savings
            break;
        case PerformanceProfile::BALANCED:
            profileValue = "1";
            setTDP(15);  // Default TDP
            break;
        case PerformanceProfile::TURBO:
            profileValue = "2";
            setTDP(25);  // Higher TDP for maximum performance
            break;
        case PerformanceProfile::MANUAL:
            profileValue = "3";
            // Keep current TDP
            break;
    }

    if (writeToSysfs(POWER_PROFILE_PATH, profileValue)) {
        m_currentProfile = profile;
        emit performanceProfileChanged(profile);
        return true;
    }
    return false;
}

bool AllySystemControl::setTDP(int watts) {
    if (watts < 5 || watts > 30) {
        qWarning() << "TDP value out of range (5-30W):" << watts;
        return false;
    }

    if (writeToSysfs(TDP_PATH, QString::number(watts * 1000000))) {
        m_currentTDP = watts;
        emit tdpChanged(watts);
        return true;
    }
    return false;
}

bool AllySystemControl::setGPUFreq(int mhz) {
    QString freqStr = QString::number(mhz);
    if (writeToSysfs(GPU_FREQ_PATH, freqStr)) {
        m_currentGPUFreq = mhz;
        emit gpuFreqChanged(mhz);
        return true;
    }
    return false;
}

bool AllySystemControl::enableFreeSync(bool enabled) {
    QProcess process;
    process.start("gamescope", QStringList() 
        << "--force-adaptive-sync" 
        << (enabled ? "1" : "0"));
    
    if (process.waitForFinished()) {
        m_freeSyncEnabled = enabled;
        emit freeSyncStatusChanged(enabled);
        return true;
    }
    return false;
}

void AllySystemControl::monitorTemperature() {
    QString temp = readFromSysfs(TEMP_PATH);
    if (!temp.isEmpty()) {
        float tempValue = temp.toFloat() / 1000.0f; // Convert from millidegrees to degrees
        if (tempValue != m_currentTemp) {
            m_currentTemp = tempValue;
            emit temperatureChanged(tempValue);
        }
    }
}

void AllySystemControl::monitorBattery() {
    QString capacityStr = readFromSysfs(BATTERY_PATH + "capacity");
    QString statusStr = readFromSysfs(BATTERY_PATH + "status");
    
    if (!capacityStr.isEmpty()) {
        int level = capacityStr.toInt();
        if (level != m_batteryLevel) {
            m_batteryLevel = level;
            emit batteryLevelChanged(level);
        }
    }
    
    bool charging = (statusStr.contains("Charging"));
    if (charging != m_isCharging) {
        m_isCharging = charging;
        emit chargingStateChanged(charging);
    }
}

void AllySystemControl::adjustFanCurve() {
    // Implement dynamic fan curve based on temperature
    int targetSpeed;
    
    if (m_currentTemp >= 80.0f) {
        targetSpeed = 100;
    } else if (m_currentTemp >= 70.0f) {
        targetSpeed = 80;
    } else if (m_currentTemp >= 60.0f) {
        targetSpeed = 60;
    } else if (m_currentTemp >= 50.0f) {
        targetSpeed = 40;
    } else {
        targetSpeed = 20;
    }
    
    setFanSpeed(targetSpeed);
}

bool AllySystemControl::setFanSpeed(int percentage) {
    if (percentage < 0 || percentage > 100) {
        return false;
    }
    
    if (writeToSysfs(FAN_PATH, QString::number(percentage))) {
        m_fanSpeed = percentage;
        emit fanSpeedChanged(percentage);
        return true;
    }
    return false;
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
    
    file.close();
    return true;
}

QString AllySystemControl::readFromSysfs(const QString& path) {
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << "Failed to open" << path << "for reading";
        return QString();
    }
    
    QString value = QString::fromUtf8(file.readAll()).trimmed();
    file.close();
    return value;
}

AllySystemControl::~AllySystemControl() {
    m_monitorTimer.stop();
}