#ifndef JSONVALIDATOR_H
#define JSONVALIDATOR_H

#include <QJsonObject>
#include <QJsonValue>
#include <QString>

namespace experiment {

class JsonValidator {
public:
    static bool loadJsonObject(const QString& filePath, QJsonObject* jsonObject, QString* error);
    static bool validateValue(const QJsonValue& instance,
                              const QJsonObject& schema,
                              const QJsonObject& rootSchema,
                              const QString& path,
                              QString* error);

    static bool applyDefaults(QJsonObject* instance,
                              const QJsonObject& schema,
                              const QJsonObject& rootSchema,
                              QString* error = nullptr);
};

} // namespace experiment

#endif // JSONVALIDATOR_H
