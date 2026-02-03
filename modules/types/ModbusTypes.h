#ifndef __MODBUSTYPES_H__
#define __MODBUSTYPES_H__

#include <QObject>

class ModbusTypes {
    Q_GADGET
public:
    enum ConnectionState {
        Disconnected,
        Connecting,
        Connected,
        Error
    };
    Q_ENUM(ConnectionState)
};

#endif // __MODBUSTYPES_H__
