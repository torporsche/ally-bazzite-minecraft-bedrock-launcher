#pragma once

#include <QObject>
#include <QString>
#include <QNetworkAccessManager>
#include <QVersionNumber>
#include <QFuture>

struct MinecraftVersion {
    QVersionNumber version;
    QString versionString;
    QString versionCode;
    bool isBeta;
    qint64 size;
};

class GooglePlayAPI : public QObject {
    Q_OBJECT

public:
    explicit GooglePlayAPI(QObject *parent = nullptr);

    QFuture<QList<MinecraftVersion>> getAvailableVersions();
    QFuture<bool> downloadVersion(const MinecraftVersion& version, 
                                const QString& downloadPath);
    
signals:
    void downloadProgress(qint64 bytesReceived, qint64 bytesTotal);
    void downloadError(const QString& error);

private:
    QNetworkAccessManager* m_networkManager;
    QString m_deviceId;
    
    void generateDeviceId();
    QByteArray createDownloadRequest(const MinecraftVersion& version);
};