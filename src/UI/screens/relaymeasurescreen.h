#ifndef RELAYMEASURESCREEN_H
#define RELAYMEASURESCREEN_H

#include <QString>
#include <QWidget>

class QShowEvent;

namespace Ui {
class RelayMeasureScreen;
}

class RelayMeasureScreen : public QWidget {
    Q_OBJECT

public:
    explicit RelayMeasureScreen(QWidget* parent = nullptr);
    ~RelayMeasureScreen();

public slots:
    void onNavigatedTo(const QString& path);

signals:
    void navigatedTo(const QString& path);

protected:
    void showEvent(QShowEvent* event) override;

private:
    Ui::RelayMeasureScreen* ui;
};

#endif // RELAYMEASURESCREEN_H
