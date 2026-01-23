#include <QDebug>
#include <QVariant>

#include <QModbusDataUnit>

#include "modbuscontroller.h"


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

void ModbusController::readHoldingRegisters(int startAddress, int count)
{
    // Клиент подключен?
    if (!m_client || m_client->state() != QModbusDevice::ConnectedState) {
        log("Cannot read: not connected");
        return;
    }

    if (count <= 0) {
        log("Cannot read: count must be > 0");
        return;
    }

    QModbusDataUnit request(QModbusDataUnit::HoldingRegisters, startAddress, count);    // Настраиваем запрос

    auto *reply = m_client->sendReadRequest(request, m_unitId); // Возвращает асинхронный ответ
    if (!reply) {
        log("Read request failed to send");
        return;
    }
    // Обработка результата и логирование
    if (!reply->isFinished()) {
        connect(reply, &QModbusReply::finished, this, [this, reply, startAddress]() {
            reply->deleteLater();

            if (reply->error() != QModbusDevice::NoError) {
                log("Read error: " + reply->errorString());
                return;
            }

            const QModbusDataUnit unit = reply->result();
            QVector<quint16> values;
            values.reserve(unit.valueCount());

            for (uint i = 0; i < unit.valueCount(); ++i) {
                values.append(unit.value(i));
            }

            log(QString("Read %1 holding registers from %2")
                    .arg(unit.valueCount())
                    .arg(startAddress));

            emit holdingRegistersRead(startAddress, values);
        });
    } else {
        // Синхронный ответ (редко, но по API надо обработать) - от Copilot
        reply->deleteLater();
    }
}

// Запись одного регистра
void ModbusController::writeHoldingRegister(int address, int value)
{
    if (!m_client || m_client->state() != QModbusDevice::ConnectedState) {
        log("Cannot write: not connected");
        return;
    }

    if (address < 0) {
        log("Cannot write: invalid address");
        return;
    }

    QModbusDataUnit request(QModbusDataUnit::HoldingRegisters, address, 1);
    request.setValue(0, static_cast<quint16>(value));

    auto *reply = m_client->sendWriteRequest(request, m_unitId);    // Асинхронная запись
    if (!reply) {
        log("Write request failed to send");
        return;
    }

    if (!reply->isFinished()) {
        connect(reply, &QModbusReply::finished, this, [this, reply, address, value]() {
            reply->deleteLater();

            if (reply->error() != QModbusDevice::NoError) {
                log("Write error: " + reply->errorString());
                return;
            }

            log(QString("Wrote value %1 to holding register %2")
                    .arg(value)
                    .arg(address));
        });
    } else {
        reply->deleteLater();
    }
}

