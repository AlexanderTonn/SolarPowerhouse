#include "hmi/ModbusTelemetry.hpp"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QHostAddress>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QDateTime>
#include <QSaveFile>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QTimer>

#include <cstdlib>

namespace solar {

namespace {

QString configuredPath(const char* variable, const QString& linuxDefault, const QString& developmentDefault) {
    if (const auto* value = std::getenv(variable); value != nullptr && *value != '\0') {
        return QString::fromLocal8Bit(value);
    }
#ifdef __linux__
    return linuxDefault;
#else
    return developmentDefault;
#endif
}

} // namespace

ModbusTelemetry::ModbusTelemetry(QObject* parent)
    : QObject(parent),
      refreshTimer_(new QTimer(this)),
      ipcSocket_(new QLocalSocket(this)) {
    loadModbusConnectionSettings();
    connect(ipcSocket_, &QLocalSocket::readyRead, this, [this]() {
        ipcBuffer_.append(ipcSocket_->readAll());
    });
    connect(ipcSocket_, &QLocalSocket::disconnected, this, [this]() {
        ipcBuffer_.append(ipcSocket_->readAll());
        const auto state = ipcBuffer_;
        ipcBuffer_.clear();
        if (state.isEmpty()) {
            setUnavailable(QStringLiteral("Core telemetry IPC unavailable"));
            return;
        }
        applyState(state);
    });
    connect(ipcSocket_, &QLocalSocket::errorOccurred, this, [this]() {
        if (ipcBuffer_.isEmpty()) {
            setUnavailable(QStringLiteral("Core telemetry IPC unavailable"));
        }
    });
    refreshTimer_->setInterval(1000);
    connect(refreshTimer_, &QTimer::timeout, this, &ModbusTelemetry::refresh);
    refreshTimer_->start();
    refresh();
}

bool ModbusTelemetry::connected() const noexcept {
    return connected_;
}

QString ModbusTelemetry::mappingKey() const {
    return mappingKey_;
}

QString ModbusTelemetry::mappingLabel() const {
    return mappingLabel_;
}

QString ModbusTelemetry::updatedAt() const {
    return updatedAt_;
}

QString ModbusTelemetry::error() const {
    return error_;
}

QVariantMap ModbusTelemetry::values() const {
    return values_;
}

QVariantList ModbusTelemetry::inputRegisters() const {
    return inputRegisters_;
}

QVariantList ModbusTelemetry::history() const {
    return history_;
}

int ModbusTelemetry::historySampleIntervalSeconds() const noexcept {
    return historySampleIntervalSeconds_;
}

int ModbusTelemetry::historyBatchIntervalSeconds() const noexcept {
    return historyBatchIntervalSeconds_;
}

int ModbusTelemetry::historyBatchSampleCount() const noexcept {
    return historyBatchSampleCount_;
}

int ModbusTelemetry::historyEstimatedBufferBytes() const noexcept {
    return historyEstimatedBufferBytes_;
}

int ModbusTelemetry::historyPendingSamples() const noexcept {
    return historyPendingSamples_;
}

bool ModbusTelemetry::historyAvailable() const noexcept {
    return historyAvailable_;
}

QString ModbusTelemetry::historyError() const {
    return historyError_;
}

QString ModbusTelemetry::modbusHost() const {
    return modbusHost_;
}

QString ModbusTelemetry::modbusSubnet() const {
    return modbusSubnet_;
}

QString ModbusTelemetry::modbusGateway() const {
    return modbusGateway_;
}

int ModbusTelemetry::modbusPort() const noexcept {
    return modbusPort_;
}

int ModbusTelemetry::modbusUnitId() const noexcept {
    return modbusUnitId_;
}

QStringList ModbusTelemetry::availableMappings() const {
    return {QStringLiteral("TOYO_RENOGY")};
}

void ModbusTelemetry::refresh() {
    if (ipcSocket_->state() != QLocalSocket::UnconnectedState) {
        return;
    }
    ipcBuffer_.clear();
    ipcSocket_->connectToServer(ipcSocketPath());
}

void ModbusTelemetry::applyState(const QByteArray& state) {
    QJsonParseError parseError{};
    const auto document = QJsonDocument::fromJson(state, &parseError);
    if (parseError.error != QJsonParseError::NoError || !document.isObject()) {
        setUnavailable(QStringLiteral("Invalid core telemetry frame"));
        return;
    }

    const auto root = document.object();
    const auto newMappingKey = root.value(QStringLiteral("mapping")).toString(mappingKey_);
    if (newMappingKey != mappingKey_) {
        history_.clear();
        lastSampleTimestamp_.clear();
        historyLoaded_ = false;
        lastFlushedHistoryTimestampMs_ = -1;
    }
    connected_ = root.value(QStringLiteral("connected")).toBool(false);
    mappingKey_ = newMappingKey;
    mappingLabel_ = root.value(QStringLiteral("mapping_label")).toString(labelForMapping(mappingKey_));
    updatedAt_ = root.value(QStringLiteral("updated_at")).toString();
    error_ = root.value(QStringLiteral("error")).toString();
    const auto modbusObject = root.value(QStringLiteral("modbus")).toObject();
    if (!modbusObject.isEmpty()) {
        modbusHost_ = modbusObject.value(QStringLiteral("host")).toString(modbusHost_);
        modbusSubnet_ = modbusObject.value(QStringLiteral("subnet")).toString(modbusSubnet_);
        modbusGateway_ = modbusObject.value(QStringLiteral("gateway")).toString(modbusGateway_);
        modbusPort_ = modbusObject.value(QStringLiteral("port")).toInt(modbusPort_);
        modbusUnitId_ = modbusObject.value(QStringLiteral("unit_id")).toInt(modbusUnitId_);
    }
    values_.clear();
    const auto valuesObject = root.value(QStringLiteral("values")).toObject();
    for (auto iterator = valuesObject.constBegin(); iterator != valuesObject.constEnd(); ++iterator) {
        values_.insert(iterator.key(), iterator.value().toVariant());
    }
    inputRegisters_.clear();
    const auto inputArray = root.value(QStringLiteral("input_registers")).toArray();
    for (const auto& value : inputArray) {
        inputRegisters_.append(value.toVariant());
    }

    const auto historyObject = root.value(QStringLiteral("history")).toObject();
    historyAvailable_ = historyObject.value(QStringLiteral("available")).toBool(false);
    historyError_ = historyObject.value(QStringLiteral("error")).toString();
    historySampleIntervalSeconds_ = historyObject.value(QStringLiteral("sample_interval_s"))
        .toInt(historySampleIntervalSeconds_);
    historyBatchIntervalSeconds_ = historyObject.value(QStringLiteral("batch_interval_s"))
        .toInt(historyBatchIntervalSeconds_);
    historyBatchSampleCount_ = historyObject.value(QStringLiteral("batch_sample_count"))
        .toInt(historyBatchSampleCount_);
    historyEstimatedBufferBytes_ = historyObject.value(QStringLiteral("estimated_buffer_bytes"))
        .toInt(historyEstimatedBufferBytes_);
    historyPendingSamples_ = historyObject.value(QStringLiteral("pending_samples"))
        .toInt(historyPendingSamples_);
    const auto lastFlushed = historyObject.contains(QStringLiteral("last_flushed_timestamp_ms"))
        ? historyObject.value(QStringLiteral("last_flushed_timestamp_ms")).toVariant().toLongLong()
        : -1;
    if (!historyLoaded_ || lastFlushed != lastFlushedHistoryTimestampMs_
        || (historyAvailable_ && history_.isEmpty())) {
        loadHistoryDatabase();
        lastFlushedHistoryTimestampMs_ = lastFlushed;
    }
    appendLatestHistorySample(
        historyObject.value(QStringLiteral("latest_sample")).toObject().toVariantMap());
    emit changed();
}

bool ModbusTelemetry::selectMapping(const QString& key) {
    if (!availableMappings().contains(key)) {
        return false;
    }

    const QFileInfo fileInfo(mappingFilePath());
    if (!QDir().mkpath(fileInfo.absolutePath())) {
        return false;
    }

    QSaveFile file(mappingFilePath());
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        return false;
    }
    file.write(key.toUtf8());
    file.write("\n");
    if (!file.commit()) {
        return false;
    }

