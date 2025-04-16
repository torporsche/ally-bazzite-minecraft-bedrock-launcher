#include "AllyGamepad.hpp"
#include <QDebug>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <dlfcn.h>
#include <fcntl.h>
#include <unistd.h>

// ROG Ally specific controller mapping
static const char* ALLY_CONTROLLER_MAPPING = 
    "03000000091200000200000011010000,"
    "ROG Ally Controller,"
    "platform:Linux,"
    "a:b0,b:b1,x:b2,y:b3,"
    "back:b8,start:b9,guide:b10,command:b11,"
    "leftshoulder:b4,rightshoulder:b5,"
    "lefttrigger:a2,righttrigger:a5,"
    "leftx:a0,lefty:a1,rightx:a3,righty:a4,"
    "dpup:h0.1,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,"
    "leftstick:b6,rightstick:b7,"
    "paddle1:b12,paddle2:b13,paddle3:b14,paddle4:b15";

AllyGamepad* AllyGamepad::s_instance = nullptr;

AllyGamepad* AllyGamepad::instance() {
    if (!s_instance) {
        s_instance = new AllyGamepad();
    }
    return s_instance;
}

AllyGamepad::AllyGamepad(QObject* parent)
    : QObject(parent)
    , m_controller(nullptr)
    , m_controllerId(-1)
    , m_connected(false)
    , m_steamInputEnabled(false)
    , m_steamInputHandle(nullptr) {
    
    initializeSDL();
    
    // Setup polling timer
    connect(&m_pollTimer, &QTimer::timeout, this, &AllyGamepad::pollEvents);
    m_pollTimer.start(16); // ~60Hz polling rate
    
    // Initialize Steam Input if available
    if (initializeSteamInput()) {
        qDebug() << "Steam Input initialized successfully";
    }
}

AllyGamepad::~AllyGamepad() {
    if (m_controller) {
        SDL_GameControllerClose(m_controller);
    }
    cleanupSteamInput();
    SDL_Quit();
}

void AllyGamepad::initializeSDL() {
    if (SDL_Init(SDL_INIT_GAMECONTROLLER) < 0) {
        qWarning() << "Failed to initialize SDL:" << SDL_GetError();
        return;
    }
    
    // Add ROG Ally controller mapping
    SDL_GameControllerAddMapping(ALLY_CONTROLLER_MAPPING);
    
    // Try to open the controller
    for (int i = 0; i < SDL_NumJoysticks(); ++i) {
        if (SDL_IsGameController(i)) {
            m_controller = SDL_GameControllerOpen(i);
            if (m_controller) {
                m_controllerId = SDL_JoystickInstanceID(
                    SDL_GameControllerGetJoystick(m_controller)
                );
                m_connected = true;
                emit connectionChanged(true);
                break;
            }
        }
    }
}

void AllyGamepad::pollEvents() {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        handleSDLEvent(event);
    }
    
    // Update Steam Input if enabled
    if (m_steamInputEnabled && m_steamInputHandle) {
        // Steam Input polling would go here
        // This requires the Steam Input API which isn't publicly available
        // We'll implement this when we have access to the Steam Input SDK
    }
}

