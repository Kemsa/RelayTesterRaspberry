#ifndef TESTINGSCREEN_H
#define TESTINGSCREEN_H

#include <QWidget>

namespace Ui {
class TestScreen;
}

class TestScreen : public QWidget {
    Q_OBJECT

public:
    explicit TestScreen(QWidget* parent = nullptr);
    ~TestScreen();

private:
    Ui::TestScreen* ui;
};

#endif // TESTINGSCREEN_H
