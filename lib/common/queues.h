#pragma once

/**
 * @file queues.h
 * @brief Typed FreeRTOS queue wrappers (RAII handles).
 */

#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>
#include <cstdint>
#include <memory>

namespace loralink {

/**
 * @class QueueHandle
 * @brief RAII wrapper around FreeRTOS queue.
 */
template<typename T>
class QueueHandle {
public:
    explicit QueueHandle(size_t capacity) {
        m_queue = xQueueCreate(capacity, sizeof(T));
    }

    ~QueueHandle() {
        if (m_queue) vQueueDelete(m_queue);
    }

    QueueHandle(const QueueHandle&) = delete;
    QueueHandle& operator=(const QueueHandle&) = delete;

    bool send(const T& item, uint32_t timeout_ms = 0) const {
        return m_queue && xQueueSend(m_queue, &item, pdMS_TO_TICKS(timeout_ms)) == pdTRUE;
    }

    bool trySend(const T& item) const { return send(item, 0); }

    bool receive(T& item, uint32_t timeout_ms = portMAX_DELAY) const {
        return m_queue && xQueueReceive(m_queue, &item, pdMS_TO_TICKS(timeout_ms)) == pdTRUE;
    }

    bool tryReceive(T& item) const { return receive(item, 0); }

    size_t waiting() const {
        return m_queue ? uxQueueMessagesWaiting(m_queue) : 0;
    }

    bool valid() const { return m_queue != nullptr; }
    QueueHandle_t raw() const { return m_queue; }

private:
    QueueHandle_t m_queue{nullptr};
};

} // namespace loralink