    mappingKey_ = key;
    mappingLabel_ = labelForMapping(key);
    error_.clear();
    history_.clear();
    lastSampleTimestamp_.clear();
    historyLoaded_ = false;
    lastFlushedHistoryTimestampMs_ = -1;
    emit changed();
    return true;
}

bool ModbusTelemetry::updateModbusConnection(
    const QString& host,
    const QString& subnet,
    const QString& gateway,
    int unitId) {
    QHostAddress hostAddress;
    QHostAddress subnetAddress;
    QHostAddress gatewayAddress;
    if (!hostAddress.setAddress(host)
        || hostAddress.protocol() != QAbstractSocket::IPv4Protocol
        || !subnetAddress.setAddress(subnet)
        || subnetAddress.protocol() != QAbstractSocket::IPv4Protocol
        || !gatewayAddress.setAddress(gateway)
        || gatewayAddress.protocol() != QAbstractSocket::IPv4Protocol
        || unitId < 1
        || unitId > 247) {
        return false;
    }

    const QFileInfo fileInfo(modbusConnectionFilePath());
    if (!QDir().mkpath(fileInfo.absolutePath())) {
        return false;
    }

    QSaveFile file(modbusConnectionFilePath());
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        return false;
    }
    const auto settings = QStringLiteral(
        "host=%1\nsubnet=%2\ngateway=%3\nport=%4\nunit_id=%5\n")
        .arg(host, subnet, gateway)
        .arg(modbusPort_)
        .arg(unitId);
    if (file.write(settings.toUtf8()) != settings.toUtf8().size() || !file.commit()) {
        return false;
    }

    modbusHost_ = host;
    modbusSubnet_ = subnet;
    modbusGateway_ = gateway;
    modbusUnitId_ = unitId;
    emit changed();
    return true;
}

