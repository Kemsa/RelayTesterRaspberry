#include "navigator.h"
#include "calibrationscreen.h"
#include "homescreen.h"
#include <QDebug>

Navigator* Navigator::s_instance = nullptr;

Navigator& Navigator::initialize(QMainWindow* mainWindow, QStackedWidget* stackedWidget) {
    if (!s_instance) {
        s_instance = new Navigator(mainWindow, stackedWidget);
    }
    return *s_instance;
}

Navigator& Navigator::instance() {
    Q_ASSERT_X(s_instance, "Navigator::instance", "Navigator is not initialized");
    return *s_instance;
}

Navigator::Navigator(QMainWindow* mainWindow, QStackedWidget* stackedWidget)
    : m_mainWindow(mainWindow), m_stackedWidget(stackedWidget) {

    addWidgetForScreen(Home_screen, new HomeScreen(m_stackedWidget));
    addWidgetForScreen(Calibration_screen, new CalibrationScreen(m_stackedWidget));

    navigateTo(Home_screen); // Show HomeScreen by default
}

void Navigator::navigateTo(NavigationScreen screen) {
    if (m_screenIndexMap.contains(screen)) {
        int index = m_screenIndexMap.value(screen);
        m_stackedWidget->setCurrentIndex(index);
        m_navigationHistory.push(screen);
    }
}

void Navigator::goBack() {
    if (!m_navigationHistory.isEmpty()) {
        m_navigationHistory.pop(); // Remove the current screen
        if (!m_navigationHistory.isEmpty()) {
            NavigationScreen previousScreen = m_navigationHistory.top();
            navigateTo(previousScreen);
        }
    }
}

void Navigator::goHome() {
    navigateTo(Home_screen);
    m_navigationHistory.clear();
}

void Navigator::addWidgetForScreen(NavigationScreen screen, QWidget* widget) {
    int index = m_stackedWidget->addWidget(widget);
    m_screenIndexMap.insert(screen, index);
}