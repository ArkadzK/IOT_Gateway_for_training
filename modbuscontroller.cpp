#include "modbuscontroller.h"
#include <QDebug>

ModbusController::ModbusController(QObject *parent) : QObject(parent)
{
    m_connectTimer.setSingleShot(true);
    m_connectTimer.setInterval(2000); // 2 секунды

    connect(&m_connectTimer, &QTimer::timeout,
            this, &ModbusController::onConnectTimeout);
}

ModbusController::ConnectionState ModbusController::state() const
{
    return m_state;
}

void ModbusController::setState(ConnectionState newState)
{
    if (m_state == newState)
        return;

    m_state = newState;
    emit stateChanged();

    qDebug() << "State changed to" << m_state;
}

void ModbusController::connectToServer(const QString &host, int port, int unitId)
{
    if(m_state != Disconnected && m_state != Error)
        return;

    log(QString("Connect request: %1:%2 (unit %3)")
            .arg(host)
            .arg(port)
            .arg(unitId));

    qDebug() << "Connecting to" << host << port << "unit" << unitId;

    setState(Connecting);

    // Temporary: позже будет реальный Modbus connect
    m_connectTimer.start();
}

void ModbusController::disconnectFromServer()
{
    if (m_state == Disconnected)
        return;

    m_connectTimer.stop();

    log("Connection failed");
    qDebug() << "Disconnected";

    setState(Disconnected);
}

void ModbusController::onConnectTimeout()
{
    // ВРЕМЕННО: имитация результата
    bool success = true; // попробуй false

    if (success) {
        log("Connection established");
        qDebug() << "Connected successfully";

        setState(Connected);
    } else {
        log("Connection failed");
        qDebug() << "Connection failed";

        setState(Error);
    }
}

void ModbusController::log(const QString &text)
{
    emit logMessage(text);
}
