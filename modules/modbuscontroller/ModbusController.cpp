#include <QDebug>
#include <QVariant>

#include <QModbusDataUnit>

#include "modbuscontroller.h"

#include <mqtt/async_client.h>

#include <QJsonArray>
#include <QJsonObject>
#include <QJsonDocument>

ModbusController::ModbusController(QObject *parent) : QObject(parent)
{
    m_client = new QModbusTcpClient(this);

    connect(m_client, &QModbusClient::stateChanged,
            this, &ModbusController::onStateChanged);

    connect(m_client, &QModbusClient::errorOccurred,
            this, &ModbusController::onErrorOccurred);

    // MQTT: пока ничего не создаём — создадим при первом подключении
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

            QJsonArray arr;
            for (quint16 v : values)
                arr.append(static_cast<int>(v));

            QJsonObject obj;
            obj["type"] = "holding_registers";
            obj["start"] = startAddress;
            obj["values"] = arr;

            QJsonDocument doc(obj);
            mqttPublish("modbus/holding", QString::fromUtf8(doc.toJson(QJsonDocument::Compact)), 1);

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

                    emit coilsRead(startAddress, values);

                    QJsonArray arr;
                    for (bool v : values)
                        arr.append(v);

                    QJsonObject obj;
                    obj["type"] = "coils";
                    obj["start"] = startAddress;
                    obj["values"] = arr;

                    QJsonDocument doc(obj);
                    mqttPublish("modbus/coils", QString::fromUtf8(doc.toJson(QJsonDocument::Compact)), 1);  //QoS = 1

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

void ModbusController::mqttConnect(const QString &host, int port)
{
    const std::string server = host.toStdString() + ":" + std::to_string(port);

    try {
        if (!m_mqttClient) {
            m_mqttClient = std::make_unique<mqtt::async_client>(server, "iotgateway-client");

            m_mqttConnOpts = mqtt::connect_options_builder()
                                 .clean_session(true)
                                 // .automatic_reconnect(true, std::chrono::seconds(1), std::chrono::seconds(30))
                                 .finalize();
            m_mqttConnOpts.set_automatic_reconnect(true);
        }

        log(QString("MQTT connecting to %1:%2").arg(host).arg(port));

        auto tok = m_mqttClient->connect(m_mqttConnOpts);
        tok->wait();

        log("MQTT connected");
    }
    catch (const mqtt::exception &ex) {
        log(QString("MQTT connect error: %1").arg(ex.what()));
    }
}

void ModbusController::mqttDisconnect()
{
    if (!m_mqttClient) {
        log("MQTT disconnect: client not created");
        return;
    }

    try {
        log("MQTT disconnect requested");
        auto tok = m_mqttClient->disconnect();
        tok->wait();
        log("MQTT disconnected");
    }
    catch (const mqtt::exception &ex) {
        log(QString("MQTT disconnect error: %1").arg(ex.what()));
    }
}

void ModbusController::mqttPublish(const QString &topic,
                                   const QString &payload,
                                   int qos,
                                   bool retain)
{
    if (!m_mqttClient || !m_mqttClient->is_connected()) {
        log("MQTT publish failed: not connected");
        return;
    }

    try {
        auto msg = mqtt::make_message(topic.toStdString(),
                                      payload.toStdString(),
                                      qos,
                                      retain);

        m_mqttClient->publish(msg)->wait();

        log(QString("MQTT published to %1: %2")
                .arg(topic, payload));
    }
    catch (const mqtt::exception &ex) {
        log(QString("MQTT publish error: %1").arg(ex.what()));
    }
}
