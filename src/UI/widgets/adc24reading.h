#ifndef ADC24READING_H
#define ADC24READING_H

#include <QFrame>

namespace Ui {
class ADC24Reading;
}

class ADC24Reading : public QFrame {
    Q_OBJECT

public:
    explicit ADC24Reading(QWidget* parent = nullptr);
    ~ADC24Reading();

private:
    Ui::ADC24Reading* ui;

    void makeMeasureAndDisplay(int nMeasures);
};

#endif // ADC24READING_H
