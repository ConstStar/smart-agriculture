/****************************************************************************
** Meta object code from reading C++ file 'mainwindow.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.15.2)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "../../mainwindow.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'mainwindow.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.15.2. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_MainWindow_t {
    QByteArrayData data[18];
    char stringdata0[349];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_MainWindow_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_MainWindow_t qt_meta_stringdata_MainWindow = {
    {
QT_MOC_LITERAL(0, 0, 10), // "MainWindow"
QT_MOC_LITERAL(1, 11, 15), // "onConnectedMQTT"
QT_MOC_LITERAL(2, 27, 0), // ""
QT_MOC_LITERAL(3, 28, 18), // "onDisconnectedMQTT"
QT_MOC_LITERAL(4, 47, 18), // "onDataReceivedMQTT"
QT_MOC_LITERAL(5, 66, 14), // "QMQTT::Message"
QT_MOC_LITERAL(6, 81, 3), // "msg"
QT_MOC_LITERAL(7, 85, 33), // "onClickListWidgetRelayNameCli..."
QT_MOC_LITERAL(8, 119, 16), // "QListWidgetItem*"
QT_MOC_LITERAL(9, 136, 4), // "item"
QT_MOC_LITERAL(10, 141, 25), // "onClicktableWidgetTrigger"
QT_MOC_LITERAL(11, 167, 17), // "QTableWidgetItem*"
QT_MOC_LITERAL(12, 185, 26), // "onClickPushButtonRelayName"
QT_MOC_LITERAL(13, 212, 33), // "onClickPushButtonRelayControl..."
QT_MOC_LITERAL(14, 246, 34), // "onClickPushButtonRelayControl..."
QT_MOC_LITERAL(15, 281, 30), // "onClickPushButtonTriggerUpdate"
QT_MOC_LITERAL(16, 312, 28), // "onClickPushButtonTriggerEdit"
QT_MOC_LITERAL(17, 341, 7) // "onTimer"

    },
    "MainWindow\0onConnectedMQTT\0\0"
    "onDisconnectedMQTT\0onDataReceivedMQTT\0"
    "QMQTT::Message\0msg\0onClickListWidgetRelayNameClicked\0"
    "QListWidgetItem*\0item\0onClicktableWidgetTrigger\0"
    "QTableWidgetItem*\0onClickPushButtonRelayName\0"
    "onClickPushButtonRelayControlOpen\0"
    "onClickPushButtonRelayControlClose\0"
    "onClickPushButtonTriggerUpdate\0"
    "onClickPushButtonTriggerEdit\0onTimer"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_MainWindow[] = {

 // content:
       8,       // revision
       0,       // classname
       0,    0, // classinfo
      11,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: name, argc, parameters, tag, flags
       1,    0,   69,    2, 0x08 /* Private */,
       3,    0,   70,    2, 0x08 /* Private */,
       4,    1,   71,    2, 0x08 /* Private */,
       7,    1,   74,    2, 0x08 /* Private */,
      10,    1,   77,    2, 0x08 /* Private */,
      12,    0,   80,    2, 0x08 /* Private */,
      13,    0,   81,    2, 0x08 /* Private */,
      14,    0,   82,    2, 0x08 /* Private */,
      15,    0,   83,    2, 0x08 /* Private */,
      16,    0,   84,    2, 0x08 /* Private */,
      17,    0,   85,    2, 0x08 /* Private */,

 // slots: parameters
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, 0x80000000 | 5,    6,
    QMetaType::Void, 0x80000000 | 8,    9,
    QMetaType::Void, 0x80000000 | 11,    9,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,

       0        // eod
};

void MainWindow::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<MainWindow *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->onConnectedMQTT(); break;
        case 1: _t->onDisconnectedMQTT(); break;
        case 2: _t->onDataReceivedMQTT((*reinterpret_cast< const QMQTT::Message(*)>(_a[1]))); break;
        case 3: _t->onClickListWidgetRelayNameClicked((*reinterpret_cast< QListWidgetItem*(*)>(_a[1]))); break;
        case 4: _t->onClicktableWidgetTrigger((*reinterpret_cast< QTableWidgetItem*(*)>(_a[1]))); break;
        case 5: _t->onClickPushButtonRelayName(); break;
        case 6: _t->onClickPushButtonRelayControlOpen(); break;
        case 7: _t->onClickPushButtonRelayControlClose(); break;
        case 8: _t->onClickPushButtonTriggerUpdate(); break;
        case 9: _t->onClickPushButtonTriggerEdit(); break;
        case 10: _t->onTimer(); break;
        default: ;
        }
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        switch (_id) {
        default: *reinterpret_cast<int*>(_a[0]) = -1; break;
        case 2:
            switch (*reinterpret_cast<int*>(_a[1])) {
            default: *reinterpret_cast<int*>(_a[0]) = -1; break;
            case 0:
                *reinterpret_cast<int*>(_a[0]) = qRegisterMetaType< QMQTT::Message >(); break;
            }
            break;
        }
    }
}

QT_INIT_METAOBJECT const QMetaObject MainWindow::staticMetaObject = { {
    QMetaObject::SuperData::link<QMainWindow::staticMetaObject>(),
    qt_meta_stringdata_MainWindow.data,
    qt_meta_data_MainWindow,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *MainWindow::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *MainWindow::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_MainWindow.stringdata0))
        return static_cast<void*>(this);
    return QMainWindow::qt_metacast(_clname);
}

int MainWindow::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QMainWindow::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 11)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 11;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 11)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 11;
    }
    return _id;
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
