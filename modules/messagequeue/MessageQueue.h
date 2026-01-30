#ifndef __MESSAGEQUEUE_H__
#define __MESSAGEQUEUE_H__

#include <QString>
#include <QMutex>
#include <QWaitCondition>
#include <QList>
#include <QUuid>
#include <QDateTime>

struct MqttPacket
{
    QString id;            // уникальный идентификатор
    qint64 timestamp;      // время создания (ms since epoch)
    QString topic;
    QString payload;
    int retryCount = 0;    // количество попыток отправки

    MqttPacket() = default;

    MqttPacket(const QString& t, const QString& p)
        : id(QUuid::createUuid().toString(QUuid::WithoutBraces)),
        timestamp(QDateTime::currentMSecsSinceEpoch()),
        topic(t),
        payload(p)
    {}
};

class MessageQueue
{
public:
    MessageQueue() = default;

    // Add new msg
    void push(const MqttPacket& packet);

    // Blocking extraction
    bool waitAndPop(MqttPacket& packet);

    // Revert a message (for example, after a sending error)
    void returnBack(const MqttPacket& packet);

    // Current queue size
    int size() const;
    // wake up all wait()
    void stop();
    void reset(); // clears the queue and removes stop

    // В будущем — включение persistence
    void enablePersistence(const QString& path);
    void saveToDisk();
    void loadFromDisk();

private:
    mutable QMutex m_mutex;
    QWaitCondition m_wait;
    QList<MqttPacket> m_queue;

    bool m_stopped = false;

    // persistence
    bool m_persistenceEnabled = false;
    QString m_persistPath;
};

#endif // __MESSAGEQUEUE_H__
