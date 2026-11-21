#pragma once

#include <QByteArray>
#include <QVariantMap>
#include <QVariantList>
#include <QLocalSocket>
#include <QObject>
#include <QString>
#include <QStringList>
#include <QtGlobal>

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
    Q_PROPERTY(int historySampleIntervalSeconds READ historySampleIntervalSeconds NOTIFY changed)
    Q_PROPERTY(int historyBatchIntervalSeconds READ historyBatchIntervalSeconds NOTIFY changed)
    Q_PROPERTY(int historyBatchSampleCount READ historyBatchSampleCount NOTIFY changed)
    Q_PROPERTY(int historyEstimatedBufferBytes READ historyEstimatedBufferBytes NOTIFY changed)
    Q_PROPERTY(int historyPendingSamples READ historyPendingSamples NOTIFY changed)
    Q_PROPERTY(bool historyAvailable READ historyAvailable NOTIFY changed)
    Q_PROPERTY(QString historyError READ historyError NOTIFY changed)
    Q_PROPERTY(QString modbusHost READ modbusHost NOTIFY changed)
    Q_PROPERTY(QString modbusSubnet READ modbusSubnet NOTIFY changed)
    Q_PROPERTY(QString modbusGateway READ modbusGateway NOTIFY changed)
    Q_PROPERTY(int modbusPort READ modbusPort NOTIFY changed)
    Q_PROPERTY(int modbusUnitId READ modbusUnitId NOTIFY changed)
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
    [[nodiscard]] int historySampleIntervalSeconds() const noexcept;
    [[nodiscard]] int historyBatchIntervalSeconds() const noexcept;
    [[nodiscard]] int historyBatchSampleCount() const noexcept;
    [[nodiscard]] int historyEstimatedBufferBytes() const noexcept;
    [[nodiscard]] int historyPendingSamples() const noexcept;
    [[nodiscard]] bool historyAvailable() const noexcept;
    [[nodiscard]] QString historyError() const;
    [[nodiscard]] QString modbusHost() const;
    [[nodiscard]] QString modbusSubnet() const;
    [[nodiscard]] QString modbusGateway() const;
    [[nodiscard]] int modbusPort() const noexcept;
    [[nodiscard]] int modbusUnitId() const noexcept;
    [[nodiscard]] QStringList availableMappings() const;

    Q_INVOKABLE void refresh();
    Q_INVOKABLE bool selectMapping(const QString& key);
    Q_INVOKABLE bool updateModbusConnection(
        const QString& host,
        const QString& subnet,
        const QString& gateway,
        int unitId);
    Q_INVOKABLE bool updateHistorySettings(int sampleIntervalSeconds, int batchIntervalSeconds);
    Q_INVOKABLE QString labelForMapping(const QString& key) const;

signals:
    void changed();

private:
    [[nodiscard]] static QString ipcSocketPath();
    [[nodiscard]] static QString mappingFilePath();
    [[nodiscard]] static QString modbusConnectionFilePath();
    [[nodiscard]] static QString historyDatabasePath();
    [[nodiscard]] static QString historySettingsPath();
    void loadModbusConnectionSettings();
    void applyState(const QByteArray& state);
    void loadHistoryDatabase();
    void appendLatestHistorySample(const QVariantMap& sample);
    void setUnavailable(const QString& error);

    QTimer* refreshTimer_;
    QLocalSocket* ipcSocket_;
    QByteArray ipcBuffer_{};
    bool connected_{false};
    QString mappingKey_{QStringLiteral("TOYO_RENOGY")};
    QString mappingLabel_{QStringLiteral("TOYO / RENOGY")};
    QString updatedAt_{};
    QString error_{};
    QVariantMap values_{};
    QVariantList inputRegisters_{};
    QVariantList history_{};
    QString lastSampleTimestamp_{};
    int historySampleIntervalSeconds_{10};
    int historyBatchIntervalSeconds_{300};
    int historyBatchSampleCount_{30};
    int historyEstimatedBufferBytes_{1200};
    int historyPendingSamples_{0};
    bool historyAvailable_{false};
    QString historyError_{};
    QString modbusHost_{QStringLiteral("192.168.10.20")};
    QString modbusSubnet_{QStringLiteral("255.255.255.0")};
    QString modbusGateway_{QStringLiteral("192.168.10.1")};
    int modbusPort_{502};
    int modbusUnitId_{1};
    qint64 lastFlushedHistoryTimestampMs_{-1};
    bool historyLoaded_{false};
};

} // namespace solar
