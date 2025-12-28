#pragma once

#include "contracts/database_contract.hpp"
#include <unordered_map>
#include <mutex>

namespace services {

/**
 * @brief In-memory реализация базы данных
 * 
 * Простая реализация для демонстрации. В реальном проекте
 * здесь была бы работа с настоящей БД.
 */
class InMemoryDatabase : public contracts::IDatabase {
public:
    InMemoryDatabase() = default;

    // Операции с пользователями
    int saveUser(const contracts::User& user) override;
    std::optional<contracts::User> findUserById(int id) const override;
    std::vector<contracts::User> findAllUsers() const override;
    bool updateUser(const contracts::User& user) override;
    bool deleteUser(int id) override;

    // Операции с заказами
    int saveOrder(const contracts::Order& order) override;
    std::optional<contracts::Order> findOrderById(int id) const override;
    std::vector<contracts::Order> findOrdersByUserId(int user_id) const override;
    std::vector<contracts::Order> findAllOrders() const override;
    bool updateOrder(const contracts::Order& order) override;
    bool deleteOrder(int id) override;

    // Служебные методы
    void clear() override;

private:
    mutable std::mutex mutex_;
    std::unordered_map<int, contracts::User> users_;
    std::unordered_map<int, contracts::Order> orders_;
    int next_user_id_ = 1;
    int next_order_id_ = 1;
};

} // namespace services

