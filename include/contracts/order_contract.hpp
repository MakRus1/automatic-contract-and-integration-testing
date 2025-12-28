#pragma once

#include <string>
#include <optional>
#include <vector>
#include <chrono>

namespace contracts {

/**
 * @brief Статус заказа
 */
enum class OrderStatus {
    PENDING,
    CONFIRMED,
    SHIPPED,
    DELIVERED,
    CANCELLED
};

/**
 * @brief Структура данных заказа - часть контракта
 */
struct Order {
    int id;
    int user_id;
    std::string product_name;
    double amount;
    OrderStatus status;

    bool operator==(const Order& other) const {
        return id == other.id &&
               user_id == other.user_id &&
               product_name == other.product_name &&
               amount == other.amount &&
               status == other.status;
    }
};

/**
 * @brief Интерфейс (контракт) сервиса заказов
 * 
 * Определяет контракт для работы с заказами.
 * Важно: заказ может быть создан только для существующего пользователя.
 */
class IOrderService {
public:
    virtual ~IOrderService() = default;

    /**
     * @brief Создать новый заказ
     * @param user_id ID пользователя (должен существовать и быть активным)
     * @param product_name Название продукта (не должно быть пустым)
     * @param amount Сумма заказа (должна быть > 0)
     * @return ID созданного заказа или -1 при ошибке
     */
    virtual int createOrder(int user_id, const std::string& product_name, double amount) = 0;

    /**
     * @brief Получить заказ по ID
     * @param id ID заказа
     * @return Заказ или nullopt если не найден
     */
    virtual std::optional<Order> getOrder(int id) const = 0;

    /**
     * @brief Получить все заказы пользователя
     * @param user_id ID пользователя
     * @return Список заказов пользователя
     */
    virtual std::vector<Order> getUserOrders(int user_id) const = 0;

    /**
     * @brief Обновить статус заказа
     * @param id ID заказа
     * @param status Новый статус
     * @return true если успешно, false если заказ не найден
     */
    virtual bool updateOrderStatus(int id, OrderStatus status) = 0;

    /**
     * @brief Отменить заказ
     * @param id ID заказа
     * @return true если успешно отменен, false если невозможно отменить
     */
    virtual bool cancelOrder(int id) = 0;

    /**
     * @brief Получить общую сумму заказов пользователя
     * @param user_id ID пользователя
     * @return Сумма всех заказов пользователя
     */
    virtual double getTotalAmount(int user_id) const = 0;
};

} // namespace contracts

