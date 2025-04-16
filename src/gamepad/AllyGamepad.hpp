#pragma once

#include <QObject>
#include <QGamepad>
#include <QTimer>
#include <memory>
#include <unordered_map>
#include <linux/input.h>
#include <SDL3/SDL.h>

class AllyGamepad : public QObject {
    Q_OBJECT

public:
    enum class Button {
        DPAD_UP,
        DPAD_DOWN,
        DPAD_LEFT,
        DPAD_RIGHT,
        A,
        B,
        X,
        Y,
        L1,
        R1,
        L2,
        R2,
        L4,
        R4,
        L5,
        R5,
        BACK,
        START,
        QUICK_ACCESS,
        COMMAND_CENTER
    };
    Q_ENUM(Button)

    enum class Axis {
        LEFT_X,
        LEFT_Y,
        RIGHT_X,
        RIGHT_Y,
        LEFT_TRIGGER,
        RIGHT_TRIGGER
    };
    Q_ENUM(Axis)

    static AllyGamepad* instance();
    
    bool isConnected() const;
    bool isSteamInputAvailable() const;
    float getAxisValue(Axis axis) const;
    bool isButtonPressed(Button button) const;
    
    // Steam Input integration
    void enableSteamInput();
    void disableSteamInput();
    bool loadSteamInputConfig(const QString& configPath);

signals:
    void buttonPressed(Button button);
    void buttonReleased(Button button);
    void axisChanged(Axis axis, float value);
    void connectionChanged(bool connected);
    void steamInputStatusChanged(bool available);

private:
    explicit AllyGamepad(QObject* parent = nullptr);
    ~AllyGamepad();

    static AllyGamepad* s_instance;
    
    // SDL Gamepad handling
    SDL_GameController* m_controller;
    SDL_JoystickID m_controllerId;
    bool m_connected;
    bool m_steamInputEnabled;
    
    // Axis and button state tracking
    std::unordered_map<Axis, float> m_axisValues;
    std::unordered_map<Button, bool> m_buttonStates;
    
    // Steam Input integration
    void* m_steamInputHandle;
    QString m_currentConfig;
    
    void initializeSDL();
    void setupControllerMapping();
    void pollEvents();
    void handleSDLEvent(const SDL_Event& event);
    void updateAxisValue(Axis axis, float value);
    void updateButtonState(Button button, bool pressed);
    
    // Steam Input functions
    bool initializeSteamInput();
    void cleanupSteamInput();
    
    // Internal timer for polling
    QTimer m_pollTimer;
};