bool ModbusTelemetry::updateHistorySettings(
    int sampleIntervalSeconds,
    int batchIntervalSeconds) {
    if (sampleIntervalSeconds < 1
        || sampleIntervalSeconds > 3600
        || batchIntervalSeconds < sampleIntervalSeconds
        || batchIntervalSeconds > 7 * 24 * 60 * 60) {
        return false;
    }

    const QFileInfo fileInfo(historySettingsPath());
    if (!QDir().mkpath(fileInfo.absolutePath())) {
        return false;
    }

    QSaveFile file(historySettingsPath());
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        return false;
    }
    const auto settings = QStringLiteral("sample_interval_s=%1\nbatch_interval_s=%2\n")
        .arg(sampleIntervalSeconds)
        .arg(batchIntervalSeconds);
    if (file.write(settings.toUtf8()) != settings.toUtf8().size() || !file.commit()) {
        return false;
    }

    historySampleIntervalSeconds_ = sampleIntervalSeconds;
    historyBatchIntervalSeconds_ = batchIntervalSeconds;
    historyBatchSampleCount_ =
        (batchIntervalSeconds + sampleIntervalSeconds - 1) / sampleIntervalSeconds;
    historyEstimatedBufferBytes_ = historyBatchSampleCount_ * 40;
    historyError_.clear();
    emit changed();
    return true;
}

QString ModbusTelemetry::labelForMapping(const QString& key) const {
    if (key == QStringLiteral("TOYO_RENOGY")) {
        return QStringLiteral("TOYO / RENOGY");
    }
    return key;
}

QString ModbusTelemetry::ipcSocketPath() {
    return configuredPath(
        "SOLAR_POWERHOUSE_IPC_SOCKET",
        QStringLiteral("/run/solarpowerhouse/telemetry.sock"),
        QStringLiteral("/tmp/solarpowerhouse-telemetry.sock"));
}

QString ModbusTelemetry::mappingFilePath() {
    return configuredPath(
        "SOLAR_POWERHOUSE_MAPPING_FILE",
        QStringLiteral("/var/lib/solarpowerhouse/modbus-mapping.conf"),
        QStringLiteral("/tmp/solarpowerhouse-modbus-mapping.conf"));
}

QString ModbusTelemetry::modbusConnectionFilePath() {
    return configuredPath(
        "SOLAR_POWERHOUSE_MODBUS_CONFIG_FILE",
        QStringLiteral("/var/lib/solarpowerhouse/modbus-connection.conf"),
        QStringLiteral("/tmp/solarpowerhouse-modbus-connection.conf"));
}

QString ModbusTelemetry::historyDatabasePath() {
    return configuredPath(
        "SOLAR_POWERHOUSE_HISTORY_DB",
        QStringLiteral("/var/lib/solarpowerhouse/telemetry.sqlite"),
        QStringLiteral("/tmp/solarpowerhouse-telemetry.sqlite"));
}

QString ModbusTelemetry::historySettingsPath() {
    return configuredPath(
        "SOLAR_POWERHOUSE_HISTORY_SETTINGS_FILE",
        QStringLiteral("/var/lib/solarpowerhouse/telemetry-history.conf"),
        QStringLiteral("/tmp/solarpowerhouse-telemetry-history.conf"));
}

