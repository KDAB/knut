#include "core/uidocument.h"

#include "common/test_utils.h"

#include <QTest>

class TestUiDocument : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase() { Q_INIT_RESOURCE(core); }

    void load()
    {
        Core::UiDocument document;
        document.load(Test::testDataPath() + QStringLiteral("/uidocument/IDD_ABCCOMPILE.ui"));

        const auto widgets = document.widgets();
        QCOMPARE(widgets.count(), 21);
        auto rootWidget = widgets.first();
        QCOMPARE(rootWidget->name(), "IDD_ABCCOMPILE");
        QCOMPARE(rootWidget->className(), "QDialog");
        QCOMPARE(rootWidget->isRoot(), true);

        auto widget = document.findWidget("IDC_RADIO_YUP");
        QCOMPARE(widget->name(), "IDC_RADIO_YUP");
        QCOMPARE(widget->className(), "QRadioButton");
        QCOMPARE(widget->isRoot(), false);

        widget = document.findWidget("NOT_EXIST");
        QCOMPARE(widget, nullptr);
    }

    void save()
    {
        Test::FileTester file(Test::testDataPath() + "/uidocument/IDD_LIGHTING_original.ui");
        Core::UiDocument document;
        document.load(file.fileName());

        const auto widgets = document.widgets();
        QCOMPARE(widgets.count(), 50);

        auto root = widgets.first();
        root->setClassName("QWidget");
        root->setName("Lighting");

        auto widget = document.findWidget("IDC_LIGHTING_SUNDIR_EDIT");
        widget->setClassName("QPushButton");
        widget->setName("LightingSundirEdit");

        document.save();

        QVERIFY(file.compare());
    }
};

QTEST_MAIN(TestUiDocument)
#include "tst_uidocument.moc"
