#ifndef __MESSAGEQUEUE_H__
#define __MESSAGEQUEUE_H__

#include <QString>
#include <QMutex>
#include <QWaitCondition>
#include <QList>
#include <QUuid>
#include <QDateTime>

// Структура сообщения, передаваемого в MQTT
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

    // Добавить новое сообщение
    void push(const MqttPacket& packet);

    // Блокирующее извлечение
    void waitAndPop(MqttPacket& packet);

    // Вернуть сообщение обратно (например, после ошибки отправки)
    void returnBack(const MqttPacket& packet);

    // Текущий размер очереди
    int size() const;

    // В будущем — включение persistence
    void enablePersistence(const QString& path);
    void saveToDisk();
    void loadFromDisk();

private:
    mutable QMutex m_mutex;
    QWaitCondition m_wait;
    QList<MqttPacket> m_queue;

    // persistence
    bool m_persistenceEnabled = false;
    QString m_persistPath;
};

#endif // __MESSAGEQUEUE_H__
