#include <QtTest>
#include <spdy_common.h>

char test_control_frame[] = {
  0x80, 0x03, 0xAA, 0x55,
  0xCC, 0x01, 0x02, 0x03,
  0x01, 0x02, 0x03, 0x04,
  0x05, 0x06, 0x07, 0x08
};

char test_data_frame[] = {
  0x01, 0x03, 0xAA, 0x55,
  0xCC, 0x01, 0x02, 0x03,
  0x08, 0x07, 0x06, 0x05,
  0x04, 0x03, 0x02, 0x01,
};

class SpdyStructTest : public QObject
{
  Q_OBJECT;
public:
  SpdyStructTest(QObject *parent = 0) : QObject(parent) { }

private slots:
  void doTestStruct()
  {
    SPDY_CONTROL_FRAME *control = (SPDY_CONTROL_FRAME *) test_control_frame;
    SPDY_DATA_FRAME    *data    = (SPDY_DATA_FRAME *) test_data_frame;

    QVERIFY(control->control == 1);  
    QVERIFY(VERSION(control) == 3);  
    QVERIFY(control->flag    == 0xCC);  
    QVERIFY(TYPE(control)    == 0xAA55);  
    QVERIFY(LENGTH(control)  == 0x010203);  

    QVERIFY(data->control    == 0);
    QVERIFY(STREAMID(data)   == 0x0103AA55);
    QVERIFY(data->flag       == 0xCC);  
    QVERIFY(LENGTH(data)     == 0x010203);  
  }
};

int main(int argc, char **argv)
{
  QCoreApplication app(argc, argv);

  SpdyStructTest structTest;
  QTest::qExec(&structTest, argc, argv);

  return 0;
}

#include "unittest.moc"
