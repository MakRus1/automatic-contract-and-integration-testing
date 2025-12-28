#include "services/database.hpp"
#include <algorithm>

namespace services {

int InMemoryDatabase::saveUser(const contracts::User& user) {
    std::lock_guard<std::mutex> lock(mutex_);
    contracts::User new_user = user;
    new_user.id = next_user_id_++;
    users_[new_user.id] = new_user;
    return new_user.id;
}

std::optional<contracts::User> InMemoryDatabase::findUserById(int id) const {
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = users_.find(id);
    if (it != users_.end()) {
        return it->second;
    }
    return std::nullopt;
}

std::vector<contracts::User> InMemoryDatabase::findAllUsers() const {
    std::lock_guard<std::mutex> lock(mutex_);
    std::vector<contracts::User> result;
    result.reserve(users_.size());
    for (const auto& [id, user] : users_) {
        result.push_back(user);
    }
    return result;
}

bool InMemoryDatabase::updateUser(const contracts::User& user) {
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = users_.find(user.id);
    if (it != users_.end()) {
        it->second = user;
        return true;
    }
    return false;
}

bool InMemoryDatabase::deleteUser(int id) {
    std::lock_guard<std::mutex> lock(mutex_);
    return users_.erase(id) > 0;
}

int InMemoryDatabase::saveOrder(const contracts::Order& order) {
    std::lock_guard<std::mutex> lock(mutex_);
    contracts::Order new_order = order;
    new_order.id = next_order_id_++;
    orders_[new_order.id] = new_order;
    return new_order.id;
}

std::optional<contracts::Order> InMemoryDatabase::findOrderById(int id) const {
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = orders_.find(id);
    if (it != orders_.end()) {
        return it->second;
    }
    return std::nullopt;
}

std::vector<contracts::Order> InMemoryDatabase::findOrdersByUserId(int user_id) const {
    std::lock_guard<std::mutex> lock(mutex_);
    std::vector<contracts::Order> result;
    for (const auto& [id, order] : orders_) {
        if (order.user_id == user_id) {
            result.push_back(order);
        }
    }
    return result;
}

std::vector<contracts::Order> InMemoryDatabase::findAllOrders() const {
    std::lock_guard<std::mutex> lock(mutex_);
    std::vector<contracts::Order> result;
    result.reserve(orders_.size());
    for (const auto& [id, order] : orders_) {
        result.push_back(order);
    }
    return result;
}

bool InMemoryDatabase::updateOrder(const contracts::Order& order) {
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = orders_.find(order.id);
    if (it != orders_.end()) {
        it->second = order;
        return true;
    }
    return false;
}

bool InMemoryDatabase::deleteOrder(int id) {
    std::lock_guard<std::mutex> lock(mutex_);
    return orders_.erase(id) > 0;
}

void InMemoryDatabase::clear() {
    std::lock_guard<std::mutex> lock(mutex_);
    users_.clear();
    orders_.clear();
    next_user_id_ = 1;
    next_order_id_ = 1;
}

} // namespace services