void ModbusTelemetry::loadModbusConnectionSettings() {
    QFile file(modbusConnectionFilePath());
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return;
    }

    QString host = modbusHost_;
    QString subnet = modbusSubnet_;
    QString gateway = modbusGateway_;
    int port = modbusPort_;
    int unitId = modbusUnitId_;
    while (!file.atEnd()) {
        const auto line = QString::fromUtf8(file.readLine()).trimmed();
        if (line.isEmpty() || line.startsWith(QLatin1Char('#'))) {
            continue;
        }
        const auto separator = line.indexOf(QLatin1Char('='));
        if (separator < 0) {
            return;
        }
        const auto key = line.left(separator).trimmed();
        const auto value = line.mid(separator + 1).trimmed();
        if (key == QStringLiteral("host")) {
            host = value;
        } else if (key == QStringLiteral("subnet")) {
            subnet = value;
        } else if (key == QStringLiteral("gateway")) {
            gateway = value;
        } else if (key == QStringLiteral("port")) {
            bool ok = false;
            const auto parsed = value.toInt(&ok);
            if (!ok || parsed < 1 || parsed > 65535) {
                return;
            }
            port = parsed;
        } else if (key == QStringLiteral("unit_id")) {
            bool ok = false;
            const auto parsed = value.toInt(&ok);
            if (!ok || parsed < 1 || parsed > 247) {
                return;
            }
            unitId = parsed;
        } else {
            return;
        }
    }

    QHostAddress hostAddress;
    QHostAddress subnetAddress;
    QHostAddress gatewayAddress;
    if (!hostAddress.setAddress(host)
        || hostAddress.protocol() != QAbstractSocket::IPv4Protocol
        || !subnetAddress.setAddress(subnet)
        || subnetAddress.protocol() != QAbstractSocket::IPv4Protocol
        || !gatewayAddress.setAddress(gateway)
        || gatewayAddress.protocol() != QAbstractSocket::IPv4Protocol) {
        return;
    }

    modbusHost_ = host;
    modbusSubnet_ = subnet;
    modbusGateway_ = gateway;
    modbusPort_ = port;
    modbusUnitId_ = unitId;
}

void ModbusTelemetry::loadHistoryDatabase() {
    const auto connectionName = QStringLiteral("solar-history-%1")
        .arg(reinterpret_cast<quintptr>(this), 0, 16);
    QVariantList loadedSamples;
    QString databaseError;
    {
        auto database = QSqlDatabase::addDatabase(QStringLiteral("QSQLITE"), connectionName);
        database.setDatabaseName(historyDatabasePath());
        database.setConnectOptions(QStringLiteral("QSQLITE_OPEN_READONLY"));
        if (!database.open()) {
            databaseError = database.lastError().text();
        } else {
            QSqlQuery query(database);
            if (!query.exec(
                    QStringLiteral(
                        "SELECT timestamp_ms, battery_voltage_v, charging_power_w, "
                        "battery_temperature_c, charging_current_a "
                        "FROM telemetry_samples ORDER BY timestamp_ms ASC"))) {
                databaseError = query.lastError().text();
            } else {
                while (query.next()) {
                    QVariantMap sample;
                    sample.insert(QStringLiteral("time"), query.value(0).toLongLong());
                    sample.insert(QStringLiteral("battery_voltage_v"), query.value(1));
                    sample.insert(QStringLiteral("charging_power_w"), query.value(2));
                    sample.insert(QStringLiteral("battery_temperature_c"), query.value(3));
                    sample.insert(QStringLiteral("charging_current_a"), query.value(4));
                    loadedSamples.append(sample);
                }
            }
            database.close();
        }
    }
    QSqlDatabase::removeDatabase(connectionName);

    historyLoaded_ = true;
    if (!databaseError.isEmpty()) {
        if (historyAvailable_) {
            historyError_ = databaseError;
        }
        return;
    }
    history_ = loadedSamples;
    historyError_.clear();
}

void ModbusTelemetry::appendLatestHistorySample(const QVariantMap& sample) {
    if (sample.isEmpty() || !sample.contains(QStringLiteral("time"))) {
        return;
    }

    const auto timestamp = sample.value(QStringLiteral("time")).toLongLong();
    if (history_.isEmpty()) {
        history_.append(sample);
        return;
    }

    const auto lastTimestamp = history_.last().toMap().value(QStringLiteral("time")).toLongLong();
    if (timestamp > lastTimestamp) {
        history_.append(sample);
    } else if (timestamp == lastTimestamp) {
        history_.replace(history_.size() - 1, sample);
    }
}

void ModbusTelemetry::setUnavailable(const QString& error) {
    const auto wasChanged = connected_ || this->error_ != error || !values_.isEmpty() || !inputRegisters_.isEmpty();
    connected_ = false;
    this->error_ = error;
    values_.clear();
    inputRegisters_.clear();
    if (wasChanged) {
        emit changed();
    }
}

} // namespace solar
