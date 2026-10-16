#include "mainWindow.h"
#ifdef Q_OS_WIN
// Must precede project headers: windows.h's INPUT clashes with the GPIO enum.
#define INPUT WIN_INPUT
#include <windows.h>
#include <winioctl.h>
#undef INPUT
#endif
#include "currentadjuster.h"
#include "homescreen.h"
#include "logbus.h"
#include "navigator.h"
#include "powercontrol.h"
#include "testingscreen.h"
#include "ui_mainWindow.h"
#include <QApplication>
#include <QDebug>
#include <QSet>
#include <QStorageInfo>
#include <QStyle>
#ifdef Q_OS_LINUX
#include <unistd.h>
#endif

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent), ui(new Ui::MainWindow) {
    ui->setupUi(this);

#ifdef Q_OS_LINUX
    this->setWindowFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
    this->setWindowState(this->windowState() | Qt::WindowFullScreen);
#else
    this->setFixedSize(800, 480);
#endif

    connect(&LogBus::instance(), &LogBus::messageLogged, this, [this](QtMsgType type, const QString& msg) {
        displayMessage(type, msg);
    });

    Navigator::initialize(this, ui->stackedWidget);

    connect(ui->backPB, &QPushButton::clicked, this, [this]() {
        Navigator::instance().goBack();
    });
    connect(ui->homePB, &QPushButton::clicked, this, [this]() {
        Navigator::instance().goHome();
    });

    connect(PowerControl::getInstance(), &PowerControl::reedStatusChanged, this, [this](bool isClosed) {
        if (isClosed) {
            ui->reed_LED->setState(LedWidget::StateOk);
        } else {
            ui->reed_LED->setState(LedWidget::StateError);
        }
    });

    connect(PowerControl::getInstance(), &PowerControl::boardStatusChanged, this, [this](bool isClosed) {
        if (isClosed) {
            ui->board_LED->setState(LedWidget::StateOk);
        } else {
            ui->board_LED->setState(LedWidget::StateError);
        }
    });

    ui->eject_PB->setIcon(QApplication::style()->standardIcon(QStyle::SP_ArrowUp));
    ui->eject_PB->setIconSize(QSize(16, 16));
    connect(ui->eject_PB, &QPushButton::clicked, this, &MainWindow::ejectUSB);

    ui->board_LED->setState(PowerControl::getInstance()->checkBoardStatus() ? LedWidget::StateOk : LedWidget::StateError);
    ui->reed_LED->setState(PowerControl::getInstance()->checkReedStatus() ? LedWidget::StateOk : LedWidget::StateError);
}

MainWindow::~MainWindow() {
    delete ui;
}

void MainWindow::closeEvent(QCloseEvent* event) {
    if (CurrentAdjuster::isInitialized()) {
        CurrentAdjuster::instance()->closeConnection();
    }

    QMainWindow::closeEvent(event);
}

void MainWindow::displayMessage(QtMsgType type, const QString& msg) {

    switch (type) {
    case QtDebugMsg:
        break;
    case QtInfoMsg:
        this->ui->statusbar->setStyleSheet("color: WhiteSmoke");
        this->ui->statusbar->showMessage(msg, 5000); // Display the message for 5 seconds
        break;
    case QtWarningMsg:
        this->ui->statusbar->setStyleSheet("color: GoldenRod");
        this->ui->statusbar->showMessage(msg, 5000); // Display the message for 5 seconds
        break;
    case QtCriticalMsg:
    case QtFatalMsg:
        this->ui->statusbar->setStyleSheet("color: Red"); // Set the text color to blue for info messages
        this->ui->statusbar->showMessage(msg, 10000);     // Display the message for 10 seconds
        break;
    }
}

