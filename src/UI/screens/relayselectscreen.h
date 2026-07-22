#ifndef RELAYSELECT_H
#define RELAYSELECT_H

#include <QWidget>

#include <memory>

namespace Ui {
class RelaySelect;
}

class QModelIndex;
class relayListModel;

class RelaySelectScreen : public QWidget {
    Q_OBJECT

public:
    explicit RelaySelectScreen(QWidget* parent = nullptr);
    ~RelaySelectScreen();

private:
    void updateSelectionState(const QModelIndex& currentIndex);

    Ui::RelaySelect* ui;
    std::unique_ptr<relayListModel> m_model;
};

#endif // RELAYSELECT_H
