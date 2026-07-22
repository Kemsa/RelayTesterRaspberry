#include <QtTest>

int runContactSelectorTest(int argc, char** argv);
int runDynamicReadingsTest(int argc, char** argv);
int runPowerControlTest(int argc, char** argv);
int runRelayListModelTest(int argc, char** argv);

int main(int argc, char** argv) {
    int result = 0;
    result |= runContactSelectorTest(argc, argv);
    result |= runDynamicReadingsTest(argc, argv);
    result |= runPowerControlTest(argc, argv);
    result |= runRelayListModelTest(argc, argv);
    return result;
}
