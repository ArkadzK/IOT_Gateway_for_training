#ifndef __MODBUSCONTROLLER_H__
#define __MODBUSCONTROLLER_H__

#include <QObject>
// #include <QString>
#include <QTimer>
// Проверить установку пакетов Qt Serial Bus и Qt Serial Port (без последнего не соберётся!)
#include <QtSerialBus/QModbusTcpClient>
// #include <QtSerialBus/QModbusClient>
#include <QtSerialBus/QModbusDevice>

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

    Q_INVOKABLE void connectToServer(const QString &host, int port, int unitId);
    Q_INVOKABLE void disconnectFromServer();

    Q_INVOKABLE void readHoldingRegisters(int startAddress, int count);
    Q_INVOKABLE void writeHoldingRegister(int address, int value);

signals:
    void stateChanged();
    void logMessage(const QString &message);

    void holdingRegistersRead(int startAddress, const QVector<quint16> &values); // Регистры 16бит

private slots:
    void onStateChanged(QModbusDevice::State state);
    void onErrorOccurred(QModbusDevice::Error error);

private:
    void setState(ConnectionState newState);
    void log(const QString &text);

    ConnectionState m_state = Disconnected;
    QModbusTcpClient *m_client = nullptr;

    int m_unitId = 1;
};

#endif // __MODBUSCONTROLLER_H__
