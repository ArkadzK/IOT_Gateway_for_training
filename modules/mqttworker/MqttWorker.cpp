#include "MqttWorker.h"
#include <QDebug>

static const int RETRY_LIMIT = 3;

MqttWorker::MqttWorker(const QString& host,
                       const QString& clientId,
                       int qos,
                       MessageQueue* queue,
                       QObject* parent)
    : QThread(parent),
    m_host(host),
    m_clientId(clientId),
    m_qos(qos),
    m_queue(queue)
{
    m_client = new mqtt::async_client(host.toStdString(),
                                      clientId.toStdString());

    mqtt::connect_options_builder builder;
    builder.clean_session(true);
    m_connOpts = builder.finalize();
}

MqttWorker::~MqttWorker()
{
    stop();
    wait();

    QMutexLocker locker(&m_mutex);
    if (m_client) {
        try {
            m_client->disconnect()->wait();
        } catch (...) {
        }
        delete m_client;
        m_client = nullptr; }
}

void MqttWorker::stop()
{
    QMutexLocker locker(&m_mutex);

    m_running.storeRelease(false);

    // Будим очередь, чтобы поток вышел из waitAndPop()
    if (m_queue)
        m_queue->stop();
}

void MqttWorker::reset()
{
    QMutexLocker locker(&m_mutex);

    // 1. Останавливаем run()
    m_running.storeRelease(false);
    if (m_queue)
        m_queue->stop();

    locker.unlock();

    // 2. Дожидаемся завершения потока
    wait();
    locker.relock();

    // 3. Безопасно уничтожаем MQTT‑клиент
    if (m_client) {
        try {
            m_client->disconnect()->wait();
        } catch (...) {
            // игнорируем ошибки
        }
        delete m_client;
        m_client = nullptr;
    }

    // 4. Сбрасываем очередь (очищаем и снимаем stop)
    if (m_queue)
        m_queue->reset();

    // 5. Создаём новый MQTT‑клиент и опции
    m_client = new mqtt::async_client(m_host.toStdString(),
                                      m_clientId.toStdString());

    mqtt::connect_options_builder builder;
    builder.clean_session(true);
    m_connOpts = builder.finalize();

    // 6. Готовы к новому запуску
    m_running.storeRelease(true);
}

void MqttWorker::run()
{
    connectToBroker();

    while (m_running.loadAcquire())
    {
        MqttPacket packet;

        // waitAndPop теперь возвращает false, если очередь остановлена
        if (!m_queue->waitAndPop(packet)) {
            // если нас остановили — выходим
            if (!m_running.loadAcquire())
                break;
            // иначе просто продолжаем цикл (например, после reset())
            continue;
        }

        publishPacket(packet);
    }
}

void MqttWorker::connectToBroker()
{
    while (m_running.loadAcquire())
    {
        try {
            qDebug() << "MQTT: connecting to broker...";
            m_client->connect(m_connOpts)->wait();
            qDebug() << "MQTT: connected";
            emit logMessage("Connected to MQTT broker");
            return;
        }
        catch (const mqtt::exception& e) {
            qDebug() << "MQTT: connection failed:" << e.what();
            emit logMessage(QString("MQTT connection failed: %1").arg(e.what()));
            QThread::sleep(2);
        }
    }
}

void MqttWorker::publishPacket(const MqttPacket& packet)
{
    try {
        mqtt::message_ptr msg = mqtt::make_message(
            packet.topic.toStdString(),
            packet.payload.toStdString(),
            m_qos,
            false
            );

        m_client->publish(msg)->wait();
        qDebug() << "MQTT: sent" << packet.topic << packet.payload;
        emit logMessage(QString("MQTT published: %1 = %2")
                        .arg(packet.topic, packet.payload));
    }
    catch (const mqtt::exception& e) {
        qDebug() << "MQTT: publish failed:" << e.what();

        if (packet.retryCount < RETRY_LIMIT)
        {
            MqttPacket retry = packet;
            retry.retryCount++;
            m_queue->returnBack(retry);
            emit logMessage(QString("MQTT retry %1 for topic %2")
                            .arg(packet.retryCount)
                            .arg(packet.topic));
        }
        else {
            qDebug() << "MQTT: retry limit reached, dropping packet";
            emit logMessage("MQTT: retry limit reached, dropping packet");
        }

        connectToBroker();
        emit logMessage("MQTT reconnected, retrying publish...");
    }
}
