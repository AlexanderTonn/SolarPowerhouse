#include "hmi/WeatherService.hpp"

#include <chrono>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QSettings>
#include <QUrl>
#include <QUrlQuery>

namespace solar {

namespace {

constexpr auto refreshInterval = std::chrono::minutes{15};

QNetworkRequest makeRequest(const QUrl& url) {
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::UserAgentHeader, QStringLiteral("SolarPowerhouse/0.1"));
    return request;
}

} // namespace

WeatherService::WeatherService(QObject* parent)
    : QObject(parent) {
    QSettings settings(QStringLiteral("SolarPowerhouse"), QStringLiteral("solar-hmi"));
    location_ = settings.value(QStringLiteral("weather/location")).toString().trimmed();
    refreshTimer_.setInterval(static_cast<int>(std::chrono::duration_cast<std::chrono::milliseconds>(refreshInterval).count()));
    connect(&refreshTimer_, &QTimer::timeout, this, &WeatherService::refresh);
    if (!location_.isEmpty()) {
        QTimer::singleShot(0, this, &WeatherService::refresh);
    }
}

QString WeatherService::location() const {
    return location_;
}

void WeatherService::setLocation(const QString& location) {
    const auto normalized = location.trimmed();
    if (location_ == normalized) {
        return;
    }
    location_ = normalized;
    QSettings settings(QStringLiteral("SolarPowerhouse"), QStringLiteral("solar-hmi"));
    settings.setValue(QStringLiteral("weather/location"), location_);
    resolvedLocation_.clear();
    condition_.clear();
    available_ = false;
    error_.clear();
    emit locationChanged();
    emit weatherChanged();
    emit errorChanged();
}

QString WeatherService::resolvedLocation() const {
    return resolvedLocation_;
}

QString WeatherService::summary() const {
    if (!available_) {
        return QStringLiteral("Weather unavailable");
    }
    return QStringLiteral("%1 °C · %2")
        .arg(temperature_, 0, 'f', 1)
        .arg(condition_);
}

QString WeatherService::condition() const {
    return condition_;
}

double WeatherService::temperature() const {
    return temperature_;
}

double WeatherService::windSpeed() const {
    return windSpeed_;
}

bool WeatherService::available() const {
    return available_;
}

bool WeatherService::loading() const {
    return loading_;
}

QString WeatherService::error() const {
    return error_;
}

void WeatherService::refresh() {
    if (location_.isEmpty()) {
        fail(QStringLiteral("Weather location is not configured"));
        return;
    }

    if (activeReply_) {
        activeReply_->abort();
        activeReply_->deleteLater();
        activeReply_.clear();
    }

    setLoading(true);
    if (!error_.isEmpty()) {
        error_.clear();
        emit errorChanged();
    }
    startGeocoding();
}

void WeatherService::startGeocoding() {
    QUrl url(QStringLiteral("https://geocoding-api.open-meteo.com/v1/search"));
    QUrlQuery query;
    query.addQueryItem(QStringLiteral("name"), location_);
    query.addQueryItem(QStringLiteral("count"), QStringLiteral("1"));
    query.addQueryItem(QStringLiteral("language"), QStringLiteral("en"));
    query.addQueryItem(QStringLiteral("format"), QStringLiteral("json"));
    url.setQuery(query);

    auto* reply = networkManager_.get(makeRequest(url));
    activeReply_ = reply;
    QTimer::singleShot(15000, reply, [reply]() {
        if (reply->isRunning()) {
            reply->abort();
        }
    });

    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        if (reply != activeReply_) {
            reply->deleteLater();
            return;
        }
        activeReply_.clear();

        if (reply->error() != QNetworkReply::NoError) {
            fail(QStringLiteral("Geocoding failed: %1").arg(reply->errorString()));
            reply->deleteLater();
            return;
        }

        QJsonParseError parseError{};
        const auto document = QJsonDocument::fromJson(reply->readAll(), &parseError);
        const auto results = document.object().value(QStringLiteral("results")).toArray();
        if (parseError.error != QJsonParseError::NoError || results.isEmpty()) {
            fail(QStringLiteral("Location not found"));
            reply->deleteLater();
            return;
        }

        const auto result = results.at(0).toObject();
        const auto latitude = result.value(QStringLiteral("latitude")).toDouble();
        const auto longitude = result.value(QStringLiteral("longitude")).toDouble();
        const auto name = result.value(QStringLiteral("name")).toString();
        const auto country = result.value(QStringLiteral("country")).toString();
        const auto place = country.isEmpty() ? name : QStringLiteral("%1, %2").arg(name, country);
        reply->deleteLater();
        startForecast(latitude, longitude, place);
    });
}

