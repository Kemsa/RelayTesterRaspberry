#ifndef RELAYMEASURE_H
#define RELAYMEASURE_H

#include "genericstep.h"
#include <QJsonObject>
#include <QString>
#include <cassert>
#include <memory>
#include <vector>

class GenericStep;

class RelayMeasure {
public:
    RelayMeasure();
    explicit RelayMeasure(QJsonObject schema);

    void fromJSON(const QString& jsonString);

private:
    QJsonObject m_schema;
    bool m_hasSchema = false;
    std::vector<std::unique_ptr<GenericStep>> m_steps;
};

#endif // RELAYMEASURE_H
