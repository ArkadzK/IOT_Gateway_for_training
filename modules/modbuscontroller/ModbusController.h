#ifndef __MODBUSCONTROLLER_H__
#define __MODBUSCONTROLLER_H__

#include <QObject>
// #include <QString>
#include <QTimer>
// Проверить установку пакетов Qt Serial Bus и Qt Serial Port (без последнего не соберётся!)
#include <QtSerialBus/QModbusTcpClient>
// #include <QtSerialBus/QModbusClient>
#include <QtSerialBus/QModbusDevice>
#include <QMqttClient>

class ModbusController : public QObject
{
    Q_OBJECT

public:
    enum ConnectionState {
        Disconnected,
        Connecting,
        Connected,
        Error
    };
    Q_ENUM(ConnectionState)

    Q_PROPERTY(ConnectionState state READ state NOTIFY stateChanged)

    ConnectionState state() const { return m_state; }

    explicit ModbusController(QObject *parent = nullptr);
    // TCP/IP connection
    Q_INVOKABLE void connectToServer(const QString &host, int port, int unitId);
    Q_INVOKABLE void disconnectFromServer();
    // Holding registers
    Q_INVOKABLE void readHoldingRegisters(int startAddress, int count);
    Q_INVOKABLE void writeHoldingRegister(int address, int value);
    // Coils
    Q_INVOKABLE void readCoils(int startAddress, int count);
    Q_INVOKABLE void writeSingleCoil(int address, bool value);
    Q_INVOKABLE void writeMultipleCoils(int startAddress, const QVector<bool>& values);
    // MQTT-client
    Q_INVOKABLE void mqttConnect(const QString &host, int port);
    Q_INVOKABLE void mqttDisconnect();
    Q_INVOKABLE void mqttPublish(const QString &topic,
                                 const QString &payload,
                                 int qos = 0,
                                 bool retain = false);

signals:
    void stateChanged();
    void logMessage(const QString &message);

    void holdingRegistersRead(int startAddress, const QVector<quint16> &values); // Регистры 16бит

    void coilsRead(int startAddress, const QVector<bool>& values);
    void coilWritten(int address, bool value);
    void multipleCoilsWritten(int startAddress, int count);

private slots:
    void onStateChanged(QModbusDevice::State state);
    void onErrorOccurred(QModbusDevice::Error error);

private:
    void setState(ConnectionState newState);
    void log(const QString &text);    
    QMqttClient *m_mqtt = nullptr;

    ConnectionState m_state = Disconnected;
    QModbusTcpClient *m_client = nullptr;

    int m_unitId = 1;
};

#endif // __MODBUSCONTROLLER_H__
