#include "GooglePlayAPI.hpp"
#include <QUuid>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QtConcurrent>
#include <QStandardPaths>

GooglePlayAPI::GooglePlayAPI(QObject *parent)
    : QObject(parent)
    , m_networkManager(new QNetworkAccessManager(this)) {
    generateDeviceId();
}

void GooglePlayAPI::generateDeviceId() {
    // Generate a persistent device ID
    QSettings settings;
    m_deviceId = settings.value("device_id").toString();
    
    if (m_deviceId.isEmpty()) {
        m_deviceId = QUuid::createUuid().toString();
        settings.setValue("device_id", m_deviceId);
    }
}

QFuture<QList<MinecraftVersion>> GooglePlayAPI::getAvailableVersions() {
    return QtConcurrent::run([this]() {
        QList<MinecraftVersion> versions;
        
        QNetworkRequest request(QUrl("https://android.clients.google.com/fdfe/details"));
        request.setHeader(QNetworkRequest::ContentTypeHeader, 
                         "application/x-www-form-urlencoded");
        
        QUrlQuery query;
        query.addQueryItem("doc", "com.mojang.minecraftpe");
        query.addQueryItem("deviceId", m_deviceId);
        
        // Send synchronous request
        QNetworkReply* reply = m_networkManager->post(request, 
                                                    query.toString().toUtf8());
        
        QEventLoop loop;
        connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
        loop.exec();
        
        if (reply->error() == QNetworkReply::NoError) {
            QJsonDocument doc = QJsonDocument::fromJson(reply->readAll());
            QJsonObject details = doc.object()["details"].toObject();
            
            // Parse available versions
            QJsonArray versionList = details["versionList"].toArray();
            for (const QJsonValue& ver : versionList) {
                QJsonObject verObj = ver.toObject();
                MinecraftVersion mcVer;
                mcVer.versionString = verObj["versionString"].toString();
                mcVer.versionCode = verObj["versionCode"].toString();
                mcVer.version = QVersionNumber::fromString(mcVer.versionString);
                mcVer.isBeta = verObj["isBeta"].toBool();
                mcVer.size = verObj["size"].toInteger();
                
                if (mcVer.version >= QVersionNumber(1, 16)) {
                    versions.append(mcVer);
                }
            }
        }
        
        reply->deleteLater();
        return versions;
    });
}

QFuture<bool> GooglePlayAPI::downloadVersion(
    const MinecraftVersion& version,
    const QString& downloadPath
) {
    return QtConcurrent::run([this, version, downloadPath]() {
        QNetworkRequest request(QUrl("https://android.clients.google.com/fdfe/delivery"));
        request.setHeader(QNetworkRequest::ContentTypeHeader, 
                         "application/x-www-form-urlencoded");
        
        QUrlQuery query;
        query.addQueryItem("doc", "com.mojang.minecraftpe");
        query.addQueryItem("vc", version.versionCode);
        query.addQueryItem("deviceId", m_deviceId);
        
        QNetworkReply* reply = m_networkManager->post(request, 
                                                    query.toString().toUtf8());
        
        QFile file(downloadPath);
        if (!file.open(QIODevice::WriteOnly)) {
            emit downloadError("Cannot open file for writing");
            return false;
        }
        
        connect(reply, &QNetworkReply::downloadProgress,
                this, &GooglePlayAPI::downloadProgress);
        
        QEventLoop loop;
        connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
        
        while (!reply->isFinished()) {
            if (reply->bytesAvailable()) {
                file.write(reply->readAll());
            }
            loop.exec();
        }
        
        file.close();
        bool success = (reply->error() == QNetworkReply::NoError);
        
        if (!success) {
            emit downloadError(reply->errorString());
            file.remove();
        }
        
        reply->deleteLater();
        return success;
    });
}