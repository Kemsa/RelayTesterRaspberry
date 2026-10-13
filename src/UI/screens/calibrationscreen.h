#ifndef CALIBRATIONSCREEN_H
#define CALIBRATIONSCREEN_H

#include <QWidget>

namespace Ui {
class CalibrationScreen;
}

class CalibrationScreen : public QWidget
{
    Q_OBJECT

public:
    explicit CalibrationScreen(QWidget *parent = nullptr);
    ~CalibrationScreen();

private slots:
    void calibrate();
    void exportCalibration();

private:
    Ui::CalibrationScreen *ui;
};

#endif // CALIBRATIONSCREEN_H
