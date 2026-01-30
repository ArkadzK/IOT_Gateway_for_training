#include "MessageQueue.h"
#include <QFile>
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonDocument>

void MessageQueue::push(const MqttPacket& packet)
{
    QMutexLocker locker(&m_mutex);

    if (m_stopped)
        return;
    m_queue.append(packet);
    m_wait.wakeOne();
}

bool MessageQueue::waitAndPop(MqttPacket& packet)
{
    QMutexLocker locker(&m_mutex);

    while (m_queue.isEmpty() && !m_stopped)
        m_wait.wait(&m_mutex);

    if (m_stopped)
        return false;
    packet = m_queue.takeFirst();
    return true;
}

void MessageQueue::returnBack(const MqttPacket& packet)
{
    QMutexLocker locker(&m_mutex);

    if (m_stopped)
        return;

    m_queue.prepend(packet);
    m_wait.wakeOne();
}

int MessageQueue::size() const
{
    QMutexLocker locker(&m_mutex);
    return m_queue.size();
}

void MessageQueue::stop()
{
    QMutexLocker locker(&m_mutex);
    m_stopped = true;
    m_wait.wakeAll();
}

void MessageQueue::reset()
{
    QMutexLocker locker(&m_mutex);
    m_stopped = false;
    m_queue.clear();
    m_wait.wakeAll();
}

void MessageQueue::enablePersistence(const QString& path)
{
    QMutexLocker locker(&m_mutex);
    m_persistenceEnabled = true;
    m_persistPath = path;
}

void MessageQueue::saveToDisk()
{
    if (!m_persistenceEnabled)
        return;

    QMutexLocker locker(&m_mutex);

    QJsonArray arr;
    for (const auto& p : m_queue)
    {
        QJsonObject obj;
        obj["id"] = p.id;
        obj["timestamp"] = QString::number(p.timestamp);
        obj["topic"] = p.topic;
        obj["payload"] = p.payload;
        obj["retryCount"] = p.retryCount;
        arr.append(obj);
    }

    QJsonDocument doc(arr);
    QFile file(m_persistPath);
    if (file.open(QIODevice::WriteOnly))
        file.write(doc.toJson());
}

void MessageQueue::loadFromDisk()
{
    if (!m_persistenceEnabled)
        return;

    QMutexLocker locker(&m_mutex);

    QFile file(m_persistPath);
    if (!file.open(QIODevice::ReadOnly))
        return;

    QByteArray data = file.readAll();
    QJsonDocument doc = QJsonDocument::fromJson(data);
    if (!doc.isArray())
        return;

    QJsonArray arr = doc.array();
    for (const auto& v : arr)
    {
        QJsonObject obj = v.toObject();
        MqttPacket p;
        p.id = obj["id"].toString();
        p.timestamp = obj["timestamp"].toString().toLongLong();
        p.topic = obj["topic"].toString();
        p.payload = obj["payload"].toString();
        p.retryCount = obj["retryCount"].toInt();
        m_queue.append(p);
    }
}
