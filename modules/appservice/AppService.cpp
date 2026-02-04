#include "AppService.h"
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonDocument>

AppService::AppService(QObject *parent) : QObject(parent)
{
    m_modbus = new ModbusController(this);

    // Пробрасываем сигналы Modbus наружу
    connect(m_modbus, &ModbusController::logMessage,
            this, &AppService::logMessage);
    // connect(m_modbus, &ModbusController::ConnectionState,
    //         this, &AppService::stateChanged);
    connect(m_modbus, &ModbusController::holdingRegistersRead,
            this, &AppService::onRegisters);
    connect(m_modbus, &ModbusController::coilsRead,
            this, &AppService::onCoils);
    connect(m_modbus, &ModbusController::stateChanged,
            this, &AppService::stateChanged);
}

// MODBUS
void AppService::connectModbus(const QString &host, int port, int unitId)
{
    m_modbus->connectToServer(host, port, unitId);
}

void AppService::disconnectModbus()
{
    m_modbus->disconnectFromServer();
}

void AppService::readRegisters(int start, int count)
{
    m_modbus->readHoldingRegisters(start, count);
}

void AppService::writeRegister(int address, int value)
{
    m_modbus->writeHoldingRegister(address, value);
}

void AppService::readCoils(int start, int count)
{
    m_modbus->readCoils(start, count);
}

void AppService::writeCoil(int address, bool value)
{
    m_modbus->writeSingleCoil(address, value);
}

// MQTT
void AppService::connectMqtt(const QString &host, int port, int qos)
{
    if (m_mqtt) {
        m_mqtt->stop();
        m_mqtt->reset();
    }

    QString clientId = "QtClient_" + QString::number(QDateTime::currentMSecsSinceEpoch());

    m_mqtt = std::make_unique<MqttWorker>(
        host,
        clientId,
        qos,
        &m_queue,
        this
        );

    // Push LOG
    connect(m_mqtt.get(), &MqttWorker::logMessage,
            this, &AppService::logMessage);

    m_queue.reset();
    m_mqtt->start();

    m_mqttConnected = true;
    emit mqttConnectedChanged();
}

void AppService::disconnectMqtt()
{
    if (m_mqtt) {
        m_mqtt->stop();
        m_mqtt->reset();
    }

    m_mqttConnected = false;
    emit mqttConnectedChanged();
}

// ROUTING
void AppService::onRegisters(int start, const QVector<quint16>& values)
{
    emit registersUpdated(start, values);

    QJsonArray arr;
    for (auto v : values)
        arr.append(int(v));

    QJsonObject obj;
    obj["type"] = "holding_registers";
    obj["start"] = start;
    obj["values"] = arr;

    QJsonDocument doc(obj);

    m_queue.push(MqttPacket("modbus/holding",
                            doc.toJson(QJsonDocument::Compact)));
}

void AppService::onCoils(int start, const QVector<bool>& values)
{
    emit coilsUpdated(start, values);

    QJsonArray arr;
    for (bool v : values)
        arr.append(v);

    QJsonObject obj;
    obj["type"] = "coils";
    obj["start"] = start;
    obj["values"] = arr;

    QJsonDocument doc(obj);

    m_queue.push(MqttPacket("modbus/coils",
                            doc.toJson(QJsonDocument::Compact)));
}
