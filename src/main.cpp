#include <QApplication>
#include "ui/LauncherWindow.hpp"
#include "core/Config.hpp"
#include "steam/SteamIntegration.hpp"

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    
    app.setApplicationName("ally-mc-launcher");
    app.setApplicationVersion(APP_VERSION);
    
    // Initialize configuration
    Config::instance()->load("/etc/ally-mc-launcher/config/default_config.json");
    
    // Initialize Steam integration
    if (!SteamIntegration::instance()->initialize()) {
        qWarning() << "Failed to initialize Steam integration";
    }
    
    LauncherWindow window;
    window.show();
    
    return app.exec();
}