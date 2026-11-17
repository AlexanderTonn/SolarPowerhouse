#pragma once

#include <QObject>
#include <QString>

namespace solar {

class LanguageManager final : public QObject {
    Q_OBJECT
    Q_PROPERTY(QString language READ language WRITE setLanguage NOTIFY languageChanged)
    Q_PROPERTY(QString dateTimeFormat READ dateTimeFormat NOTIFY languageChanged)

public:
    explicit LanguageManager(QObject* parent = nullptr);

    [[nodiscard]] QString language() const;
    void setLanguage(const QString& language);

    [[nodiscard]] QString dateTimeFormat() const;

    Q_INVOKABLE QString text(const QString& key) const;

signals:
    void languageChanged();

private:
    QString language_;
};

}  // namespace solar
