#ifndef NAVIGATOR_H
#define NAVIGATOR_H

#include <QMainWindow>
#include <QMap>
#include <QStack>
#include <QStackedWidget>

class Navigator {
public:
    enum NavigationScreen {
        Home_screen,
        Calibration_screen,
        RelaySelect_screen,
        // Add more screens as needed
    };

    static Navigator& initialize(QMainWindow* mainWindow, QStackedWidget* stackedWidget);
    static Navigator& instance();

    Navigator(const Navigator&) = delete;
    Navigator& operator=(const Navigator&) = delete;

    void goBack();
    void goHome();
    void navigateTo(NavigationScreen screen);

private:
    explicit Navigator(QMainWindow* mainWindow, QStackedWidget* stackedWidget);

    static Navigator* s_instance;

    QMainWindow* m_mainWindow;
    QStackedWidget* m_stackedWidget;
    QStack<NavigationScreen> m_navigationHistory;
    QMap<NavigationScreen, int> m_screenIndexMap; // Maps screens to their corresponding index in the QStackedWidget

    void addWidgetForScreen(NavigationScreen screen, QWidget* widget);
};

#endif // NAVIGATOR_H
