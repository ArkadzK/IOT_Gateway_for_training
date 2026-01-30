#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>

// #include "ModbusController.h"

#include "AppService.h"

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);

    qmlRegisterType<AppService>("Backend", 1, 0, "AppService");

    AppService service;

    QQmlApplicationEngine engine;
    engine.rootContext()->setContextProperty("app", &service);

    engine.load(QUrl(QStringLiteral("qrc:/Main.qml")));

    if (engine.rootObjects().isEmpty())
        return -1;

    return app.exec();
}



// int main(int argc, char *argv[])
// {
//     QGuiApplication app(argc, argv);

//     // Регистрируем тип (чтобы QML знал о классе)
//     qmlRegisterType<ModbusController>("ModbusMaster", 1, 0, "ModbusController");

//     // СОЗДАЁМ ОБЪЕКТ
//     ModbusController modbusController;

//     QQmlApplicationEngine engine;

//     // ПЕРЕДАЁМ В QML
//     engine.rootContext()->setContextProperty("modbusController", &modbusController);

//     engine.load(QUrl(QStringLiteral("qrc:/Main.qml")));

//     if (engine.rootObjects().isEmpty())
//         return -1;

//     return app.exec();
// }



// int main(int argc, char *argv[])
// {
//     QGuiApplication app(argc, argv);

//     // Разобраться, без этого не работает
//     qmlRegisterUncreatableType<ModbusController>(
//                     "Modbus",        // module
//                     1, 0,            // version
//                     "ModbusController",
//                     "Created in C++ only"
//                     );

//     ModbusController controller;

//     QQmlApplicationEngine engine;

//     ModbusController modbusController;
//     engine.rootContext()->setContextProperty(
//                             "modbusController",
//                             &modbusController
//                             );

//     engine.loadFromModule("ModbusMaster", "Main");

//     if (engine.rootObjects().isEmpty())
//         return -1;

//     return app.exec();
// }
