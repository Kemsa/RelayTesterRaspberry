#include "genericstep.h"

void GenericStep::setResultStatus(ResultStatus status) {
    if (resultStatus != status) {
        resultStatus = status;
        emit measureStatusChanged(resultStatus);
    }
}

GenericStep::ResultStatus GenericStep::getResultStatus() const {
    return resultStatus;
}