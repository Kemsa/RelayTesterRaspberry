#ifndef CONTACTSELECTWIDGET_H
#define CONTACTSELECTWIDGET_H

#include <QWidget>
#include <qframe.h>

namespace Ui {
class ContactSelectWidget;
}

class ContactSelectWidget : public QFrame
{
    Q_OBJECT

public:
    explicit ContactSelectWidget(QWidget *parent = nullptr);
    ~ContactSelectWidget();

private:
    Ui::ContactSelectWidget *ui;
};

#endif // CONTACTSELECTWIDGET_H