#ifdef Q_OS_WIN
namespace {

bool isUsbVolume(const QString& root) {
    const QString path = QStringLiteral("\\\\.\\%1:").arg(root.at(0));
    HANDLE h = CreateFileW(reinterpret_cast<LPCWSTR>(path.utf16()), 0,
                           FILE_SHARE_READ | FILE_SHARE_WRITE, nullptr, OPEN_EXISTING, 0, nullptr);
    if (h == INVALID_HANDLE_VALUE) {
        return false;
    }
    STORAGE_PROPERTY_QUERY query = {};
    query.PropertyId = StorageDeviceProperty;
    query.QueryType = PropertyStandardQuery;
    alignas(STORAGE_DEVICE_DESCRIPTOR) char buffer[1024] = {};
    DWORD returned = 0;
    bool usb = false;
    if (DeviceIoControl(h, IOCTL_STORAGE_QUERY_PROPERTY, &query, sizeof(query), buffer, sizeof(buffer), &returned, nullptr)) {
        usb = reinterpret_cast<STORAGE_DEVICE_DESCRIPTOR*>(buffer)->BusType == BusTypeUsb;
    }
    CloseHandle(h);
    return usb;
}

bool ejectVolume(const QString& root) {
    const QString path = QStringLiteral("\\\\.\\%1:").arg(root.at(0));
    HANDLE h = CreateFileW(reinterpret_cast<LPCWSTR>(path.utf16()), GENERIC_READ | GENERIC_WRITE,
                           FILE_SHARE_READ | FILE_SHARE_WRITE, nullptr, OPEN_EXISTING, 0, nullptr);
    if (h == INVALID_HANDLE_VALUE) {
        return false;
    }
    DWORD returned = 0;
    bool locked = false;
    for (int i = 0; i < 5 && !locked; ++i) {
        locked = DeviceIoControl(h, FSCTL_LOCK_VOLUME, nullptr, 0, nullptr, 0, &returned, nullptr);
        if (!locked) {
            Sleep(300);
        }
    }
    bool ok = false;
    if (locked && DeviceIoControl(h, FSCTL_DISMOUNT_VOLUME, nullptr, 0, nullptr, 0, &returned, nullptr)) {
        PREVENT_MEDIA_REMOVAL pmr = {FALSE};
        DeviceIoControl(h, IOCTL_STORAGE_MEDIA_REMOVAL, &pmr, sizeof(pmr), nullptr, 0, &returned, nullptr);
        ok = DeviceIoControl(h, IOCTL_STORAGE_EJECT_MEDIA, nullptr, 0, nullptr, 0, &returned, nullptr);
    }
    CloseHandle(h);
    return ok;
}

} // namespace
#endif

#ifdef Q_OS_LINUX
#include <QDir>
#include <QFileInfo>
#include <QMap>
#include <QProcess>

namespace {

// Returns the parent disk name (e.g. "sda" for "/dev/sda1") if the device sits on the USB bus.
QString usbParentDisk(const QString& devicePath) {
    const QString name = QFileInfo(devicePath).fileName();
    const QFileInfo sysEntry(QStringLiteral("/sys/class/block/") + name);
    if (!sysEntry.exists()) {
        return QString();
    }
    const QString real = sysEntry.canonicalFilePath();
    if (!real.contains(QStringLiteral("/usb"))) {
        return QString();
    }
    // A partition's parent directory is the disk; a whole disk's parent is not a block device.
    const QString parentName = QFileInfo(real).dir().dirName();
    if (QFileInfo::exists(QStringLiteral("/sys/class/block/") + parentName)) {
        return parentName;
    }
    return name;
}

bool runCmd(const QString& program, const QStringList& args) {
    QProcess p;
    p.start(program, args);
    if (!p.waitForFinished(15000)) {
        p.kill();
        return false;
    }
    return p.exitStatus() == QProcess::NormalExit && p.exitCode() == 0;
}

} // namespace
#endif

void MainWindow::ejectUSB() {
    int found = 0;
    int ejected = 0;

#ifdef Q_OS_WIN
    for (const QStorageInfo& vol : QStorageInfo::mountedVolumes()) {
        const QString root = vol.rootPath();
        if (root.size() < 2 || root.at(1) != ':' || !isUsbVolume(root)) {
            continue;
        }
        ++found;
        if (ejectVolume(root)) {
            ++ejected;
        } else {
            qWarning() << "Impossible d'éjecter" << root << ": volume utilisé par un programme";
        }
    }
#elif defined(Q_OS_LINUX)
    QMap<QString, QStringList> diskPartitions;
    for (const QStorageInfo& vol : QStorageInfo::mountedVolumes()) {
        const QString dev = QString::fromUtf8(vol.device());
        qDebug() << "Found device" << dev << "mounted at" << vol.rootPath();
        if (!vol.rootPath().startsWith(QStringLiteral("/media/"))) {
            continue;
        }
        qDebug() << "Device" << dev << "is a USB device";
        const QString disk = usbParentDisk(dev);
        if (disk.isEmpty()) {
            continue;
        }
        diskPartitions[disk].append(dev);
    }

    for (auto it = diskPartitions.cbegin(); it != diskPartitions.cend(); ++it) {
        const QString diskPath = QStringLiteral("/dev/") + it.key();
        ++found;
        ::sync();

        bool ok = false;
        for (const QString& partDev : it.value()) {
            qDebug() << "running command" << "udisksctl unmount -b" << partDev;
            runCmd(QStringLiteral("udisksctl"), {QStringLiteral("unmount"), QStringLiteral("-b"), partDev});
        }
        qDebug() << "running command" << "udisksctl power-off -b" << diskPath;
        ok = runCmd(QStringLiteral("udisksctl"), {QStringLiteral("power-off"), QStringLiteral("-b"), diskPath});

        if (ok) {
            ++ejected;
        } else {
            qWarning() << "Impossible d'éjecter" << diskPath;
        }
    }

#endif

    if (found == 0) {
        qInfo() << "Aucune clé USB détectée";
    } else if (ejected == found) {
        qInfo() << "Clé USB éjectée, vous pouvez la retirer";
    } else {
        qCritical() << "Échec de l'éjection de la clé USB";
    }
}