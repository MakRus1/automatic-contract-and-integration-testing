#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "services/order_service.hpp"
#include "services/user_service.hpp"
#include "services/database.hpp"

using namespace services;
using namespace contracts;

class OrderServiceUnitTest : public ::testing::Test {
protected:
    void SetUp() override {
        database_ = std::make_shared<InMemoryDatabase>();
        userService_ = std::make_shared<UserService>(database_);
        orderService_ = std::make_unique<OrderService>(database_, userService_);
        
        // Создаем тестового пользователя
        test_user_id_ = userService_->createUser("Test User", "test@example.com");
    }

    std::shared_ptr<InMemoryDatabase> database_;
    std::shared_ptr<UserService> userService_;
    std::unique_ptr<OrderService> orderService_;
    int test_user_id_;
};

TEST_F(OrderServiceUnitTest, CreateOrder_ValidData_ReturnsPositiveId) {
    int id = orderService_->createOrder(test_user_id_, "Product A", 100.0);
    EXPECT_GT(id, 0);
}

TEST_F(OrderServiceUnitTest, CreateOrder_NonExistingUser_ReturnsMinusOne) {
    int id = orderService_->createOrder(999, "Product A", 100.0);
    EXPECT_EQ(id, -1);
}

TEST_F(OrderServiceUnitTest, CreateOrder_InactiveUser_ReturnsMinusOne) {
    userService_->deactivateUser(test_user_id_);
    int id = orderService_->createOrder(test_user_id_, "Product A", 100.0);
    EXPECT_EQ(id, -1);
}

TEST_F(OrderServiceUnitTest, CreateOrder_EmptyProductName_ReturnsMinusOne) {
    int id = orderService_->createOrder(test_user_id_, "", 100.0);
    EXPECT_EQ(id, -1);
}

TEST_F(OrderServiceUnitTest, CreateOrder_ZeroAmount_ReturnsMinusOne) {
    int id = orderService_->createOrder(test_user_id_, "Product A", 0);
    EXPECT_EQ(id, -1);
}

TEST_F(OrderServiceUnitTest, CreateOrder_NegativeAmount_ReturnsMinusOne) {
    int id = orderService_->createOrder(test_user_id_, "Product A", -10.0);
    EXPECT_EQ(id, -1);
}

TEST_F(OrderServiceUnitTest, GetOrder_ExistingOrder_ReturnsOrder) {
    int id = orderService_->createOrder(test_user_id_, "Product A", 100.0);
    auto order = orderService_->getOrder(id);
    
    ASSERT_TRUE(order.has_value());
    EXPECT_EQ(order->product_name, "Product A");
    EXPECT_EQ(order->amount, 100.0);
    EXPECT_EQ(order->status, OrderStatus::PENDING);
}

TEST_F(OrderServiceUnitTest, GetOrder_NonExistingOrder_ReturnsNullopt) {
    auto order = orderService_->getOrder(999);
    EXPECT_FALSE(order.has_value());
}

TEST_F(OrderServiceUnitTest, GetUserOrders_ReturnsAllUserOrders) {
    orderService_->createOrder(test_user_id_, "Product A", 100.0);
    orderService_->createOrder(test_user_id_, "Product B", 200.0);
    
    auto orders = orderService_->getUserOrders(test_user_id_);
    EXPECT_EQ(orders.size(), 2);
}

TEST_F(OrderServiceUnitTest, UpdateOrderStatus_ExistingOrder_ReturnsTrue) {
    int id = orderService_->createOrder(test_user_id_, "Product A", 100.0);
    bool result = orderService_->updateOrderStatus(id, OrderStatus::CONFIRMED);
    
    EXPECT_TRUE(result);
    auto order = orderService_->getOrder(id);
    EXPECT_EQ(order->status, OrderStatus::CONFIRMED);
}

TEST_F(OrderServiceUnitTest, CancelOrder_PendingOrder_ReturnsTrue) {
    int id = orderService_->createOrder(test_user_id_, "Product A", 100.0);
    bool result = orderService_->cancelOrder(id);
    
    EXPECT_TRUE(result);
    auto order = orderService_->getOrder(id);
    EXPECT_EQ(order->status, OrderStatus::CANCELLED);
}

TEST_F(OrderServiceUnitTest, CancelOrder_ShippedOrder_ReturnsFalse) {
    int id = orderService_->createOrder(test_user_id_, "Product A", 100.0);
    orderService_->updateOrderStatus(id, OrderStatus::SHIPPED);
    
    bool result = orderService_->cancelOrder(id);
    EXPECT_FALSE(result);
}

TEST_F(OrderServiceUnitTest, GetTotalAmount_ExcludesCancelledOrders) {
    orderService_->createOrder(test_user_id_, "Product A", 100.0);
    int id2 = orderService_->createOrder(test_user_id_, "Product B", 200.0);
    orderService_->cancelOrder(id2);
    
    double total = orderService_->getTotalAmount(test_user_id_);
    EXPECT_DOUBLE_EQ(total, 100.0);
}

