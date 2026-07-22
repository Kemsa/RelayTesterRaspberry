#ifndef RELAYMEASURESCREEN_H
#define RELAYMEASURESCREEN_H

#include <QWidget>

namespace Ui {
class RelayMeasureScreen;
}

class RelayMeasureScreen : public QWidget
{
    Q_OBJECT

public:
    explicit RelayMeasureScreen(QWidget *parent = nullptr);
    ~RelayMeasureScreen();

private:
    Ui::RelayMeasureScreen *ui;
};

#endif // RELAYMEASURESCREEN_H
