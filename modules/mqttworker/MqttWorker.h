#ifndef __MQTTWORKER_H__
#define __MQTTWORKER_H__

#include <QObject>
#include <QString>
#include <QThread>
#include <QAtomicInt>
#include <QMutex>

#include "MessageQueue.h"
#include <mqtt/async_client.h>

class MqttWorker : public QThread
{
    Q_OBJECT

signals:
    void logMessage(const QString &msg);

public:
    MqttWorker(const QString& host,
               const QString& clientId,
               int qos,
               MessageQueue* queue,
               QObject* parent = nullptr);

    ~MqttWorker() override;

    void stop();
    void reset();
protected:
    void run() override;

private:
    void connectToBroker();
    void publishPacket(const MqttPacket& packet);

private:
    QString m_host;
    QString m_clientId;

    mqtt::async_client* m_client = nullptr;
    mqtt::connect_options m_connOpts;

    MessageQueue* m_queue = nullptr;

    int m_qos = 0;

    QAtomicInt m_running { true };
    QMutex m_mutex;
    // bool m_resetting = false;
};

#endif // __MQTTWORKER_H__
