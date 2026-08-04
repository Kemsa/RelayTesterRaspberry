#include "jsonvalidator.h"

#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonParseError>
#include <cmath>

namespace {

QString childPath(const QString& parentPath, const QString& childName) {
    return parentPath.isEmpty() ? childName : parentPath + childName;
}

bool isIntegerValue(const QJsonValue& value) {
    if (!value.isDouble()) {
        return false;
    }

    const double number = value.toDouble();
    return std::floor(number) == number;
}

bool validateType(const QJsonValue& instance, const QString& type, const QString& path, QString* error) {
    if (type == QStringLiteral("object") && instance.isObject()) {
        return true;
    }
    if (type == QStringLiteral("array") && instance.isArray()) {
        return true;
    }
    if (type == QStringLiteral("string") && instance.isString()) {
        return true;
    }
    if (type == QStringLiteral("boolean") && instance.isBool()) {
        return true;
    }
    if (type == QStringLiteral("number") && instance.isDouble()) {
        return true;
    }
    if (type == QStringLiteral("integer") && isIntegerValue(instance)) {
        return true;
    }

    *error = QStringLiteral("expected %1 at %2").arg(type, path);
    return false;
}

bool validateNumericBounds(const QJsonValue& instance, const QJsonObject& schema, const QString& path, QString* error) {
    if (!instance.isDouble()) {
        return true;
    }

    const double number = instance.toDouble();
    if (schema.contains(QStringLiteral("minimum")) && number < schema.value(QStringLiteral("minimum")).toDouble()) {
        *error = QStringLiteral("value at %1 is below minimum").arg(path);
        return false;
    }

    if (schema.contains(QStringLiteral("maximum")) && number > schema.value(QStringLiteral("maximum")).toDouble()) {
        *error = QStringLiteral("value at %1 is above maximum").arg(path);
        return false;
    }

    return true;
}

bool validateConst(const QJsonValue& instance, const QJsonObject& schema, const QString& path, QString* error) {
    if (!schema.contains(QStringLiteral("const"))) {
        return true;
    }

    if (schema.value(QStringLiteral("const")) == instance) {
        return true;
    }

    *error = QStringLiteral("value at %1 does not match expected constant").arg(path);
    return false;
}

bool validateEnum(const QJsonValue& instance, const QJsonObject& schema, const QString& path, QString* error) {
    const QJsonArray enumValues = schema.value(QStringLiteral("enum")).toArray();
    if (enumValues.isEmpty()) {
        return true;
    }

    for (const QJsonValue& enumValue : enumValues) {
        if (enumValue == instance) {
            return true;
        }
    }

    *error = QStringLiteral("value at %1 is not part of enum").arg(path);
    return false;
}

bool validateRef(const QJsonValue& instance,
                 const QString& ref,
                 const QJsonObject& rootSchema,
                 const QString& path,
                 QString* error);

bool validateValue(const QJsonValue& instance,
                   const QJsonObject& schema,
                   const QJsonObject& rootSchema,
                   const QString& path,
                   QString* error);

bool validateOneOf(const QJsonValue& instance,
                   const QJsonObject& schema,
                   const QJsonObject& rootSchema,
                   const QString& path,
                   QString* error) {
    const QJsonArray variants = schema.value(QStringLiteral("oneOf")).toArray();
    if (variants.isEmpty()) {
        return true;
    }

    int matchCount = 0;
    QString lastError;
    for (const QJsonValue& variantValue : variants) {
        if (!variantValue.isObject()) {
            continue;
        }

        QString variantError;
        if (validateValue(instance, variantValue.toObject(), rootSchema, path, &variantError)) {
            ++matchCount;
        } else {
            lastError = variantError;
        }
    }

    if (matchCount == 1) {
        return true;
    }

    if (matchCount == 0) {
        *error = QStringLiteral("value at %1 does not match any allowed schema: %2").arg(path, lastError);
    } else {
        *error = QStringLiteral("value at %1 matches multiple schema variants").arg(path);
    }

    return false;
}

bool validateRequiredProperties(const QJsonObject& object,
                                const QJsonObject& schema,
                                const QString& path,
                                QString* error) {
    const QJsonArray requiredProperties = schema.value(QStringLiteral("required")).toArray();
    for (const QJsonValue& requiredValue : requiredProperties) {
        const QString propertyName = requiredValue.toString();
        if (!object.contains(propertyName)) {
            *error = QStringLiteral("missing required property %1").arg(childPath(path, QStringLiteral(".") + propertyName));
            return false;
        }
    }

    return true;
}

bool validateObject(const QJsonValue& instance,
                    const QJsonObject& schema,
                    const QJsonObject& rootSchema,
                    const QString& path,
                    QString* error) {
    if (!instance.isObject()) {
        *error = QStringLiteral("expected object at %1").arg(path);
        return false;
    }

    const QJsonObject object = instance.toObject();
    if (!validateRequiredProperties(object, schema, path, error)) {
        return false;
    }

    const QJsonObject properties = schema.value(QStringLiteral("properties")).toObject();
    for (auto it = properties.begin(); it != properties.end(); ++it) {
        if (!object.contains(it.key()) || !it.value().isObject()) {
            continue;
        }

        if (!validateValue(object.value(it.key()),
                           it.value().toObject(),
                           rootSchema,
                           childPath(path, QStringLiteral(".") + it.key()),
                           error)) {
            return false;
        }
    }

    return true;
}

bool validateArray(const QJsonValue& instance,
                   const QJsonObject& schema,
                   const QJsonObject& rootSchema,
                   const QString& path,
                   QString* error) {
    if (!instance.isArray()) {
        *error = QStringLiteral("expected array at %1").arg(path);
        return false;
    }

    if (!schema.value(QStringLiteral("items")).isObject()) {
        return true;
    }

    const QJsonObject itemSchema = schema.value(QStringLiteral("items")).toObject();
    const QJsonArray array = instance.toArray();
    for (int index = 0; index < array.size(); ++index) {
        if (!validateValue(array.at(index), itemSchema, rootSchema, QStringLiteral("%1[%2]").arg(path).arg(index), error)) {
            return false;
        }
    }

    return true;
}

bool validateValue(const QJsonValue& instance,
                   const QJsonObject& schema,
                   const QJsonObject& rootSchema,
                   const QString& path,
                   QString* error) {
    if (schema.contains(QStringLiteral("$ref"))) {
        return validateRef(instance, schema.value(QStringLiteral("$ref")).toString(), rootSchema, path, error);
    }

    if (!validateOneOf(instance, schema, rootSchema, path, error)) {
        return false;
    }

    if (!validateConst(instance, schema, path, error)) {
        return false;
    }

    if (!validateEnum(instance, schema, path, error)) {
        return false;
    }

    const QString type = schema.value(QStringLiteral("type")).toString();
    if (!type.isEmpty() && !validateType(instance, type, path, error)) {
        return false;
    }

    if (!validateNumericBounds(instance, schema, path, error)) {
        return false;
    }

    if (type == QStringLiteral("object")) {
        return validateObject(instance, schema, rootSchema, path, error);
    }

    if (type == QStringLiteral("array")) {
        return validateArray(instance, schema, rootSchema, path, error);
    }

    return true;
}

bool validateRef(const QJsonValue& instance,
                 const QString& ref,
                 const QJsonObject& rootSchema,
                 const QString& path,
                 QString* error) {
    if (!ref.startsWith(QStringLiteral("#/"))) {
        *error = QStringLiteral("unsupported schema reference %1").arg(ref);
        return false;
    }

    QJsonValue resolved(rootSchema);
    const QStringList segments = ref.mid(2).split(QLatin1Char('/'), Qt::SkipEmptyParts);
    for (const QString& segment : segments) {
        if (!resolved.isObject()) {
            *error = QStringLiteral("invalid schema reference %1").arg(ref);
            return false;
        }

        resolved = resolved.toObject().value(segment);
        if (resolved.isUndefined()) {
            *error = QStringLiteral("unknown schema reference %1").arg(ref);
            return false;
        }
    }

    if (!resolved.isObject()) {
        *error = QStringLiteral("schema reference %1 does not resolve to an object").arg(ref);
        return false;
    }

    return validateValue(instance, resolved.toObject(), rootSchema, path, error);
}

} // namespace

namespace experiment {

bool JsonValidator::loadJsonObject(const QString& filePath, QJsonObject* jsonObject, QString* error) {
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        *error = QStringLiteral("cannot open file");
        return false;
    }

    QJsonParseError parseError;
    const QJsonDocument document = QJsonDocument::fromJson(file.readAll(), &parseError);
    if (parseError.error != QJsonParseError::NoError) {
        *error = QStringLiteral("JSON parse error: %1").arg(parseError.errorString());
        return false;
    }

    if (!document.isObject()) {
        *error = QStringLiteral("root JSON value must be an object");
        return false;
    }

    *jsonObject = document.object();
    return true;
}

bool JsonValidator::validateValue(const QJsonValue& instance,
                                  const QJsonObject& schema,
                                  const QJsonObject& rootSchema,
                                  const QString& path,
                                  QString* error) {
    return ::validateValue(instance, schema, rootSchema, path, error);
}

} // namespace experiment
