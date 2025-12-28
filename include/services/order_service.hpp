#pragma once

#include "contracts/order_contract.hpp"
#include "contracts/user_contract.hpp"
#include "contracts/database_contract.hpp"
#include <memory>

namespace services {

/**
 * @brief Реализация сервиса заказов
 * 
 * Реализует контракт IOrderService. Зависит от IUserService
 * для проверки существования пользователя при создании заказа.
 */
class OrderService : public contracts::IOrderService {
public:
    OrderService(std::shared_ptr<contracts::IDatabase> database,
                 std::shared_ptr<contracts::IUserService> userService);

    int createOrder(int user_id, const std::string& product_name, double amount) override;
    std::optional<contracts::Order> getOrder(int id) const override;
    std::vector<contracts::Order> getUserOrders(int user_id) const override;
    bool updateOrderStatus(int id, contracts::OrderStatus status) override;
    bool cancelOrder(int id) override;
    double getTotalAmount(int user_id) const override;

private:
    std::shared_ptr<contracts::IDatabase> database_;
    std::shared_ptr<contracts::IUserService> userService_;

    // Валидация согласно контракту
    bool isValidProductName(const std::string& name) const;
    bool isValidAmount(double amount) const;
    bool canCancel(contracts::OrderStatus status) const;
};

} // namespace services

