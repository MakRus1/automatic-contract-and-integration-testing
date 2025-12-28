/**
 * @file user_order_integration_test.cpp
 * @brief Интеграционные тесты
 * 
 * Эти тесты проверяют корректное взаимодействие всех компонентов системы:
 * - UserService
 * - OrderService  
 * - InMemoryDatabase
 * 
 * В отличие от контрактных тестов, интеграционные тесты проверяют
 * сквозные сценарии использования системы.
 */

#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "services/user_service.hpp"
#include "services/order_service.hpp"
#include "services/database.hpp"
#include <memory>
#include <vector>

using namespace services;
using namespace contracts;

class UserOrderIntegrationTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Собираем полную систему
        database_ = std::make_shared<InMemoryDatabase>();
        userService_ = std::make_shared<UserService>(database_);
        orderService_ = std::make_shared<OrderService>(database_, userService_);
    }

    void TearDown() override {
        database_->clear();
    }

    std::shared_ptr<InMemoryDatabase> database_;
    std::shared_ptr<UserService> userService_;
    std::shared_ptr<OrderService> orderService_;
};

// ============================================================================
// Сквозной сценарий: полный цикл жизни пользователя и заказов
// ============================================================================

TEST_F(UserOrderIntegrationTest, FullUserOrderLifecycle) {
    // 1. Регистрация нового пользователя
    int user_id = userService_->createUser("Иван Петров", "ivan@example.com");
    ASSERT_GT(user_id, 0) << "Пользователь должен быть создан";
    
    // 2. Пользователь делает первый заказ
    int order1_id = orderService_->createOrder(user_id, "Ноутбук", 75000.0);
    ASSERT_GT(order1_id, 0) << "Первый заказ должен быть создан";
    
    // 3. Пользователь делает второй заказ
    int order2_id = orderService_->createOrder(user_id, "Мышь", 2500.0);
    ASSERT_GT(order2_id, 0) << "Второй заказ должен быть создан";
    
    // 4. Проверяем что оба заказа видны
    auto orders = orderService_->getUserOrders(user_id);
    EXPECT_EQ(orders.size(), 2) << "У пользователя должно быть 2 заказа";
    
    // 5. Проверяем общую сумму
    double total = orderService_->getTotalAmount(user_id);
    EXPECT_DOUBLE_EQ(total, 77500.0) << "Общая сумма должна быть 77500";
    
    // 6. Подтверждаем первый заказ
    EXPECT_TRUE(orderService_->updateOrderStatus(order1_id, OrderStatus::CONFIRMED));
    
    // 7. Отменяем второй заказ
    EXPECT_TRUE(orderService_->cancelOrder(order2_id));
    
    // 8. Проверяем что сумма уменьшилась
    total = orderService_->getTotalAmount(user_id);
    EXPECT_DOUBLE_EQ(total, 75000.0) << "После отмены сумма должна быть 75000";
    
    // 9. Отправляем первый заказ
    EXPECT_TRUE(orderService_->updateOrderStatus(order1_id, OrderStatus::SHIPPED));
    
    // 10. Теперь первый заказ нельзя отменить
    EXPECT_FALSE(orderService_->cancelOrder(order1_id)) 
        << "Отправленный заказ нельзя отменить";
    
    // 11. Доставляем заказ
    EXPECT_TRUE(orderService_->updateOrderStatus(order1_id, OrderStatus::DELIVERED));
    
    // Финальная проверка состояния
    auto order1 = orderService_->getOrder(order1_id);
    auto order2 = orderService_->getOrder(order2_id);
    
    ASSERT_TRUE(order1.has_value());
    ASSERT_TRUE(order2.has_value());
    EXPECT_EQ(order1->status, OrderStatus::DELIVERED);
    EXPECT_EQ(order2->status, OrderStatus::CANCELLED);
}

// ============================================================================
// Сценарий: множество пользователей
// ============================================================================

TEST_F(UserOrderIntegrationTest, MultipleUsersIndependentOrders) {
    // Создаем трех пользователей
    int user1_id = userService_->createUser("User1", "user1@test.com");
    int user2_id = userService_->createUser("User2", "user2@test.com");
    int user3_id = userService_->createUser("User3", "user3@test.com");
    
    // Каждый делает заказы
    orderService_->createOrder(user1_id, "Product A", 100.0);
    orderService_->createOrder(user1_id, "Product B", 200.0);
    
    orderService_->createOrder(user2_id, "Product C", 300.0);
    
    orderService_->createOrder(user3_id, "Product D", 400.0);
    orderService_->createOrder(user3_id, "Product E", 500.0);
    orderService_->createOrder(user3_id, "Product F", 600.0);
    
    // Проверяем изоляцию заказов
    EXPECT_EQ(orderService_->getUserOrders(user1_id).size(), 2);
    EXPECT_EQ(orderService_->getUserOrders(user2_id).size(), 1);
    EXPECT_EQ(orderService_->getUserOrders(user3_id).size(), 3);
    
    // Проверяем суммы
    EXPECT_DOUBLE_EQ(orderService_->getTotalAmount(user1_id), 300.0);
    EXPECT_DOUBLE_EQ(orderService_->getTotalAmount(user2_id), 300.0);
    EXPECT_DOUBLE_EQ(orderService_->getTotalAmount(user3_id), 1500.0);
}

// ============================================================================
// Сценарий: деактивация пользователя влияет на возможность создания заказов
// ============================================================================

