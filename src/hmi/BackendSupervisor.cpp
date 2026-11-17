#include "hmi/BackendSupervisor.hpp"

#include <QTimer>
#include <QVariant>

namespace solar {

namespace {
constexpr auto systemctlPath = "/usr/bin/systemctl";
constexpr auto backendUnit = "solar-core.service";
}

BackendSupervisor::BackendSupervisor(QObject* parent)
    : QObject(parent) {
    connect(&process_, &QProcess::finished, this, [this](int exitCode, QProcess::ExitStatus exitStatus) {
        const auto operation = process_.property("operation").toInt();
        setBusy(false);

        if (exitStatus == QProcess::CrashExit) {
            setState(QStringLiteral("Error"));
            setError(QStringLiteral("systemctl terminated unexpectedly"));
            return;
        }

        if (operation == static_cast<int>(Operation::Status)) {
            if (exitCode == 0) {
                setState(QStringLiteral("Running"));
                setError({});
            } else {
                setState(QStringLiteral("Stopped"));
            }
            return;
        }

        if (exitCode == 0) {
            setState(operation == static_cast<int>(Operation::Stop)
                ? QStringLiteral("Stopped")
                : QStringLiteral("Running"));
            setError({});
        } else {
            setState(QStringLiteral("Error"));
            setError(QStringLiteral("Cannot %1 %2: %3")
                .arg(operationName(static_cast<Operation>(operation)),
                     QString::fromLatin1(backendUnit),
                     QString::fromLocal8Bit(process_.readAllStandardError()).trimmed()));
        }
    });
    connect(&process_, &QProcess::errorOccurred, this, [this](QProcess::ProcessError error) {
        if (error == QProcess::FailedToStart) {
            setBusy(false);
            setState(QStringLiteral("Unavailable"));
            setError(QStringLiteral("systemctl is unavailable; automatic backend control disabled"));
        }
    });
    QTimer::singleShot(0, this, &BackendSupervisor::ensureRunning);
}

QString BackendSupervisor::state() const {
    return state_;
}

QString BackendSupervisor::error() const {
    return error_;
}

bool BackendSupervisor::busy() const {
    return busy_;
}

void BackendSupervisor::refresh() {
    run(Operation::Status);
}

void BackendSupervisor::ensureRunning() {
    run(Operation::Start);
}

void BackendSupervisor::start() {
    run(Operation::Start);
}

void BackendSupervisor::stop() {
    run(Operation::Stop);
}

void BackendSupervisor::restart() {
    run(Operation::Restart);
}

void BackendSupervisor::run(Operation operation) {
    if (busy_ || process_.state() != QProcess::NotRunning) {
        return;
    }

    setBusy(true);
    setError({});
    setState(operation == Operation::Status
        ? QStringLiteral("Checking")
        : operation == Operation::Start
            ? QStringLiteral("Starting")
            : operation == Operation::Stop
                ? QStringLiteral("Stopping")
                : QStringLiteral("Restarting"));

    process_.setProperty("operation", static_cast<int>(operation));
    QStringList arguments;
    arguments << QStringLiteral("--no-ask-password");
    if (operation == Operation::Status) {
        arguments << QStringLiteral("is-active") << QStringLiteral("--quiet");
    } else {
        arguments << operationName(operation);
    }
    arguments << QString::fromLatin1(backendUnit);
    process_.start(QString::fromLatin1(systemctlPath), arguments);
}

void BackendSupervisor::setState(const QString& state) {
    if (state_ == state) {
        return;
    }
    state_ = state;
    emit stateChanged();
}

void BackendSupervisor::setError(const QString& error) {
    if (error_ == error) {
        return;
    }
    error_ = error;
    emit errorChanged();
}

void BackendSupervisor::setBusy(bool busy) {
    if (busy_ == busy) {
        return;
    }
    busy_ = busy;
    emit busyChanged();
}

QString BackendSupervisor::operationName(Operation operation) {
    switch (operation) {
    case Operation::Status: return QStringLiteral("is-active");
    case Operation::Start: return QStringLiteral("start");
    case Operation::Stop: return QStringLiteral("stop");
    case Operation::Restart: return QStringLiteral("restart");
    }
    return {};
}

} // namespace solar
