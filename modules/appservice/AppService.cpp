#include "AppService.h"

AppService::AppService(QObject *parent)
    : QObject(parent)
{
    // Пробрасываем сигналы Modbus наружу
    connect(&m_modbus, &ModbusController::logMessage,
            this, &AppService::logMessage);

    connect(&m_modbus, &ModbusController::holdingRegistersRead,
            this, &AppService::registersUpdated);

    connect(&m_modbus, &ModbusController::coilsRead,
            this, &AppService::coilsUpdated);

    // Пробрасываем MQTT‑лог - удалено, т.к. m_mqtt создаётся в AppService::connectMqtt(...)
    // connect(m_mqtt.get(), &MqttWorker::logMessage,
            // this, &AppService::logMessage);
}

// -------------------- MODBUS --------------------

void AppService::connectModbus(const QString &host, int port, int unitId)
{
    m_modbus.connectToServer(host, port, unitId);
}

void AppService::disconnectModbus()
{
    m_modbus.disconnectFromServer();
}

void AppService::readRegisters(int start, int count)
{
    m_modbus.readHoldingRegisters(start, count);
}

void AppService::writeRegister(int address, int value)
{
    m_modbus.writeHoldingRegister(address, value);
}

void AppService::readCoils(int start, int count)
{
    m_modbus.readCoils(start, count);
}

void AppService::writeCoil(int address, bool value)
{
    m_modbus.writeSingleCoil(address, value);
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
        &m_queue,
        this
        );

    // Push LOG
    connect(m_mqtt.get(), &MqttWorker::logMessage,
            this, &AppService::logMessage);

    m_mqtt->start();
}

void AppService::disconnectMqtt()
{
    if (m_mqtt) {
        m_mqtt->stop();
        m_mqtt->reset();
    }
}

void AppService::publish(const QString &topic, const QString &payload)
{
    MqttPacket packet;
    packet.topic = topic;
    packet.payload = payload;
    packet.retryCount = 0;

    m_queue.push(packet);
}
