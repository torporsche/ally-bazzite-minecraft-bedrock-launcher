#include "GooglePlayAuth.hpp"
#include <QDesktopServices>
#include <QUrlQuery>
#include <QSettings>
#include <QStandardPaths>
#include <QDir>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>

GooglePlayAuth::GooglePlayAuth(QObject *parent) 
    : QObject(parent)
    , m_isAuthenticated(false)
    , m_networkManager(new QNetworkAccessManager(this)) {
    
    // Initialize secure storage location
    QString dataPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir().mkpath(dataPath + "/auth");
    m_tokenFile = dataPath + "/auth/google_play_token.enc";
    
    // Try to load existing token
    loadStoredToken();
}

void GooglePlayAuth::startAuthentication() {
    if (!m_webView) {
        m_webView = std::make_unique<QWebEngineView>();
        connect(m_webView.get(), &QWebEngineView::urlChanged,
                this, &GooglePlayAuth::onUrlChanged);
        connect(m_webView.get(), &QWebEngineView::loadFinished,
                this, &GooglePlayAuth::onLoadFinished);
    }

    // Set up custom user agent to mimic Android device
    QString userAgent = "Mozilla/5.0 (Linux; Android 13; ROG Ally) "
                       "AppleWebKit/537.36 (KHTML, like Gecko) "
                       "Chrome/96.0.4664.104 Mobile Safari/537.36";
    m_webView->page()->profile()->setHttpUserAgent(userAgent);

    // Start OAuth flow
    QUrl authUrl("https://accounts.google.com/o/oauth2/v2/auth");
    QUrlQuery query;
    query.addQueryItem("client_id", "com.android.vending");
    query.addQueryItem("response_type", "code");
    query.addQueryItem("scope", "https://www.googleapis.com/auth/androidpublisher");
    query.addQueryItem("redirect_uri", "urn:ietf:wg:oauth:2.0:oob");
    authUrl.setQuery(query);

    m_webView->setUrl(authUrl);
    m_webView->show();
}

void GooglePlayAuth::onUrlChanged(const QUrl& url) {
    // Check for OAuth callback
    if (url.host() == "accounts.google.com" && 
        url.path().contains("oauth2/approval")) {
        parseAuthResponse(url);
    }
}

void GooglePlayAuth::parseAuthResponse(const QUrl& url) {
    QUrlQuery query(url.query());
    QString code = query.queryItemValue("code");
    
    if (!code.isEmpty()) {
        // Exchange authorization code for tokens
        QUrl tokenUrl("https://oauth2.googleapis.com/token");
        QNetworkRequest request(tokenUrl);
        request.setHeader(QNetworkRequest::ContentTypeHeader, 
                         "application/x-www-form-urlencoded");

        QUrlQuery tokenQuery;
        tokenQuery.addQueryItem("code", code);
        tokenQuery.addQueryItem("client_id", "com.android.vending");
        tokenQuery.addQueryItem("grant_type", "authorization_code");
        tokenQuery.addQueryItem("redirect_uri", "urn:ietf:wg:oauth:2.0:oob");

        QNetworkReply* reply = m_networkManager->post(request, 
                                                    tokenQuery.toString().toUtf8());
        
        connect(reply, &QNetworkReply::finished, this, [this, reply]() {
            if (reply->error() == QNetworkReply::NoError) {
                QJsonDocument doc = QJsonDocument::fromJson(reply->readAll());
                QJsonObject obj = doc.object();
                
                m_authToken = obj["access_token"].toString();
                m_refreshToken = obj["refresh_token"].toString();
                m_tokenExpiry = QDateTime::currentDateTime().addSecs(
                    obj["expires_in"].toInt());
                
                storeToken();
                m_isAuthenticated = true;
                emit authenticationSucceeded();
            } else {
                emit authenticationFailed(reply->errorString());
            }
            reply->deleteLater();
            m_webView->hide();
        });
    }
}

void GooglePlayAuth::storeToken() {
    QJsonObject tokenData;
    tokenData["access_token"] = m_authToken;
    tokenData["refresh_token"] = m_refreshToken;
    tokenData["expiry"] = m_tokenExpiry.toString(Qt::ISODate);
    
    // Encrypt token data before storing
    QByteArray tokenJson = QJsonDocument(tokenData).toJson();
    QByteArray encrypted = encryptData(tokenJson);
    
    QFile file(m_tokenFile);
    if (file.open(QIODevice::WriteOnly)) {
        file.write(encrypted);
        file.close();
    }
}

bool GooglePlayAuth::isAuthenticated() const {
    return m_isAuthenticated && 
           QDateTime::currentDateTime() < m_tokenExpiry;
}

QString GooglePlayAuth::getAuthToken() {
    if (!isAuthenticated()) {
        refreshToken();
    }
    return m_authToken;
}

void GooglePlayAuth::refreshToken() {
    if (m_refreshToken.isEmpty()) {
        emit authenticationFailed("No refresh token available");
        return;
    }

    QUrl tokenUrl("https://oauth2.googleapis.com/token");
    QNetworkRequest request(tokenUrl);
    request.setHeader(QNetworkRequest::ContentTypeHeader, 
                     "application/x-www-form-urlencoded");

    QUrlQuery query;
    query.addQueryItem("refresh_token", m_refreshToken);
    query.addQueryItem("client_id", "com.android.vending");
    query.addQueryItem("grant_type", "refresh_token");

    QNetworkReply* reply = m_networkManager->post(request, 
                                                query.toString().toUtf8());
    
    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        if (reply->error() == QNetworkReply::NoError) {
            QJsonDocument doc = QJsonDocument::fromJson(reply->readAll());
            QJsonObject obj = doc.object();
            
            m_authToken = obj["access_token"].toString();
            m_tokenExpiry = QDateTime::currentDateTime().addSecs(
                obj["expires_in"].toInt());
            
            storeToken();
            m_isAuthenticated = true;
            emit authenticationSucceeded();
        } else {
            emit authenticationFailed(reply->errorString());
        }
        reply->deleteLater();
    });
}

// Utility function for simple encryption (this should be enhanced for production)
QByteArray GooglePlayAuth::encryptData(const QByteArray& data) {
    // In a production environment, use proper encryption (e.g., libsodium)
    // For now, using simple XOR encryption for demonstration
    QByteArray key = QCryptographicHash::hash(
        QSysInfo::machineUniqueId(), 
        QCryptographicHash::Sha256
    );
    
    QByteArray encrypted;
    for (int i = 0; i < data.size(); ++i) {
        encrypted.append(data[i] ^ key[i % key.size()]);
    }
    return encrypted;
}