/**
 * @file order_contract_test.cpp
 * @brief Контрактные тесты для IOrderService
 * 
 * Эти тесты проверяют контракт сервиса заказов.
 * Ключевой аспект: заказ можно создать только для
 * существующего активного пользователя.
 */

#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "contracts/order_contract.hpp"
#include "contracts/user_contract.hpp"
#include "services/order_service.hpp"
#include "services/user_service.hpp"
#include "services/database.hpp"

using namespace services;
using namespace contracts;

class OrderContractTest : public ::testing::Test {
protected:
    void SetUp() override {
        database_ = std::make_shared<InMemoryDatabase>();
        userService_ = std::make_shared<UserService>(database_);
        orderService_ = std::make_unique<OrderService>(database_, userService_);
        
        // Создаем активного пользователя для тестов
        active_user_id_ = userService_->createUser("Active User", "active@test.com");
        
        // Создаем неактивного пользователя
        inactive_user_id_ = userService_->createUser("Inactive User", "inactive@test.com");
        userService_->deactivateUser(inactive_user_id_);
    }

    std::shared_ptr<InMemoryDatabase> database_;
    std::shared_ptr<IUserService> userService_;
    std::unique_ptr<IOrderService> orderService_;  // Используем интерфейс!
    int active_user_id_;
    int inactive_user_id_;
};

// ============================================================================
// КОНТРАКТ: createOrder
// Предусловие: user_id существует и активен, product_name не пустое, amount > 0
// Постусловие: положительный ID при успехе, -1 при нарушении
// ============================================================================

TEST_F(OrderContractTest, CreateOrder_Contract_ValidInput_ReturnsPositiveId) {
    int result = orderService_->createOrder(active_user_id_, "Product", 100.0);
    
    EXPECT_GT(result, 0) 
        << "CONTRACT VIOLATION: createOrder must return positive ID for valid input";
}

TEST_F(OrderContractTest, CreateOrder_Contract_NonExistingUser_ReturnsMinusOne) {
    int result = orderService_->createOrder(99999, "Product", 100.0);
    
    EXPECT_EQ(result, -1) 
        << "CONTRACT VIOLATION: createOrder must return -1 for non-existing user";
}

TEST_F(OrderContractTest, CreateOrder_Contract_InactiveUser_ReturnsMinusOne) {
    int result = orderService_->createOrder(inactive_user_id_, "Product", 100.0);
    
    EXPECT_EQ(result, -1) 
        << "CONTRACT VIOLATION: createOrder must return -1 for inactive user";
}

TEST_F(OrderContractTest, CreateOrder_Contract_EmptyProductName_ReturnsMinusOne) {
    int result = orderService_->createOrder(active_user_id_, "", 100.0);
    
    EXPECT_EQ(result, -1) 
        << "CONTRACT VIOLATION: createOrder must return -1 for empty product name";
}

TEST_F(OrderContractTest, CreateOrder_Contract_ZeroAmount_ReturnsMinusOne) {
    int result = orderService_->createOrder(active_user_id_, "Product", 0.0);
    
    EXPECT_EQ(result, -1) 
        << "CONTRACT VIOLATION: createOrder must return -1 for zero amount";
}

TEST_F(OrderContractTest, CreateOrder_Contract_NegativeAmount_ReturnsMinusOne) {
    int result = orderService_->createOrder(active_user_id_, "Product", -50.0);
    
    EXPECT_EQ(result, -1) 
        << "CONTRACT VIOLATION: createOrder must return -1 for negative amount";
}

TEST_F(OrderContractTest, CreateOrder_Contract_InitialStatusIsPending) {
    int id = orderService_->createOrder(active_user_id_, "Product", 100.0);
    auto order = orderService_->getOrder(id);
    
    ASSERT_TRUE(order.has_value());
    EXPECT_EQ(order->status, OrderStatus::PENDING) 
        << "CONTRACT VIOLATION: new order must have PENDING status";
}

// ============================================================================
// КОНТРАКТ: getOrder
// Предусловие: нет
// Постусловие: Order если существует, nullopt если нет
// ============================================================================

TEST_F(OrderContractTest, GetOrder_Contract_ExistingOrder_ReturnsOrder) {
    int id = orderService_->createOrder(active_user_id_, "Product", 100.0);
    auto result = orderService_->getOrder(id);
    
    ASSERT_TRUE(result.has_value()) 
        << "CONTRACT VIOLATION: getOrder must return order for existing ID";
    EXPECT_EQ(result->id, id);
}

TEST_F(OrderContractTest, GetOrder_Contract_NonExistingOrder_ReturnsNullopt) {
    auto result = orderService_->getOrder(99999);
    
    EXPECT_FALSE(result.has_value()) 
        << "CONTRACT VIOLATION: getOrder must return nullopt for non-existing ID";
}

TEST_F(OrderContractTest, GetOrder_Contract_DataIntegrity) {
    std::string product = "Test Product";
    double amount = 150.50;
    int id = orderService_->createOrder(active_user_id_, product, amount);
    
    auto order = orderService_->getOrder(id);
    
    ASSERT_TRUE(order.has_value());
    EXPECT_EQ(order->user_id, active_user_id_) 
        << "CONTRACT VIOLATION: order user_id must be preserved";
    EXPECT_EQ(order->product_name, product) 
        << "CONTRACT VIOLATION: order product_name must be preserved";
    EXPECT_DOUBLE_EQ(order->amount, amount) 
        << "CONTRACT VIOLATION: order amount must be preserved";
}

// ============================================================================
// КОНТРАКТ: getUserOrders
// Предусловие: нет
// Постусловие: список заказов пользователя (может быть пустым)
// ============================================================================

