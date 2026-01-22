#include "modbuscontroller.h"
#include <QDebug>
#include <QVariant>

ModbusController::ModbusController(QObject *parent) : QObject(parent)
{
    m_client = new QModbusTcpClient(this);

    connect(m_client, &QModbusClient::stateChanged,
            this, &ModbusController::onStateChanged);

    connect(m_client, &QModbusClient::errorOccurred,
            this, &ModbusController::onErrorOccurred);
}

void ModbusController::connectToServer(const QString &host, int port, int unitId)
{
    if (host.isEmpty() || port <= 0 || port > 65535) {
        log("Invalid host or port");
        return;
    }

    m_unitId = unitId;

    log(QString("Connect request: %1:%2 (unit %3)")
        .arg(host).arg(port).arg(unitId));

    if (m_client->state() != QModbusDevice::UnconnectedState)
        m_client->disconnectDevice();

    m_client->setConnectionParameter(QModbusDevice::NetworkAddressParameter, host);
    m_client->setConnectionParameter(QModbusDevice::NetworkPortParameter, port);
    m_client->setTimeout(3000);
    m_client->setNumberOfRetries(3);

    setState(Connecting);
    m_client->connectDevice();
}

void ModbusController::disconnectFromServer()
{
    log("Disconnect requested");
    m_client->disconnectDevice();
}

void ModbusController::onStateChanged(QModbusDevice::State state)
{
    switch (state) {
    case QModbusDevice::ConnectingState:
        log("Connecting...");
        setState(Connecting);
        break;
    case QModbusDevice::ConnectedState:
        log("Connection established");
        setState(Connected);
        break;
    case QModbusDevice::ClosingState:
        log("Closing connection...");
        break;
    case QModbusDevice::UnconnectedState:
        log("Disconnected");
        setState(Disconnected);
        break;
    default:
        break;
    }
}

void ModbusController::onErrorOccurred(QModbusDevice::Error error)
{
    if (error == QModbusDevice::NoError)
        return;

    log("Connection error: " + m_client->errorString());
    setState(Error);
}

void ModbusController::setState(ConnectionState newState)
{
    if (m_state == newState)
        return;

    m_state = newState;
    emit stateChanged();
}

void ModbusController::log(const QString &text)
{
    emit logMessage(text);
}
