# RelayTesterRaspberry

## Installation

### all builds

This project uses Qt. To compile with vscode, use the following extensions:
- C/C++ extension pack
- Qt C++ extension pack
- Qt extension pack
- cmake

Register Qt installation using command (F1):

```Qt: register```

and point toward qmake folder

For each sub-library, follow the specific instructions for your platform

### build on windows

install Qt5:
- use online installation, it will add Qt6 only
- launch qt maintenance tool. There you can select older version. Select latest Qt5
- register qt installation as Qt5

Project can now be built and debug using cmake

### standalone unit-test project

A parallel unit-test project is available in `unit-tests/` to keep test builds isolated from the main application build.

Build and run on Windows:

```bash
cmake --preset windows-debug-with-tests
cmake --build --preset windows-debug-unit-tests
ctest --preset windows-debug-unit-tests
```

You can still use the standalone folder presets from `unit-tests/` when needed.

Build and run on Raspberry Pi:

```bash
cd unit-tests
cmake --preset raspi-native
cmake --build --preset raspi-native
ctest --preset raspi-native
```

### build on raspberry

Raspberry must be bookworm 32bits. As of june 2026, the picolog ADC24 library is not working on Trixie or later and is not compatible with 64 bits.

- install libs:

```sudo apt install qtcreator build-essential qtbase5-dev cmake```

- in vscode, register qt kit. Path is usually ```/usr/lib/qt5/bin/qmake```

- select the qt5 kit with cmake command (F1):

```cmake: select a kit```

Project can now be built and debug using cmake

### cross compile from windows to raspberry and deploy

A script was made to cross compile the code for raspberry from a widows machine and send it to raspberry. So far, no debugging is possible from remote windows. It uses bash scripts and docker.

- install and configure docker
- enable ssh on raspberry
- create an ssh key and send it to raspberry. It will be used for sending the file and configuration via ssh/scp:
```
ssh-keygen -t ed25519 -f ~/.ssh/id_rsa -N ""
ssh-copy-id -i ~/.ssh/id_rsa -p 22 pi@192.168.1.11
```
Change the file name and pi's address according to your configuration

- copy rpi-deploy.env.example to .vscode/rpi-deploy.env and fill with correct connexion information

To build and deploy, execute "Build + Deploy relayTester to Raspberry Pi" from launch tab. A folder "Relay tester" in home folder will be created containing the new executable. You can execute it from raspberry.