TEST_F(UserOrderIntegrationTest, DeactivatedUserCannotCreateNewOrders) {
    // Создаем пользователя и заказ
    int user_id = userService_->createUser("Test User", "test@test.com");
    int existing_order_id = orderService_->createOrder(user_id, "Existing Product", 100.0);
    ASSERT_GT(existing_order_id, 0);
    
    // Деактивируем пользователя
    EXPECT_TRUE(userService_->deactivateUser(user_id));
    
    // Попытка создать новый заказ должна провалиться
    int new_order_id = orderService_->createOrder(user_id, "New Product", 200.0);
    EXPECT_EQ(new_order_id, -1) << "Деактивированный пользователь не может создавать заказы";
    
    // Существующие заказы остаются доступными
    auto existing_order = orderService_->getOrder(existing_order_id);
    EXPECT_TRUE(existing_order.has_value());
    
    // Можно менять статус существующего заказа
    EXPECT_TRUE(orderService_->updateOrderStatus(existing_order_id, OrderStatus::CONFIRMED));
    
    // Пользователь все еще виден в системе
    auto user = userService_->getUser(user_id);
    EXPECT_TRUE(user.has_value());
    EXPECT_FALSE(user->is_active);
}

// ============================================================================
// Сценарий: состояние базы данных согласовано между сервисами
// ============================================================================

TEST_F(UserOrderIntegrationTest, DatabaseConsistencyAcrossServices) {
    // Создаем данные через сервисы
    int user_id = userService_->createUser("DB Test User", "db@test.com");
    int order_id = orderService_->createOrder(user_id, "DB Test Product", 999.99);
    
    // Проверяем что база данных содержит согласованные данные
    auto db_user = database_->findUserById(user_id);
    auto db_order = database_->findOrderById(order_id);
    
    ASSERT_TRUE(db_user.has_value());
    ASSERT_TRUE(db_order.has_value());
    
    // Данные в БД соответствуют тому, что возвращают сервисы
    auto service_user = userService_->getUser(user_id);
    auto service_order = orderService_->getOrder(order_id);
    
    EXPECT_EQ(db_user->id, service_user->id);
    EXPECT_EQ(db_user->name, service_user->name);
    EXPECT_EQ(db_user->email, service_user->email);
    
    EXPECT_EQ(db_order->id, service_order->id);
    EXPECT_EQ(db_order->user_id, service_order->user_id);
    EXPECT_EQ(db_order->product_name, service_order->product_name);
}

// ============================================================================
// Сценарий: пустая система
// ============================================================================

TEST_F(UserOrderIntegrationTest, EmptySystemBehavior) {
    // Проверяем поведение пустой системы
    EXPECT_FALSE(userService_->userExists(1));
    EXPECT_FALSE(userService_->getUser(1).has_value());
    EXPECT_TRUE(userService_->getActiveUsers().empty());
    
    EXPECT_FALSE(orderService_->getOrder(1).has_value());
    EXPECT_TRUE(orderService_->getUserOrders(1).empty());
    EXPECT_DOUBLE_EQ(orderService_->getTotalAmount(1), 0.0);
}

// ============================================================================
// Сценарий: восстановление после очистки БД
// ============================================================================

TEST_F(UserOrderIntegrationTest, SystemRecoveryAfterClear) {
    // Создаем данные
    int user_id = userService_->createUser("User", "user@test.com");
    orderService_->createOrder(user_id, "Product", 100.0);
    
    // Очищаем БД
    database_->clear();
    
    // Проверяем что система в начальном состоянии
    EXPECT_FALSE(userService_->userExists(user_id));
    EXPECT_TRUE(orderService_->getUserOrders(user_id).empty());
    
    // Можем снова создавать данные
    int new_user_id = userService_->createUser("New User", "new@test.com");
    EXPECT_GT(new_user_id, 0);
    
    int new_order_id = orderService_->createOrder(new_user_id, "New Product", 200.0);
    EXPECT_GT(new_order_id, 0);
}

// ============================================================================
// Сценарий: граничные случаи
// ============================================================================

TEST_F(UserOrderIntegrationTest, EdgeCases) {
    // Минимальные валидные данные
    int user_id = userService_->createUser("A", "a@b");  // Минимальное имя и email
    EXPECT_GT(user_id, 0);
    
    int order_id = orderService_->createOrder(user_id, "X", 0.01);  // Минимальный заказ
    EXPECT_GT(order_id, 0);
    
    // Большие значения
    int order2_id = orderService_->createOrder(user_id, 
        "Very Long Product Name That Should Still Work Fine In The System",
        999999999.99);
    EXPECT_GT(order2_id, 0);
    
    // Много заказов от одного пользователя
    for (int i = 0; i < 100; ++i) {
        int oid = orderService_->createOrder(user_id, "Product " + std::to_string(i), 10.0);
        EXPECT_GT(oid, 0);
    }
    
    auto orders = orderService_->getUserOrders(user_id);
    EXPECT_EQ(orders.size(), 102);  // 2 + 100
}

// ============================================================================
// Сценарий: проверка изоляции тестов (TearDown работает корректно)
// ============================================================================

TEST_F(UserOrderIntegrationTest, TestIsolation_Part1) {
    // Этот тест создает данные
    userService_->createUser("Isolation Test", "iso@test.com");
    auto users = userService_->getActiveUsers();
    EXPECT_EQ(users.size(), 1);
}

TEST_F(UserOrderIntegrationTest, TestIsolation_Part2) {
    // Этот тест должен начинаться с чистой базы
    auto users = userService_->getActiveUsers();
    EXPECT_EQ(users.size(), 0) << "База должна быть пустой между тестами";
}

