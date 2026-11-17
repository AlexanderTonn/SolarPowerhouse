#pragma once

#include <QVariantMap>
#include <QVariantList>
#include <QObject>
#include <QString>
#include <QStringList>

class QTimer;

namespace solar {

class ModbusTelemetry final : public QObject {
    Q_OBJECT
    Q_PROPERTY(bool connected READ connected NOTIFY changed)
    Q_PROPERTY(QString mappingKey READ mappingKey NOTIFY changed)
    Q_PROPERTY(QString mappingLabel READ mappingLabel NOTIFY changed)
    Q_PROPERTY(QString updatedAt READ updatedAt NOTIFY changed)
    Q_PROPERTY(QString error READ error NOTIFY changed)
    Q_PROPERTY(QVariantMap values READ values NOTIFY changed)
    Q_PROPERTY(QVariantList inputRegisters READ inputRegisters NOTIFY changed)
    Q_PROPERTY(QVariantList history READ history NOTIFY changed)
    Q_PROPERTY(QStringList availableMappings READ availableMappings CONSTANT)

public:
    explicit ModbusTelemetry(QObject* parent = nullptr);

    [[nodiscard]] bool connected() const noexcept;
    [[nodiscard]] QString mappingKey() const;
    [[nodiscard]] QString mappingLabel() const;
    [[nodiscard]] QString updatedAt() const;
    [[nodiscard]] QString error() const;
    [[nodiscard]] QVariantMap values() const;
    [[nodiscard]] QVariantList inputRegisters() const;
    [[nodiscard]] QVariantList history() const;
    [[nodiscard]] QStringList availableMappings() const;

    Q_INVOKABLE void refresh();
    Q_INVOKABLE bool selectMapping(const QString& key);
    Q_INVOKABLE QString labelForMapping(const QString& key) const;

signals:
    void changed();

private:
    [[nodiscard]] static QString stateFilePath();
    [[nodiscard]] static QString mappingFilePath();
    void setUnavailable(const QString& error);

    QTimer* refreshTimer_;
    bool connected_{false};
    QString mappingKey_{QStringLiteral("TOYO_RENOGY")};
    QString mappingLabel_{QStringLiteral("TOYO / RENOGY")};
    QString updatedAt_{};
    QString error_{};
    QVariantMap values_{};
    QVariantList inputRegisters_{};
    QVariantList history_{};
    QString lastSampleTimestamp_{};
};

} // namespace solar
