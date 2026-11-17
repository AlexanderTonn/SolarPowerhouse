#include "hmi/UserManager.hpp"

namespace solar {

UserManager::UserManager(QObject* parent)
    : QObject(parent) {}

bool UserManager::passwordProtectionEnabled() const noexcept {
    // Secure credential provisioning is a deployment step. Keep this false
    // until a password database and password-hashing policy are provisioned.
    return false;
}

bool UserManager::setupAccessGranted() const noexcept {
    return setupAccessGranted_;
}

QString UserManager::activeUser() const {
    return activeUser_;
}

QString UserManager::activeRole() const {
    return activeRole_;
}

QString UserManager::accessMessage() const {
    return accessMessage_;
}

QStringList UserManager::roles() const {
    return {
        QStringLiteral("Viewer"),
        QStringLiteral("Operator"),
        QStringLiteral("Administrator"),
    };
}

bool UserManager::hasPermission(const QString& permission) const {
    if (!setupAccessGranted_) {
        return false;
    }
    if (permission == QStringLiteral("setup.view")) {
        return activeRole_ == QStringLiteral("Administrator")
            || activeRole_ == QStringLiteral("Operator");
    }
    if (permission == QStringLiteral("backend.control")
        || permission == QStringLiteral("modbus.mapping.change")) {
        return activeRole_ == QStringLiteral("Administrator");
    }
    return false;
}

bool UserManager::authenticate(const QString& user, const QString& password) {
    Q_UNUSED(user)
    Q_UNUSED(password)
    accessMessage_ = QStringLiteral("Password authentication is not provisioned");
    emit accessChanged();
    return false;
}

void UserManager::lockSetup() {
    if (!passwordProtectionEnabled()) {
        accessMessage_ = QStringLiteral("Setup cannot be locked before password policy provisioning");
        emit accessChanged();
        return;
    }
    setupAccessGranted_ = false;
    activeUser_.clear();
    activeRole_ = QStringLiteral("None");
    accessMessage_ = QStringLiteral("Setup locked");
    emit accessChanged();
}

} // namespace solar
