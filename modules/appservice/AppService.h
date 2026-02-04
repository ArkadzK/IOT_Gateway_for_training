#ifndef __APPSERVICE_H__
#define __APPSERVICE_H__

#include <QObject>
#include <memory>

#include "ModbusController.h"
#include "MqttWorker.h"
#include "MessageQueue.h"

#include "ModbusTypes.h"

class AppService : public QObject
{
    Q_OBJECT

    // Modbus state
    Q_PROPERTY(ModbusTypes::ConnectionState state READ state NOTIFY stateChanged)

    // MQTT state exposed to QML
    Q_PROPERTY(bool mqttConnected READ mqttConnected NOTIFY mqttConnectedChanged)

public:
    explicit AppService(QObject *parent = nullptr);

    // Getter for QML
    bool mqttConnected() const { return m_mqttConnected; }

    // Modbus API
    Q_INVOKABLE void connectModbus(const QString &host, int port, int unitId);
    Q_INVOKABLE void disconnectModbus();
    Q_INVOKABLE void readRegisters(int start, int count);
    Q_INVOKABLE void writeRegister(int address, int value);
    Q_INVOKABLE void readCoils(int start, int count);
    Q_INVOKABLE void writeCoil(int address, bool value);

    ModbusTypes::ConnectionState state() const {
        return m_modbus->state(); }

    // MQTT API
    Q_INVOKABLE void connectMqtt(const QString &host, int port, int qos);
    Q_INVOKABLE void disconnectMqtt();

signals:
    // Signals exposed to QML
    void logMessage(const QString &msg);
    void registersUpdated(int start, const QVector<quint16> &values);
    void coilsUpdated(int start, const QVector<bool> &values);

    // Modbus state
    void stateChanged(ModbusTypes::ConnectionState newState);
    // MQTT state changed
    void mqttConnectedChanged();

private slots:
    void onRegisters(int start, const QVector<quint16>& values);
    void onCoils(int start, const QVector<bool>& values);

private:
    ModbusController* m_modbus = nullptr;
    MessageQueue m_queue;
    std::unique_ptr<MqttWorker> m_mqtt;

    bool m_mqttConnected = false;
};

#endif // __APPSERVICE_H__
