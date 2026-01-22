#include "modbuscontroller.h"
#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);

    // Разобраться, без этого не работает
    qmlRegisterUncreatableType<ModbusController>(
                    "Modbus",        // module
                    1, 0,            // version
                    "ModbusController",
                    "Created in C++ only"
                    );

    ModbusController controller;

    QQmlApplicationEngine engine;

    ModbusController modbusController;
    engine.rootContext()->setContextProperty(
                            "modbusController",
                            &modbusController
                            );

    engine.loadFromModule("ModbusMaster", "Main");

    if (engine.rootObjects().isEmpty())
        return -1;

    return app.exec();
}
