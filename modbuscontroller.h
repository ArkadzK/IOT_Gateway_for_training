#ifndef __MODBUSCONTROLLER_H__
#define __MODBUSCONTROLLER_H__

#include <QObject>
#include <QString>
#include <QTimer>

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

    explicit ModbusController(QObject *parent = nullptr);

    ConnectionState state() const;

    Q_INVOKABLE void connectToServer(
        const QString &host,
        int port,
        int unitId
        );

    Q_INVOKABLE void disconnectFromServer();

signals:
    void stateChanged();
    void logMessage(const QString &message);

private:
    void setState(ConnectionState newState);
    void onConnectTimeout();
    void log(const QString &text);

private:
    ConnectionState m_state = Disconnected;
    QTimer m_connectTimer;
};

#endif // __MODBUSCONTROLLER_H__
