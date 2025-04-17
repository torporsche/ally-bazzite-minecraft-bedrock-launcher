#pragma once

#include <QObject>
#include <QVariantMap>

class Config : public QObject {
    Q_OBJECT

public:
    static Config* instance();
    
    bool load(const QString& path);
    bool save(const QString& path) const;
    
    QVariant value(const QString& key, const QVariant& defaultValue = QVariant()) const;
    void setValue(const QString& key, const QVariant& value);
    
    bool contains(const QString& key) const;
    void remove(const QString& key);
    
signals:
    void configChanged(const QString& key);

private:
    explicit Config(QObject* parent = nullptr);
    ~Config() = default;
    
    static Config* s_instance;
    QVariantMap m_data;
};