TEST_F(OrderContractTest, GetUserOrders_Contract_ReturnsOnlyUserOrders) {
    // Создаем заказы для разных пользователей
    int user2_id = userService_->createUser("User2", "user2@test.com");
    
    orderService_->createOrder(active_user_id_, "Product A", 100.0);
    orderService_->createOrder(active_user_id_, "Product B", 200.0);
    orderService_->createOrder(user2_id, "Product C", 300.0);
    
    auto orders = orderService_->getUserOrders(active_user_id_);
    
    // Все заказы должны принадлежать указанному пользователю
    for (const auto& order : orders) {
        EXPECT_EQ(order.user_id, active_user_id_) 
            << "CONTRACT VIOLATION: getUserOrders must return only orders of specified user";
    }
    EXPECT_EQ(orders.size(), 2);
}

// ============================================================================
// КОНТРАКТ: cancelOrder
// Предусловие: заказ существует
// Постусловие: true если отменен (PENDING/CONFIRMED), false если невозможно
// ============================================================================

TEST_F(OrderContractTest, CancelOrder_Contract_PendingOrder_ReturnsTrue) {
    int id = orderService_->createOrder(active_user_id_, "Product", 100.0);
    
    bool result = orderService_->cancelOrder(id);
    
    EXPECT_TRUE(result) 
        << "CONTRACT VIOLATION: PENDING order must be cancellable";
}

TEST_F(OrderContractTest, CancelOrder_Contract_ConfirmedOrder_ReturnsTrue) {
    int id = orderService_->createOrder(active_user_id_, "Product", 100.0);
    orderService_->updateOrderStatus(id, OrderStatus::CONFIRMED);
    
    bool result = orderService_->cancelOrder(id);
    
    EXPECT_TRUE(result) 
        << "CONTRACT VIOLATION: CONFIRMED order must be cancellable";
}

TEST_F(OrderContractTest, CancelOrder_Contract_ShippedOrder_ReturnsFalse) {
    int id = orderService_->createOrder(active_user_id_, "Product", 100.0);
    orderService_->updateOrderStatus(id, OrderStatus::SHIPPED);
    
    bool result = orderService_->cancelOrder(id);
    
    EXPECT_FALSE(result) 
        << "CONTRACT VIOLATION: SHIPPED order must NOT be cancellable";
}

TEST_F(OrderContractTest, CancelOrder_Contract_DeliveredOrder_ReturnsFalse) {
    int id = orderService_->createOrder(active_user_id_, "Product", 100.0);
    orderService_->updateOrderStatus(id, OrderStatus::DELIVERED);
    
    bool result = orderService_->cancelOrder(id);
    
    EXPECT_FALSE(result) 
        << "CONTRACT VIOLATION: DELIVERED order must NOT be cancellable";
}

TEST_F(OrderContractTest, CancelOrder_Contract_StatusBecomesCancelled) {
    int id = orderService_->createOrder(active_user_id_, "Product", 100.0);
    orderService_->cancelOrder(id);
    
    auto order = orderService_->getOrder(id);
    
    ASSERT_TRUE(order.has_value());
    EXPECT_EQ(order->status, OrderStatus::CANCELLED) 
        << "CONTRACT VIOLATION: cancelled order must have CANCELLED status";
}

// ============================================================================
// КОНТРАКТ: getTotalAmount
// Предусловие: нет
// Постусловие: сумма всех не-отмененных заказов пользователя
// ============================================================================

TEST_F(OrderContractTest, GetTotalAmount_Contract_ExcludesCancelledOrders) {
    orderService_->createOrder(active_user_id_, "Product A", 100.0);
    int id2 = orderService_->createOrder(active_user_id_, "Product B", 200.0);
    orderService_->createOrder(active_user_id_, "Product C", 300.0);
    orderService_->cancelOrder(id2);  // Отменяем заказ на 200
    
    double total = orderService_->getTotalAmount(active_user_id_);
    
    // Должно быть 100 + 300 = 400 (без отмененного 200)
    EXPECT_DOUBLE_EQ(total, 400.0) 
        << "CONTRACT VIOLATION: getTotalAmount must exclude cancelled orders";
}

TEST_F(OrderContractTest, GetTotalAmount_Contract_NoOrders_ReturnsZero) {
    int new_user_id = userService_->createUser("New User", "new@test.com");
    
    double total = orderService_->getTotalAmount(new_user_id);
    
    EXPECT_DOUBLE_EQ(total, 0.0) 
        << "CONTRACT VIOLATION: getTotalAmount must return 0 for user with no orders";
}

// ============================================================================
// КОНТРАКТ МЕЖСЕРВИСНОГО ВЗАИМОДЕЙСТВИЯ
// Проверяем, что OrderService корректно зависит от UserService
// ============================================================================

TEST_F(OrderContractTest, InterServiceContract_OrderRequiresActiveUser) {
    // Сценарий: пользователь деактивируется после создания заказа
    int user_id = userService_->createUser("Temp User", "temp@test.com");
    
    // Можем создать заказ пока пользователь активен
    int order1_id = orderService_->createOrder(user_id, "Product", 100.0);
    EXPECT_GT(order1_id, 0);
    
    // Деактивируем пользователя
    userService_->deactivateUser(user_id);
    
    // Не можем создать новый заказ для неактивного пользователя
    int order2_id = orderService_->createOrder(user_id, "Another Product", 200.0);
    EXPECT_EQ(order2_id, -1) 
        << "CONTRACT VIOLATION: cannot create order for deactivated user";
    
    // Но существующий заказ остается доступным
    auto existing_order = orderService_->getOrder(order1_id);
    EXPECT_TRUE(existing_order.has_value()) 
        << "CONTRACT: existing orders should remain accessible";
}