void AllyGamepad::handleSDLEvent(const SDL_Event& event) {
    switch (event.type) {
        case SDL_CONTROLLERDEVICEADDED:
            if (!m_controller) {
                m_controller = SDL_GameControllerOpen(event.cdevice.which);
                if (m_controller) {
                    m_controllerId = SDL_JoystickInstanceID(
                        SDL_GameControllerGetJoystick(m_controller)
                    );
                    m_connected = true;
                    emit connectionChanged(true);
                }
            }
            break;
            
        case SDL_CONTROLLERDEVICEREMOVED:
            if (event.cdevice.which == m_controllerId) {
                SDL_GameControllerClose(m_controller);
                m_controller = nullptr;
                m_connected = false;
                emit connectionChanged(false);
            }
            break;
            
        case SDL_CONTROLLERBUTTONDOWN:
        case SDL_CONTROLLERBUTTONUP:
            if (event.cbutton.which == m_controllerId) {
                Button button;
                switch (event.cbutton.button) {
                    case SDL_CONTROLLER_BUTTON_A:
                        button = Button::A;
                        break;
                    case SDL_CONTROLLER_BUTTON_B:
                        button = Button::B;
                        break;
                    case SDL_CONTROLLER_BUTTON_X:
                        button = Button::X;
                        break;
                    case SDL_CONTROLLER_BUTTON_Y:
                        button = Button::Y;
                        break;
                    // Add mappings for ROG Ally specific buttons
                    case SDL_CONTROLLER_BUTTON_BACK:
                        button = Button::BACK;
                        break;
                    case SDL_CONTROLLER_BUTTON_START:
                        button = Button::START;
                        break;
                    case SDL_CONTROLLER_BUTTON_GUIDE:
                        button = Button::QUICK_ACCESS;
                        break;
                    case static_cast<Uint8>(SDL_CONTROLLER_BUTTON_MISC1):
                        button = Button::COMMAND_CENTER;
                        break;
                    default:
                        return;
                }
                
                bool pressed = (event.type == SDL_CONTROLLERBUTTONDOWN);
                updateButtonState(button, pressed);
                
                if (pressed) {
                    emit buttonPressed(button);
                } else {
                    emit buttonReleased(button);
                }
            }
            break;
            
        case SDL_CONTROLLERAXISMOTION:
            if (event.caxis.which == m_controllerId) {
                Axis axis;
                float value = event.caxis.value / 32767.0f;
                
                switch (event.caxis.axis) {
                    case SDL_CONTROLLER_AXIS_LEFTX:
                        axis = Axis::LEFT_X;
                        break;
                    case SDL_CONTROLLER_AXIS_LEFTY:
                        axis = Axis::LEFT_Y;
                        break;
                    case SDL_CONTROLLER_AXIS_RIGHTX:
                        axis = Axis::RIGHT_X;
                        break;
                    case SDL_CONTROLLER_AXIS_RIGHTY:
                        axis = Axis::RIGHT_Y;
                        break;
                    case SDL_CONTROLLER_AXIS_TRIGGERLEFT:
                        axis = Axis::LEFT_TRIGGER;
                        break;
                    case SDL_CONTROLLER_AXIS_TRIGGERRIGHT:
                        axis = Axis::RIGHT_TRIGGER;
                        break;
                    default:
                        return;
                }
                
                updateAxisValue(axis, value);
                emit axisChanged(axis, value);
            }
            break;
    }
}

void AllyGamepad::updateAxisValue(Axis axis, float value) {
    m_axisValues[axis] = value;
}

void AllyGamepad::updateButtonState(Button button, bool pressed) {
    m_buttonStates[button] = pressed;
}

bool AllyGamepad::isConnected() const {
    return m_connected;
}

bool AllyGamepad::isSteamInputAvailable() const {
    return m_steamInputHandle != nullptr;
}

float AllyGamepad::getAxisValue(Axis axis) const {
    auto it = m_axisValues.find(axis);
    return (it != m_axisValues.end()) ? it->second : 0.0f;
}

bool AllyGamepad::isButtonPressed(Button button) const {
    auto it = m_buttonStates.find(button);
    return (it != m_buttonStates.end()) ? it->second : false;
}

// Steam Input Integration
bool AllyGamepad::initializeSteamInput() {
    // Try to load Steam Input API library
    void* steamApiLib = dlopen("libsteam_api.so", RTLD_NOW);
    if (!steamApiLib) {
        qWarning() << "Failed to load Steam API:" << dlerror();
        return false;
    }
    
    // Initialize Steam Input interface
    typedef bool (*SteamInputInit_t)();
    auto SteamInputInit = (SteamInputInit_t)dlsym(steamApiLib, "SteamInput_Init");
    if (!SteamInputInit) {
        dlclose(steamApiLib);
        return false;
    }
    
    if (!SteamInputInit()) {
        dlclose(steamApiLib);
        return false;
    }
    
    m_steamInputHandle = steamApiLib;
    return true;
}

void AllyGamepad::cleanupSteamInput() {
    if (m_steamInputHandle) {
        dlclose(m_steamInputHandle);
        m_steamInputHandle = nullptr;
    }
}

void AllyGamepad::enableSteamInput() {
    if (m_steamInputHandle && !m_steamInputEnabled) {
        m_steamInputEnabled = true;
        // Initialize Steam Input configuration
        emit steamInputStatusChanged(true);
    }
}

void AllyGamepad::disableSteamInput() {
    if (m_steamInputEnabled) {
        m_steamInputEnabled = false;
        emit steamInputStatusChanged(false);
    }
}

bool AllyGamepad::loadSteamInputConfig(const QString& configPath) {
    if (!m_steamInputEnabled || !m_steamInputHandle) {
        return false;
    }
    
    QFile configFile(configPath);
    if (!configFile.open(QIODevice::ReadOnly)) {
        return false;
    }
    
    QJsonDocument configDoc = QJsonDocument::fromJson(configFile.readAll());
    if (configDoc.isNull()) {
        return false;
    }
    
    // Apply Steam Input configuration
    // This would use the Steam Input API to apply the configuration
    // Implementation depends on Steam Input SDK access
    
    m_currentConfig = configPath;
    return true;
}