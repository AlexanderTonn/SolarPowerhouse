#pragma once

#include <QNetworkAccessManager>
#include <QObject>
#include <QPointer>
#include <QString>
#include <QTimer>

class QNetworkReply;

namespace solar {

class WeatherService final : public QObject {
    Q_OBJECT
    Q_PROPERTY(QString location READ location WRITE setLocation NOTIFY locationChanged)
    Q_PROPERTY(QString resolvedLocation READ resolvedLocation NOTIFY weatherChanged)
    Q_PROPERTY(QString summary READ summary NOTIFY weatherChanged)
    Q_PROPERTY(QString condition READ condition NOTIFY weatherChanged)
    Q_PROPERTY(double temperature READ temperature NOTIFY weatherChanged)
    Q_PROPERTY(double windSpeed READ windSpeed NOTIFY weatherChanged)
    Q_PROPERTY(bool available READ available NOTIFY weatherChanged)
    Q_PROPERTY(bool loading READ loading NOTIFY loadingChanged)
    Q_PROPERTY(QString error READ error NOTIFY errorChanged)

public:
    explicit WeatherService(QObject* parent = nullptr);

    [[nodiscard]] QString location() const;
    void setLocation(const QString& location);
    [[nodiscard]] QString resolvedLocation() const;
    [[nodiscard]] QString summary() const;
    [[nodiscard]] QString condition() const;
    [[nodiscard]] double temperature() const;
    [[nodiscard]] double windSpeed() const;
    [[nodiscard]] bool available() const;
    [[nodiscard]] bool loading() const;
    [[nodiscard]] QString error() const;

    Q_INVOKABLE void refresh();

signals:
    void locationChanged();
    void weatherChanged();
    void loadingChanged();
    void errorChanged();

private:
    void startGeocoding();
    void startForecast(double latitude, double longitude, const QString& place);
    void fail(const QString& message);
    void setLoading(bool loading);
    static QString conditionForCode(int code);

    QNetworkAccessManager networkManager_;
    QTimer refreshTimer_;
    QPointer<QNetworkReply> activeReply_;
    QString location_;
    QString resolvedLocation_;
    QString condition_;
    QString error_;
    double temperature_{0.0};
    double windSpeed_{0.0};
    bool available_{false};
    bool loading_{false};
};

} // namespace solar