void WeatherService::startForecast(double latitude, double longitude, const QString& place) {
    QUrl url(QStringLiteral("https://api.open-meteo.com/v1/forecast"));
    QUrlQuery query;
    query.addQueryItem(QStringLiteral("latitude"), QString::number(latitude, 'f', 6));
    query.addQueryItem(QStringLiteral("longitude"), QString::number(longitude, 'f', 6));
    query.addQueryItem(QStringLiteral("current"), QStringLiteral("temperature_2m,weather_code,wind_speed_10m"));
    query.addQueryItem(QStringLiteral("timezone"), QStringLiteral("auto"));
    url.setQuery(query);

    auto* reply = networkManager_.get(makeRequest(url));
    activeReply_ = reply;
    QTimer::singleShot(15000, reply, [reply]() {
        if (reply->isRunning()) {
            reply->abort();
        }
    });

    connect(reply, &QNetworkReply::finished, this, [this, reply, place]() {
        if (reply != activeReply_) {
            reply->deleteLater();
            return;
        }
        activeReply_.clear();

        if (reply->error() != QNetworkReply::NoError) {
            fail(QStringLiteral("Weather request failed: %1").arg(reply->errorString()));
            reply->deleteLater();
            return;
        }

        QJsonParseError parseError{};
        const auto document = QJsonDocument::fromJson(reply->readAll(), &parseError);
        const auto current = document.object().value(QStringLiteral("current")).toObject();
        if (parseError.error != QJsonParseError::NoError || current.isEmpty()) {
            fail(QStringLiteral("Weather response is invalid"));
            reply->deleteLater();
            return;
        }

        const auto temperatureValue = current.value(QStringLiteral("temperature_2m"));
        const auto windSpeedValue = current.value(QStringLiteral("wind_speed_10m"));
        const bool validTemperature = temperatureValue.isDouble();
        const bool validWindSpeed = windSpeedValue.isDouble();
        const auto temperature = temperatureValue.toDouble();
        const auto windSpeed = windSpeedValue.toDouble();
        const auto weatherCode = current.value(QStringLiteral("weather_code")).toInt(-1);
        if (!validTemperature || !validWindSpeed || weatherCode < 0) {
            fail(QStringLiteral("Weather response is incomplete"));
            reply->deleteLater();
            return;
        }

        resolvedLocation_ = place;
        temperature_ = temperature;
        windSpeed_ = windSpeed;
        condition_ = conditionForCode(weatherCode);
        available_ = true;
        setLoading(false);
        error_.clear();
        refreshTimer_.start();
        emit weatherChanged();
        emit errorChanged();
        reply->deleteLater();
    });
}

void WeatherService::fail(const QString& message) {
    available_ = false;
    setLoading(false);
    if (error_ != message) {
        error_ = message;
        emit errorChanged();
    }
    emit weatherChanged();
}

void WeatherService::setLoading(bool loading) {
    if (loading_ == loading) {
        return;
    }
    loading_ = loading;
    emit loadingChanged();
}

QString WeatherService::conditionForCode(int code) {
    if (code == 0) return QStringLiteral("Clear");
    if (code <= 3) return QStringLiteral("Partly cloudy");
    if (code <= 48) return QStringLiteral("Fog");
    if (code <= 57) return QStringLiteral("Drizzle");
    if (code <= 67) return QStringLiteral("Rain");
    if (code <= 77) return QStringLiteral("Snow");
    if (code <= 82) return QStringLiteral("Rain showers");
    if (code <= 86) return QStringLiteral("Snow showers");
    return QStringLiteral("Thunderstorm");
}

} // namespace solar
