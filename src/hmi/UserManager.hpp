#pragma once

#include <QObject>
#include <QString>
#include <QStringList>

namespace solar {

class UserManager final : public QObject {
    Q_OBJECT
    Q_PROPERTY(bool passwordProtectionEnabled READ passwordProtectionEnabled CONSTANT)
    Q_PROPERTY(bool setupAccessGranted READ setupAccessGranted NOTIFY accessChanged)
    Q_PROPERTY(QString activeUser READ activeUser NOTIFY accessChanged)
    Q_PROPERTY(QString activeRole READ activeRole NOTIFY accessChanged)
    Q_PROPERTY(QString accessMessage READ accessMessage NOTIFY accessChanged)
    Q_PROPERTY(QStringList roles READ roles CONSTANT)

public:
    explicit UserManager(QObject* parent = nullptr);

    [[nodiscard]] bool passwordProtectionEnabled() const noexcept;
    [[nodiscard]] bool setupAccessGranted() const noexcept;
    [[nodiscard]] QString activeUser() const;
    [[nodiscard]] QString activeRole() const;
    [[nodiscard]] QString accessMessage() const;
    [[nodiscard]] QStringList roles() const;

    Q_INVOKABLE bool hasPermission(const QString& permission) const;
    Q_INVOKABLE bool authenticate(const QString& user, const QString& password);
    Q_INVOKABLE void lockSetup();

signals:
    void accessChanged();

private:
    bool setupAccessGranted_{true};
    QString activeUser_{QStringLiteral("commissioning")};
    QString activeRole_{QStringLiteral("Administrator")};
    QString accessMessage_{QStringLiteral("Commissioning access active; password policy is not enabled")};
};

} // namespace solar
