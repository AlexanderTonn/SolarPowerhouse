#pragma once

#include <QProcess>
#include <QObject>
#include <QString>

namespace solar {

class BackendSupervisor final : public QObject {
    Q_OBJECT
    Q_PROPERTY(QString state READ state NOTIFY stateChanged)
    Q_PROPERTY(QString error READ error NOTIFY errorChanged)
    Q_PROPERTY(bool busy READ busy NOTIFY busyChanged)

public:
    explicit BackendSupervisor(QObject* parent = nullptr);

    [[nodiscard]] QString state() const;
    [[nodiscard]] QString error() const;
    [[nodiscard]] bool busy() const;

    Q_INVOKABLE void refresh();
    Q_INVOKABLE void ensureRunning();
    Q_INVOKABLE void start();
    Q_INVOKABLE void stop();
    Q_INVOKABLE void restart();

signals:
    void stateChanged();
    void errorChanged();
    void busyChanged();

private:
    enum class Operation {
        Status,
        Start,
        Stop,
        Restart,
    };

    void run(Operation operation);
    void setState(const QString& state);
    void setError(const QString& error);
    void setBusy(bool busy);
    [[nodiscard]] static QString operationName(Operation operation);

    QProcess process_;
    QString state_{QStringLiteral("Unknown")};
    QString error_{};
    bool busy_{false};
};

} // namespace solar
