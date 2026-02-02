#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>

#include "AppService.h"

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);

    AppService service;

    QQmlApplicationEngine engine;

    engine.rootContext()->setContextProperty("app", &service);

    engine.load(QUrl(QStringLiteral("qrc:/Main.qml")));

    if (engine.rootObjects().isEmpty())
        return -1;

    return app.exec();
}
