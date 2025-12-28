#include "services/order_service.hpp"
#include <numeric>

namespace services {

OrderService::OrderService(std::shared_ptr<contracts::IDatabase> database,
                           std::shared_ptr<contracts::IUserService> userService)
    : database_(std::move(database)), userService_(std::move(userService)) {}

int OrderService::createOrder(int user_id, const std::string& product_name, double amount) {
    // Проверка контракта: пользователь должен существовать
    auto user = userService_->getUser(user_id);
    if (!user.has_value()) {
        return -1;
    }

    // Проверка контракта: пользователь должен быть активен
    if (!user->is_active) {
        return -1;
    }

    // Проверка контракта: название продукта и сумма должны быть валидны
    if (!isValidProductName(product_name) || !isValidAmount(amount)) {
        return -1;
    }

    contracts::Order order;
    order.id = 0;  // Будет присвоен базой данных
    order.user_id = user_id;
    order.product_name = product_name;
    order.amount = amount;
    order.status = contracts::OrderStatus::PENDING;

    return database_->saveOrder(order);
}

std::optional<contracts::Order> OrderService::getOrder(int id) const {
    return database_->findOrderById(id);
}

std::vector<contracts::Order> OrderService::getUserOrders(int user_id) const {
    return database_->findOrdersByUserId(user_id);
}

bool OrderService::updateOrderStatus(int id, contracts::OrderStatus status) {
    auto order = database_->findOrderById(id);
    if (!order.has_value()) {
        return false;
    }

    order->status = status;
    return database_->updateOrder(*order);
}

bool OrderService::cancelOrder(int id) {
    auto order = database_->findOrderById(id);
    if (!order.has_value()) {
        return false;
    }

    // Контракт: можно отменить только заказы в определенных статусах
    if (!canCancel(order->status)) {
        return false;
    }

    order->status = contracts::OrderStatus::CANCELLED;
    return database_->updateOrder(*order);
}

double OrderService::getTotalAmount(int user_id) const {
    auto orders = database_->findOrdersByUserId(user_id);
    
    return std::accumulate(orders.begin(), orders.end(), 0.0,
        [](double sum, const contracts::Order& order) {
            // Не учитываем отмененные заказы
            if (order.status != contracts::OrderStatus::CANCELLED) {
                return sum + order.amount;
            }
            return sum;
        });
}

bool OrderService::isValidProductName(const std::string& name) const {
    // Контракт: название продукта не должно быть пустым
    return !name.empty();
}

bool OrderService::isValidAmount(double amount) const {
    // Контракт: сумма должна быть больше 0
    return amount > 0;
}

bool OrderService::canCancel(contracts::OrderStatus status) const {
    // Контракт: можно отменить только ожидающие или подтвержденные заказы
    return status == contracts::OrderStatus::PENDING || 
           status == contracts::OrderStatus::CONFIRMED;
}

} // namespace services

