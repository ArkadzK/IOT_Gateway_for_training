#ifndef __APPSERVICE_H__
#define __APPSERVICE_H__

#include <QObject>

#include <memory>

#include "ModbusController.h"
#include "MqttWorker.h"
#include "MessageQueue.h"

class AppService : public QObject
{
    Q_OBJECT

public:
    explicit AppService(QObject *parent = nullptr);

    // --- Modbus API ---
    Q_INVOKABLE void connectModbus(const QString &host, int port, int unitId);
    Q_INVOKABLE void disconnectModbus();
    Q_INVOKABLE void readRegisters(int start, int count);
    Q_INVOKABLE void writeRegister(int address, int value);
    Q_INVOKABLE void readCoils(int start, int count);
    Q_INVOKABLE void writeCoil(int address, bool value);

    // --- MQTT API ---
    Q_INVOKABLE void connectMqtt(const QString &host, int port, int qos);
    Q_INVOKABLE void disconnectMqtt();
    Q_INVOKABLE void publish(const QString &topic, const QString &payload);

signals:
    // Пробрасываем сигналы Modbus наружу
    void logMessage(const QString &msg);
    void registersUpdated(int start, const QVector<quint16> &values);
    void coilsUpdated(int start, const QVector<bool> &values);

private:
    ModbusController m_modbus;
    MessageQueue m_queue;

    std::unique_ptr<MqttWorker> m_mqtt;
};

#endif // __APPSERVICE_H__
