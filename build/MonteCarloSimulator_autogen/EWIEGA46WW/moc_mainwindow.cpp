/****************************************************************************
** Meta object code from reading C++ file 'mainwindow.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.15.13)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "../../../mainwindow.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'mainwindow.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.15.13. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_MainWindow_t {
    QByteArrayData data[17];
    char stringdata0[232];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_MainWindow_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_MainWindow_t qt_meta_stringdata_MainWindow = {
    {
QT_MOC_LITERAL(0, 0, 10), // "MainWindow"
QT_MOC_LITERAL(1, 11, 23), // "onSimulateButtonClicked"
QT_MOC_LITERAL(2, 35, 0), // ""
QT_MOC_LITERAL(3, 36, 27), // "onMostLikelyCheckBoxToggled"
QT_MOC_LITERAL(4, 64, 7), // "checked"
QT_MOC_LITERAL(5, 72, 18), // "onSelectionChanged"
QT_MOC_LITERAL(6, 91, 17), // "onMouseMoveInPlot"
QT_MOC_LITERAL(7, 109, 12), // "QMouseEvent*"
QT_MOC_LITERAL(8, 122, 5), // "event"
QT_MOC_LITERAL(9, 128, 19), // "onLegendDoubleClick"
QT_MOC_LITERAL(10, 148, 10), // "QCPLegend*"
QT_MOC_LITERAL(11, 159, 6), // "legend"
QT_MOC_LITERAL(12, 166, 22), // "QCPAbstractLegendItem*"
QT_MOC_LITERAL(13, 189, 4), // "item"
QT_MOC_LITERAL(14, 194, 19), // "onXAxisRangeChanged"
QT_MOC_LITERAL(15, 214, 8), // "QCPRange"
QT_MOC_LITERAL(16, 223, 8) // "newRange"

    },
    "MainWindow\0onSimulateButtonClicked\0\0"
    "onMostLikelyCheckBoxToggled\0checked\0"
    "onSelectionChanged\0onMouseMoveInPlot\0"
    "QMouseEvent*\0event\0onLegendDoubleClick\0"
    "QCPLegend*\0legend\0QCPAbstractLegendItem*\0"
    "item\0onXAxisRangeChanged\0QCPRange\0"
    "newRange"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_MainWindow[] = {

 // content:
       8,       // revision
       0,       // classname
       0,    0, // classinfo
       6,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: name, argc, parameters, tag, flags
       1,    0,   44,    2, 0x08 /* Private */,
       3,    1,   45,    2, 0x08 /* Private */,
       5,    0,   48,    2, 0x08 /* Private */,
       6,    1,   49,    2, 0x08 /* Private */,
       9,    2,   52,    2, 0x08 /* Private */,
      14,    1,   57,    2, 0x08 /* Private */,

 // slots: parameters
    QMetaType::Void,
    QMetaType::Void, QMetaType::Bool,    4,
    QMetaType::Void,
    QMetaType::Void, 0x80000000 | 7,    8,
    QMetaType::Void, 0x80000000 | 10, 0x80000000 | 12,   11,   13,
    QMetaType::Void, 0x80000000 | 15,   16,

       0        // eod
};

void MainWindow::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<MainWindow *>(_o);
        (void)_t;
        switch (_id) {
        case 0: _t->onSimulateButtonClicked(); break;
        case 1: _t->onMostLikelyCheckBoxToggled((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 2: _t->onSelectionChanged(); break;
        case 3: _t->onMouseMoveInPlot((*reinterpret_cast< QMouseEvent*(*)>(_a[1]))); break;
        case 4: _t->onLegendDoubleClick((*reinterpret_cast< QCPLegend*(*)>(_a[1])),(*reinterpret_cast< QCPAbstractLegendItem*(*)>(_a[2]))); break;
        case 5: _t->onXAxisRangeChanged((*reinterpret_cast< const QCPRange(*)>(_a[1]))); break;
        default: ;
        }
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        switch (_id) {
        default: *reinterpret_cast<int*>(_a[0]) = -1; break;
        case 4:
            switch (*reinterpret_cast<int*>(_a[1])) {
            default: *reinterpret_cast<int*>(_a[0]) = -1; break;
            case 1:
                *reinterpret_cast<int*>(_a[0]) = qRegisterMetaType< QCPAbstractLegendItem* >(); break;
            case 0:
                *reinterpret_cast<int*>(_a[0]) = qRegisterMetaType< QCPLegend* >(); break;
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
        if (_id < 6)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 6;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 6)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 6;
    }
    return _id;
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
