#pragma once

#include <QObject>
#include <QString>
#include <QMap>

class GameManager : public QObject {
    Q_OBJECT

public:
    static GameManager* instance();

    bool applyROGAllyOptimizations();
    bool setupGamepadMapping();
    bool configureGraphicsAPI();
    bool setGameResolution(int width, int height);
    bool setRefreshRate(int hz);
    bool enableFSR(bool enabled);
    
    // Performance profiles
    enum class GraphicsPreset {
        BATTERY_SAVER,
        BALANCED,
        PERFORMANCE
    };
    
    bool setGraphicsPreset(GraphicsPreset preset);

signals:
    void optimizationsChanged();
    void graphicsPresetChanged(GraphicsPreset preset);
    void fpsLimitChanged(int limit);
    void resolutionChanged(int width, int height);
    void graphicsAPIChanged(const QString& api);

private:
    explicit GameManager(QObject* parent = nullptr);
    ~GameManager();

    static GameManager* s_instance;
    
    void setupVulkanLayers();
    void configureGameScope();
    void setupControllerHints();
    void optimizeShaderCache();
    
    // Current settings
    GraphicsPreset m_currentPreset;
    QString m_currentAPI;
    int m_targetFPS;
    bool m_fsrEnabled;
    QMap<QString, QString> m_vulkanLayers;
};