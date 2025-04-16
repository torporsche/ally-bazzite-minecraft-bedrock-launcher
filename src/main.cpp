#include <QApplication>
#include <QCommandLineParser>
#include <QtWebEngineWidgets>
#include <QGamepad>
#include "LauncherWindow.hpp"
#include "Config.hpp"
#include "GooglePlayAuth.hpp"

int main(int argc, char *argv[]) {
    // Enable Wayland support
    qputenv("QT_QPA_PLATFORM", "wayland");
    
    // Enable VRR if supported
    qputenv("QT_WAYLAND_DISABLE_WINDOWDECORATION", "1");
    qputenv("ENABLE_VRR", "1");
    
    QApplication app(argc, argv);
    app.setApplicationName("Ally MC Launcher");
    app.setOrganizationName("torporsche");
    
    // Initialize WebEngine for Google Play authentication
    QtWebEngineWidgets::initialize();
    
    // Parse command line arguments
    QCommandLineParser parser;
    parser.setApplicationDescription("Minecraft Bedrock Launcher for ROG Ally X");
    parser.addHelpOption();
    parser.addVersionOption();
    parser.process(app);
    
    // Initialize configuration
    Config::initialize();
    
    // Create and show the main window
    LauncherWindow window;
    window.show();
    
    return app.exec();
}