#ifndef GENERICSTEP_H
#define GENERICSTEP_H

#include <QJsonObject>

class GenericStep {
public:
    virtual ~GenericStep() = default;

    virtual void fromJSON(const QJsonObject& object) = 0;
};

#endif // GENERICSTEP_H
