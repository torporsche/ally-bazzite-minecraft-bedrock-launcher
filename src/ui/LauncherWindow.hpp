#pragma once

#include <QMainWindow>
#include <QTouchEvent>
#include <QGestureEvent>
#include <QPinchGesture>
#include <QSwipeGesture>
#include <memory>

class LauncherWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit LauncherWindow(QWidget* parent = nullptr);
    ~LauncherWindow();

protected:
    bool event(QEvent* event) override;

signals:
    void touchGestureDetected();
    void bigPictureModeChanged(bool enabled);

private:
    void setupTouchSupport();
    void handlePinchGesture(QPinchGesture* gesture);
    void handleSwipeGesture(QSwipeGesture* gesture);
    void setupBigPictureMode();
    void touchEvent(QTouchEvent* event);
    void gestureEvent(QGestureEvent* event);
    
    // Touch and gesture handling
    void processTouchBegin(const QTouchEvent* event);
    void processTouchUpdate(const QTouchEvent* event);
    void processTouchEnd(const QTouchEvent* event);
    
    // Big Picture Mode
    bool m_bigPictureMode;
    void toggleBigPictureMode(bool enabled);
    void updateUIScale();
    
    // Touch state tracking
    struct TouchPoint {
        QPointF startPos;
        QPointF lastPos;
        qint64 startTime;
    };
    QMap<int, TouchPoint> m_touchPoints;
    
    // Gesture recognition
    bool m_gesturesEnabled;
    float m_pinchScale;
    float m_currentRotation;
};