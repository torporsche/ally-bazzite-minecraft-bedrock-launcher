#pragma once

#include <QObject>
#include <QString>
#include <QWebEngineView>
#include <memory>

class GooglePlayAuth : public QObject {
    Q_OBJECT
    
public:
    explicit GooglePlayAuth(QObject *parent = nullptr);
    
    void startAuthentication();
    bool isAuthenticated() const;
    QString getAuthToken() const;
    
signals:
    void authenticationSucceeded();
    void authenticationFailed(const QString& error);
    
private slots:
    void onUrlChanged(const QUrl& url);
    void onLoadFinished(bool ok);
    
private:
    std::unique_ptr<QWebEngineView> m_webView;
    QString m_authToken;
    bool m_isAuthenticated;
    
    void parseAuthResponse(const QUrl& url);
};