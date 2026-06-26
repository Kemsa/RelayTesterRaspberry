#ifndef CALIBRATION_H
#define CALIBRATION_H

#include <QWidget>

namespace Ui {
class Calibration;
}

class Calibration : public QWidget
{
    Q_OBJECT

public:
    explicit Calibration(QWidget *parent = nullptr);
    ~Calibration();

private:
    Ui::Calibration *ui;
};

#endif // CALIBRATION_H
