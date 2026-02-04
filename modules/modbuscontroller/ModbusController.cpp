#include <QDebug>
#include <QVariant>

#include <QModbusDataUnit>

#include "modbuscontroller.h"

#include <QJsonArray>
#include <QJsonObject>
#include <QJsonDocument>

ModbusController::ModbusController (QObject *parent) : QObject(parent)
{
    m_client = new QModbusTcpClient(this);

    connect(m_client, &QModbusClient::stateChanged,
            this, &ModbusController::onStateChanged);

    connect(m_client, &QModbusClient::errorOccurred,
            this, &ModbusController::onErrorOccurred);
}

ModbusTypes::ConnectionState ModbusController::state() const
{
    return m_state;
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

    setState(ModbusTypes::Connecting);
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
        setState(ModbusTypes::Connecting);
        break;
    case QModbusDevice::ConnectedState:
        log("Connection established");
        setState(ModbusTypes::Connected);
        break;
    case QModbusDevice::ClosingState:
        log("Closing connection...");
        break;
    case QModbusDevice::UnconnectedState:
        log("Disconnected");
        setState(ModbusTypes::Disconnected);
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
    setState(ModbusTypes::Error);
}

void ModbusController::setState(ModbusTypes::ConnectionState newState)
{
    if (m_state == newState)
        return;

    m_state = newState;
    emit stateChanged(newState);
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

            QJsonArray arr;
            for (quint16 v : values)
                arr.append(static_cast<int>(v));

            QJsonObject obj;
            obj["type"] = "holding_registers";
            obj["start"] = startAddress;
            obj["values"] = arr;

            QJsonDocument doc(obj);

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

void ModbusController::readCoils(int startAddress, int count)
{
    if (!m_client || m_client->state() != QModbusDevice::ConnectedState) {
        log("Cannot read coils: not connected");
        return;
    }

    if (count <= 0) {
        log("Cannot read coils: count must be > 0");
        return;
    }

    QModbusDataUnit request(QModbusDataUnit::Coils, startAddress, count);

    auto *reply = m_client->sendReadRequest(request, m_unitId);
    if (!reply) {
        log("Read coils request failed to send");
        return;
    }

    if (!reply->isFinished()) {
        connect(reply, &QModbusReply::finished, this,
                [this, reply, startAddress]() {
                    reply->deleteLater();

                    if (reply->error() != QModbusDevice::NoError) {
                        log("Read coils error: " + reply->errorString());
                        return;
                    }

                    const QModbusDataUnit unit = reply->result();
                    QVector<bool> values;
                    values.reserve(unit.valueCount());

                    for (uint i = 0; i < unit.valueCount(); ++i)
                        values.append(unit.value(i));

                    log(QString("Read %1 coils from %2")
                            .arg(unit.valueCount())
                            .arg(startAddress));

                    emit coilsRead(startAddress, values);

                    QJsonArray arr;
                    for (bool v : values)
                        arr.append(v);

                    QJsonObject obj;
                    obj["type"] = "coils";
                    obj["start"] = startAddress;
                    obj["values"] = arr;

                    QJsonDocument doc(obj);
                });
    } else {
        reply->deleteLater();
    }
}

void ModbusController::writeSingleCoil(int address, bool value)
{
    if (!m_client || m_client->state() != QModbusDevice::ConnectedState) {
        log("Cannot write coil: not connected");
        return;
    }

    if (address < 0) {
        log("Cannot write coil: invalid address");
        return;
    }

    QModbusDataUnit request(QModbusDataUnit::Coils, address, 1);
    request.setValue(0, value);

    auto *reply = m_client->sendWriteRequest(request, m_unitId);
    if (!reply) {
        log("Write coil request failed to send");
        return;
    }

    if (!reply->isFinished()) {
        connect(reply, &QModbusReply::finished, this,
                [this, reply, address, value]() {
                    reply->deleteLater();

                    if (reply->error() != QModbusDevice::NoError) {
                        log("Write coil error: " + reply->errorString());
                        return;
                    }

                    log(QString("Wrote coil %1 = %2")
                            .arg(address)
                            .arg(value));

                    emit coilWritten(address, value);
                });
    } else {
        reply->deleteLater();
    }
}

void ModbusController::writeMultipleCoils(int startAddress, const QVector<bool> &values)
{
    if (!m_client || m_client->state() != QModbusDevice::ConnectedState) {
        log("Cannot write coils: not connected");
        return;
    }

    if (values.isEmpty()) {
        log("Cannot write coils: values list is empty");
        return;
    }

    QModbusDataUnit request(QModbusDataUnit::Coils, startAddress, values.size());

    for (int i = 0; i < values.size(); ++i)
        request.setValue(i, values[i]);

    auto *reply = m_client->sendWriteRequest(request, m_unitId);
    if (!reply) {
        log("Write multiple coils request failed to send");
        return;
    }

    if (!reply->isFinished()) {
        connect(reply, &QModbusReply::finished, this,
                [this, reply, startAddress, values]() {
                    reply->deleteLater();

                    if (reply->error() != QModbusDevice::NoError) {
                        log("Write multiple coils error: " + reply->errorString());
                        return;
                    }

                    log(QString("Wrote %1 coils starting at %2")
                            .arg(values.size())
                            .arg(startAddress));

                    emit multipleCoilsWritten(startAddress, values.size());
                });
    } else {
        reply->deleteLater();
    }
}
