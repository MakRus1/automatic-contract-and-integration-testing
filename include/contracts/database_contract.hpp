#pragma once

#include "user_contract.hpp"
#include "order_contract.hpp"
#include <optional>
#include <vector>

namespace contracts {

/**
 * @brief Интерфейс (контракт) базы данных
 * 
 * Абстрагирует хранение данных. Позволяет легко подменять
 * реальную БД на mock для тестирования.
 */
class IDatabase {
public:
    virtual ~IDatabase() = default;

    // Операции с пользователями
    virtual int saveUser(const User& user) = 0;
    virtual std::optional<User> findUserById(int id) const = 0;
    virtual std::vector<User> findAllUsers() const = 0;
    virtual bool updateUser(const User& user) = 0;
    virtual bool deleteUser(int id) = 0;

    // Операции с заказами
    virtual int saveOrder(const Order& order) = 0;
    virtual std::optional<Order> findOrderById(int id) const = 0;
    virtual std::vector<Order> findOrdersByUserId(int user_id) const = 0;
    virtual std::vector<Order> findAllOrders() const = 0;
    virtual bool updateOrder(const Order& order) = 0;
    virtual bool deleteOrder(int id) = 0;

    // Служебные методы
    virtual void clear() = 0;
};

} // namespace contracts

