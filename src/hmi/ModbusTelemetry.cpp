#include "hmi/ModbusTelemetry.hpp"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QDateTime>
#include <QSaveFile>
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
      refreshTimer_(new QTimer(this)) {
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

QStringList ModbusTelemetry::availableMappings() const {
    return {QStringLiteral("TOYO_RENOGY")};
}

void ModbusTelemetry::refresh() {
    QFile file(stateFilePath());
    if (!file.open(QIODevice::ReadOnly)) {
        setUnavailable(QStringLiteral("Opta telemetry state unavailable"));
        return;
    }

    QJsonParseError parseError{};
    const auto document = QJsonDocument::fromJson(file.readAll(), &parseError);
    if (parseError.error != QJsonParseError::NoError || !document.isObject()) {
        setUnavailable(QStringLiteral("Invalid Opta telemetry state"));
        return;
    }

    const auto root = document.object();
    const auto newMappingKey = root.value(QStringLiteral("mapping")).toString(mappingKey_);
    if (newMappingKey != mappingKey_) {
        history_.clear();
        lastSampleTimestamp_.clear();
    }
    connected_ = root.value(QStringLiteral("connected")).toBool(false);
    mappingKey_ = newMappingKey;
    mappingLabel_ = root.value(QStringLiteral("mapping_label")).toString(labelForMapping(mappingKey_));
    updatedAt_ = root.value(QStringLiteral("updated_at")).toString();
    error_ = root.value(QStringLiteral("error")).toString();
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
    if (connected_ && updatedAt_ != lastSampleTimestamp_) {
        QVariantMap sample;
        sample.insert(QStringLiteral("time"), QDateTime::currentMSecsSinceEpoch());
        for (const auto& key : {
                 QStringLiteral("battery_voltage_v"),
                 QStringLiteral("charging_power_w"),
                 QStringLiteral("battery_temperature_c"),
                 QStringLiteral("charging_current_a")}) {
            sample.insert(key, values_.value(key));
        }
        history_.append(sample);
        constexpr int maximumHistorySamples = 3600;
        while (history_.size() > maximumHistorySamples) {
            history_.removeFirst();
        }
        lastSampleTimestamp_ = updatedAt_;
    }
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
    emit changed();
    return true;
}

QString ModbusTelemetry::labelForMapping(const QString& key) const {
    if (key == QStringLiteral("TOYO_RENOGY")) {
        return QStringLiteral("TOYO / RENOGY");
    }
    return key;
}

QString ModbusTelemetry::stateFilePath() {
    return configuredPath(
        "SOLAR_POWERHOUSE_STATE_FILE",
        QStringLiteral("/var/lib/solarpowerhouse/modbus-state.json"),
        QStringLiteral("/tmp/solarpowerhouse-modbus-state.json"));
}

QString ModbusTelemetry::mappingFilePath() {
    return configuredPath(
        "SOLAR_POWERHOUSE_MAPPING_FILE",
        QStringLiteral("/var/lib/solarpowerhouse/modbus-mapping.conf"),
        QStringLiteral("/tmp/solarpowerhouse-modbus-mapping.conf"));
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
