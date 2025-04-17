#pragma once

#include <QObject>
#include <QTimer>
#include <QString>
#include <memory>

class AllySystemControl : public QObject {
    Q_OBJECT

public:
    enum class PerformanceProfile {
        SILENT,      // Optimized for battery life
        BALANCED,    // Default profile
        TURBO,       // Maximum performance
        MANUAL       // Custom settings
    };
    Q_ENUM(PerformanceProfile)

    static AllySystemControl* instance();

    bool setPerformanceProfile(PerformanceProfile profile);
    bool setTDP(int watts);  // Range: 5-30W
    bool setGPUFreq(int mhz);
    bool enableFreeSync(bool enabled);
    bool setFanSpeed(int percentage);  // Range: 0-100

    // Getters
    PerformanceProfile currentProfile() const;
    int currentTDP() const;
    int currentGPUFreq() const;
    bool isFreeSyncEnabled() const;
    float getCurrentTemperature() const;
    int getBatteryLevel() const;
    bool isCharging() const;

signals:
    void temperatureChanged(float temp);
    void batteryLevelChanged(int level);
    void chargingStateChanged(bool charging);
    void performanceProfileChanged(PerformanceProfile profile);
    void tdpChanged(int watts);
    void gpuFreqChanged(int mhz);
    void freeSyncStatusChanged(bool enabled);
    void fanSpeedChanged(int percentage);

private:
    explicit AllySystemControl(QObject* parent = nullptr);
    ~AllySystemControl();

    static AllySystemControl* s_instance;

    // Hardware monitoring
    QTimer m_monitorTimer;
    void monitorTemperature();
    void monitorBattery();
    void adjustFanCurve();
    
    // System file paths
    static const QString POWER_PROFILE_PATH;
    static const QString TDP_PATH;
    static const QString GPU_FREQ_PATH;
    static const QString TEMP_PATH;
    static const QString FAN_PATH;
    static const QString BATTERY_PATH;
    
    // Current state
    PerformanceProfile m_currentProfile;
    int m_currentTDP;
    int m_currentGPUFreq;
    bool m_freeSyncEnabled;
    float m_currentTemp;
    int m_batteryLevel;
    bool m_isCharging;
    int m_fanSpeed;

    bool writeToSysfs(const QString& path, const QString& value);
    QString readFromSysfs(const QString& path);
};