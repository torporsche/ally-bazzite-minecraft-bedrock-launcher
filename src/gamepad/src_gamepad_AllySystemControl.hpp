#pragma once

#include <QObject>
#include <QString>

class AllySystemControl : public QObject {
    Q_OBJECT

public:
    enum class PerformanceMode {
        SILENT,
        BALANCED,
        TURBO,
        CUSTOM
    };
    Q_ENUM(PerformanceMode)

    static AllySystemControl* instance();

    // VRR Control
    bool isVRRSupported() const;
    bool isVRREnabled() const;
    bool setVRREnabled(bool enabled);
    int getCurrentRefreshRate() const;
    QList<int> getSupportedRefreshRates() const;
    bool setRefreshRate(int rate);

    // Performance Profile Control
    PerformanceMode getCurrentMode() const;
    bool setPerformanceMode(PerformanceMode mode);
    
    // Custom Performance Settings
    bool setCustomTDP(int watts);
    bool setCustomFanSpeed(int percentage);
    int getCurrentTDP() const;
    int getCurrentFanSpeed() const;

signals:
    void vrrStatusChanged(bool enabled);
    void refreshRateChanged(int rate);
    void performanceModeChanged(PerformanceMode mode);
    void tdpChanged(int watts);
    void fanSpeedChanged(int percentage);

private:
    explicit AllySystemControl(QObject* parent = nullptr);
    ~AllySystemControl();

    static AllySystemControl* s_instance;

    bool m_vrrSupported;
    bool m_vrrEnabled;
    int m_currentRefreshRate;
    PerformanceMode m_currentMode;
    int m_currentTDP;
    int m_currentFanSpeed;

    // System file paths
    QString m_vrrSysfsPath;
    QString m_perfModePath;
    QString m_tdpPath;
    QString m_fanPath;

    bool initializeVRRControl();
    bool initializePerformanceControl();
    bool writeToSysfs(const QString& path, const QString& value);
    QString readFromSysfs(const QString& path);
};