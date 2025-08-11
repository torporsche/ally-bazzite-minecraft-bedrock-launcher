#include "LauncherWindow.hpp"
#include <QApplication>
#include <QScreen>
#include <QTimer>
#include <QtMath>
#include <QPropertyAnimation>

LauncherWindow::LauncherWindow(QWidget* parent)
    : QMainWindow(parent)
    , m_bigPictureMode(false)
    , m_gesturesEnabled(true)
    , m_pinchScale(1.0f)
    , m_currentRotation(0.0f) {
    
    setupTouchSupport();
    setupBigPictureMode();
    
    // Set window attributes for Steam Deck/ROG Ally
    setWindowFlag(Qt::FramelessWindowHint);
    setAttribute(Qt::WA_AcceptTouchEvents);
    
    // Enable gesture support
    grabGesture(Qt::PinchGesture);
    grabGesture(Qt::SwipeGesture);
    grabGesture(Qt::PanGesture);
}

void LauncherWindow::setupTouchSupport() {
    QList<QScreen*> screens = QApplication::screens();
    for (QScreen* screen : screens) {
        if (screen->physicalDotsPerInch() > 150) {
            // High DPI touch screen detected, adjust UI scaling
            qreal scaling = screen->physicalDotsPerInch() / 96.0;
            setProperty("scaling", scaling);
        }
    }
}

bool LauncherWindow::event(QEvent* event) {
    switch (event->type()) {
        case QEvent::TouchBegin:
        case QEvent::TouchUpdate:
        case QEvent::TouchEnd:
            touchEvent(static_cast<QTouchEvent*>(event));
            return true;
            
        case QEvent::Gesture:
            gestureEvent(static_cast<QGestureEvent*>(event));
            return true;
            
        default:
            return QMainWindow::event(event);
    }
}

void LauncherWindow::touchEvent(QTouchEvent* event) {
    switch (event->type()) {
        case QEvent::TouchBegin:
            processTouchBegin(event);
            break;
            
        case QEvent::TouchUpdate:
            processTouchUpdate(event);
            break;
            
        case QEvent::TouchEnd:
            processTouchEnd(event);
            break;
            
        default:
            break;
    }
    
    emit touchGestureDetected();
}

void LauncherWindow::gestureEvent(QGestureEvent* event) {
    if (QGesture* pinch = event->gesture(Qt::PinchGesture)) {
        handlePinchGesture(static_cast<QPinchGesture*>(pinch));
    }
    
    if (QGesture* swipe = event->gesture(Qt::SwipeGesture)) {
        handleSwipeGesture(static_cast<QSwipeGesture*>(swipe));
    }
}

void LauncherWindow::processTouchBegin(const QTouchEvent* event) {
    const QList<QTouchEvent::TouchPoint> touchPoints = event->points();
    
    for (const QTouchEvent::TouchPoint& tp : touchPoints) {
        TouchPoint point;
        point.startPos = tp.position();
        point.lastPos = tp.position();
        point.startTime = QDateTime::currentMSecsSinceEpoch();
        m_touchPoints.insert(tp.id(), point);
    }
}

void LauncherWindow::processTouchUpdate(const QTouchEvent* event) {
    const QList<QTouchEvent::TouchPoint> touchPoints = event->points();
    
    if (touchPoints.size() == 2) {
        // Handle pinch/zoom
        const QTouchEvent::TouchPoint& tp1 = touchPoints.at(0);
        const QTouchEvent::TouchPoint& tp2 = touchPoints.at(1);
        
        // Use pressPosition() instead of startPosition() for Qt6
        QLineF line1(tp1.pressPosition(), tp2.pressPosition());
        QLineF line2(tp1.position(), tp2.position());
        
        qreal scale = line2.length() / line1.length();
        if (qAbs(scale - m_pinchScale) > 0.1) {
            handlePinchGesture(nullptr);
        }
    }
}

void LauncherWindow::processTouchEnd(const QTouchEvent* event) {
    const QList<QTouchEvent::TouchPoint> touchPoints = event->points();
    
    for (const QTouchEvent::TouchPoint& tp : touchPoints) {
        if (m_touchPoints.contains(tp.id())) {
            const TouchPoint& startPoint = m_touchPoints[tp.id()];
            
            // Calculate swipe
            QPointF delta = tp.position() - startPoint.startPos;
            qint64 duration = QDateTime::currentMSecsSinceEpoch() - startPoint.startTime;
            
            if (duration < 300) { // Short duration for swipe
                if (qAbs(delta.x()) > 50 || qAbs(delta.y()) > 50) {
                    // Handle swipe
                    QSwipeGesture swipe;
                    handleSwipeGesture(&swipe);
                }
            }
            
            m_touchPoints.remove(tp.id());
        }
    }
}

void LauncherWindow::handlePinchGesture(QPinchGesture* gesture) {
    if (!m_gesturesEnabled) return;
    
    if (gesture) {
        m_pinchScale *= gesture->scaleFactor();
        
        // Update UI scaling
        updateUIScale();
    }
}

void LauncherWindow::handleSwipeGesture(QSwipeGesture* gesture) {
    if (!m_gesturesEnabled) return;
    
    if (gesture) {
        if (gesture->state() == Qt::GestureFinished) {
            if (gesture->horizontalDirection() == QSwipeGesture::Left) {
                // Handle left swipe
                toggleBigPictureMode(!m_bigPictureMode);
            }
        }
    }
}

void LauncherWindow::setupBigPictureMode() {
    // Set up animations for transitions
    QPropertyAnimation* scaleAnimation = new QPropertyAnimation(this, "windowOpacity");
    scaleAnimation->setDuration(250);
    scaleAnimation->setEasingCurve(QEasingCurve::InOutQuad);
}

void LauncherWindow::toggleBigPictureMode(bool enabled) {
    m_bigPictureMode = enabled;
    
    if (enabled) {
        // Switch to fullscreen optimized for controller/touch
        showFullScreen();
        setStyleSheet("QMainWindow { background-color: #1a1a1a; }");
        
        // Increase UI element sizes
        updateUIScale();
    } else {
        showNormal();
        setStyleSheet("");
    }
    
    // Emit signal for other components to adjust
    emit bigPictureModeChanged(enabled);
}

void LauncherWindow::updateUIScale() {
    qreal scale = m_bigPictureMode ? 1.5 : 1.0;
    scale *= m_pinchScale;
    
    // Update font sizes
    QFont font = QApplication::font();
    font.setPointSizeF(font.pointSizeF() * scale);
    QApplication::setFont(font);
    
    // Update icon sizes
    int iconSize = static_cast<int>(32 * scale);
    setIconSize(QSize(iconSize, iconSize));
}

LauncherWindow::~LauncherWindow() {
    // Cleanup
}