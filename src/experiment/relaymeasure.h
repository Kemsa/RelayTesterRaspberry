#ifndef RELAYMEASURE_H
#define RELAYMEASURE_H

#include "genericstep.h"
#include <QString>
#include <cassert>
#include <memory>
#include <vector>

class GenericStep;

class RelayMeasure {
public:
    RelayMeasure();

    void fromJSON(const QString& jsonString);

private:
    std::vector<std::unique_ptr<GenericStep>> m_steps;
};

#endif // RELAYMEASURE_H
