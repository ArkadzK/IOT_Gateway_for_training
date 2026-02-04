#ifndef __MODBUSCONTROLLER_H__
#define __MODBUSCONTROLLER_H__

#include <QObject>
#include <QTimer>

// Проверить установку пакетов Qt Serial Bus и Qt Serial Port (без последнего не соберётся!)
#include <QtSerialBus/QModbusTcpClient>
#include <QtSerialBus/QModbusDevice>

#include "ModbusTypes.h"

class ModbusController : public QObject
{
    Q_OBJECT

public:
    Q_PROPERTY(ModbusTypes::ConnectionState state READ state NOTIFY stateChanged)

    explicit ModbusController(QObject *parent = nullptr);

    ModbusTypes::ConnectionState state() const;

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

signals:
    void stateChanged(ModbusTypes::ConnectionState state);
    void logMessage(const QString &message);

    void holdingRegistersRead(int startAddress, const QVector<quint16> &values); // Регистры 16бит
    void coilsRead(int startAddress, const QVector<bool>& values);
    void coilWritten(int address, bool value);
    void multipleCoilsWritten(int startAddress, int count);

private slots:
    void onStateChanged(QModbusDevice::State state);
    void onErrorOccurred(QModbusDevice::Error error);

private:
    void setState(ModbusTypes::ConnectionState newState);
    void log(const QString &text);

    // Modbus
    ModbusTypes::ConnectionState m_state = ModbusTypes::Disconnected;
    QModbusTcpClient *m_client = nullptr;

    int m_unitId = 1;
};

#endif // __MODBUSCONTROLLER_H__
