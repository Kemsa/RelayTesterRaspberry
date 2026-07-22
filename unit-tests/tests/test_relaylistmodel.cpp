#include <QDir>
#include <QFile>
#include <QTemporaryDir>
#include <QtTest>

#include "relaylistmodel.h"

namespace {
const char* kMinimalRelaySchema = R"JSON({
    "type": "object",
    "properties": {
        "relayBrand": { "type": "string" },
        "relayModel": { "type": "string" },
        "measures": {
            "type": "array",
            "items": {
                "type": "object",
                "properties": {
                    "name": { "type": "string" },
                    "parameters": {
                        "oneOf": [
                            { "$ref": "#/definitions/coilResistanceMeasure" }
                        ]
                    }
                },
                "required": ["name", "parameters"]
            }
        }
    },
    "required": ["relayBrand", "relayModel", "measures"],
    "definitions": {
        "coilResistanceMeasure": {
            "type": "object",
            "properties": {
                "measureType": { "const": "coilResistance" },
                "coilToMeasure": { "enum": [1, 2] },
                "supplyVoltage_cV": { "type": "integer", "minimum": 0, "maximum": 6000 },
                "successValues": {
                    "type": "object",
                    "properties": {
                        "minResistance_ohm": { "type": "integer", "minimum": 0 },
                        "maxResistance_ohm": { "type": "integer", "minimum": 0 }
                    },
                    "required": ["minResistance_ohm", "maxResistance_ohm"]
                }
            },
            "required": ["measureType", "coilToMeasure", "supplyVoltage_cV", "successValues"]
        }
    }
})JSON";

const char* kValidRelayJson = R"JSON({
    "relayBrand": "axicom",
    "relayModel": "v23062-B0029",
    "measures": [
        {
            "name": "coil resistance",
            "parameters": {
                "measureType": "coilResistance",
                "coilToMeasure": 1,
                "supplyVoltage_cV": 3500,
                "successValues": {
                    "minResistance_ohm": 2125,
                    "maxResistance_ohm": 2875
                }
            }
        }
    ]
})JSON";

const char* kInvalidRelayJson = R"JSON({
    "relayBrand": "axicom",
    "measures": []
})JSON";

void writeTextFile(const QString& filePath, const QByteArray& content) {
    QFile file(filePath);
    QVERIFY(file.open(QIODevice::WriteOnly | QIODevice::Text));
    QVERIFY(file.write(content) != -1);
    file.close();
}
} // namespace

class RelayListModelTest : public QObject {
    Q_OBJECT

private slots:
    void showsFoldersAsRootAndFilesAsLeaves();
};

void RelayListModelTest::showsFoldersAsRootAndFilesAsLeaves() {
    QTemporaryDir temporaryDir;
    QVERIFY2(temporaryDir.isValid(), "Temporary directory must be created for relay model tests");

    QDir rootDir(temporaryDir.path());
    QVERIFY(rootDir.mkpath("AXICOM"));
    QVERIFY(rootDir.mkpath("TEC"));
    QVERIFY(rootDir.mkpath("Self Test"));
    QVERIFY(rootDir.mkpath("Invalid"));

    writeTextFile(rootDir.filePath("relay_schema.json"), QByteArray(kMinimalRelaySchema));
    writeTextFile(rootDir.filePath("AXICOM/v23062-B0029.json"), QByteArray(kValidRelayJson));
    writeTextFile(rootDir.filePath("TEC/1804820000.json"), QByteArray(kValidRelayJson));
    writeTextFile(rootDir.filePath("Self Test/manual_board.json"), QByteArray(kValidRelayJson));

    const QString invalidFilePath = rootDir.filePath("Invalid/bad.json");
    QTest::ignoreMessage(
        QtWarningMsg,
        qPrintable(QStringLiteral("Skipping invalid relay JSON \"%1\": missing required property $.relayModel")
                       .arg(QDir::toNativeSeparators(invalidFilePath))));
    writeTextFile(invalidFilePath, QByteArray(kInvalidRelayJson));

    relayListModel model(temporaryDir.path());

    QCOMPARE(model.columnCount(QModelIndex()), 1);
    QCOMPARE(model.rowCount(QModelIndex()), 3);

    const QModelIndex axicomIndex = model.index(0, 0, QModelIndex());
    const QModelIndex selfTestIndex = model.index(1, 0, QModelIndex());
    const QModelIndex tecIndex = model.index(2, 0, QModelIndex());

    QCOMPARE(model.data(axicomIndex, Qt::DisplayRole).toString(), QStringLiteral("AXICOM"));
    QCOMPARE(model.data(selfTestIndex, Qt::DisplayRole).toString(), QStringLiteral("Self Test"));
    QCOMPARE(model.data(tecIndex, Qt::DisplayRole).toString(), QStringLiteral("TEC"));

    QVERIFY(!model.isFile(axicomIndex));
    QCOMPARE(model.parent(axicomIndex), QModelIndex());
    QCOMPARE(model.rowCount(axicomIndex), 1);

    const QModelIndex axicomRelayIndex = model.index(0, 0, axicomIndex);
    QCOMPARE(model.data(axicomRelayIndex, Qt::DisplayRole).toString(), QStringLiteral("v23062-B0029"));
    QVERIFY(model.isFile(axicomRelayIndex));
    QCOMPARE(model.parent(axicomRelayIndex), axicomIndex);
    QVERIFY(model.filePath(axicomRelayIndex).endsWith(QStringLiteral("AXICOM/v23062-B0029.json")));

    const QModelIndex selfTestRelayIndex = model.index(0, 0, selfTestIndex);
    QCOMPARE(model.data(selfTestRelayIndex, Qt::DisplayRole).toString(), QStringLiteral("manual_board"));
    QVERIFY(model.isFile(selfTestRelayIndex));

    const QModelIndex tecRelayIndex = model.index(0, 0, tecIndex);
    QCOMPARE(model.data(tecRelayIndex, Qt::DisplayRole).toString(), QStringLiteral("1804820000"));
    QVERIFY(model.isFile(tecRelayIndex));
}

int runRelayListModelTest(int argc, char** argv) {
    RelayListModelTest test;
    return QTest::qExec(&test, argc, argv);
}

#include "test_relaylistmodel.moc"