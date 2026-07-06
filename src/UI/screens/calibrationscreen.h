#ifndef CALIBRATIONSCREEN_H
#define CALIBRATIONSCREEN_H

#include <QWidget>

namespace Ui {
class Calibration;
}

class CalibrationScreen : public QWidget
{
    Q_OBJECT

public:
    explicit CalibrationScreen(QWidget *parent = nullptr);
    ~CalibrationScreen();

private:
    Ui::Calibration *ui;
};

#endif // CALIBRATIONSCREEN_H
