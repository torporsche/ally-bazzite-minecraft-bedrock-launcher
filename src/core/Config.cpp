#include "Config.hpp"
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>

Config* Config::s_instance = nullptr;

Config* Config::instance() {
    if (!s_instance) {
        s_instance = new Config();
    }
    return s_instance;
}

Config::Config(QObject* parent) : QObject(parent) {}

bool Config::load(const QString& path) {
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly)) {
        return false;
    }

    QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
    if (doc.isNull()) {
        return false;
    }

    m_data = doc.object().toVariantMap();
    return true;
}

bool Config::save(const QString& path) const {
    QFile file(path);
    if (!file.open(QIODevice::WriteOnly)) {
        return false;
    }

    QJsonDocument doc(QJsonObject::fromVariantMap(m_data));
    file.write(doc.toJson());
    return true;
}

QVariant Config::value(const QString& key, const QVariant& defaultValue) const {
    return m_data.value(key, defaultValue);
}

void Config::setValue(const QString& key, const QVariant& value) {
    if (m_data[key] != value) {
        m_data[key] = value;
        emit configChanged(key);
    }
}

bool Config::contains(const QString& key) const {
    return m_data.contains(key);
}

void Config::remove(const QString& key) {
    if (m_data.remove(key) > 0) {
        emit configChanged(key);
    }
}