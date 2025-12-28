/**
 * @file user_contract_test.cpp
 * @brief Контрактные тесты для IUserService
 * 
 * Эти тесты проверяют, что любая реализация IUserService
 * соответствует определенному контракту. Контракт определяет:
 * 1. Предусловия (что должно быть выполнено перед вызовом)
 * 2. Постусловия (что гарантируется после вызова)
 * 3. Инварианты (что всегда должно быть истинным)
 */

#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "contracts/user_contract.hpp"
#include "services/user_service.hpp"
#include "services/database.hpp"

using namespace services;
using namespace contracts;

/**
 * Тестовый класс для проверки контракта IUserService.
 * Любая реализация IUserService должна проходить эти тесты.
 */
class UserContractTest : public ::testing::Test {
protected:
    void SetUp() override {
        database_ = std::make_shared<InMemoryDatabase>();
        // Тестируем конкретную реализацию на соответствие контракту
        service_ = std::make_unique<UserService>(database_);
    }

    std::shared_ptr<InMemoryDatabase> database_;
    std::unique_ptr<IUserService> service_;  // Используем интерфейс!
};

// ============================================================================
// КОНТРАКТ: createUser
// Предусловие: name не пустое, email содержит @
// Постусловие: возвращает положительный ID при успехе, -1 при нарушении предусловия
// ============================================================================

TEST_F(UserContractTest, CreateUser_Contract_ValidInput_ReturnsPositiveId) {
    // Arrange: предусловия выполнены
    std::string valid_name = "John";
    std::string valid_email = "john@test.com";
    
    // Act
    int result = service_->createUser(valid_name, valid_email);
    
    // Assert: постусловие - положительный ID
    EXPECT_GT(result, 0) 
        << "CONTRACT VIOLATION: createUser must return positive ID for valid input";
}

TEST_F(UserContractTest, CreateUser_Contract_EmptyName_ReturnsMinusOne) {
    // Arrange: нарушено предусловие (пустое имя)
    std::string empty_name = "";
    std::string valid_email = "john@test.com";
    
    // Act
    int result = service_->createUser(empty_name, valid_email);
    
    // Assert: постусловие - возврат -1
    EXPECT_EQ(result, -1) 
        << "CONTRACT VIOLATION: createUser must return -1 for empty name";
}

TEST_F(UserContractTest, CreateUser_Contract_InvalidEmail_ReturnsMinusOne) {
    // Arrange: нарушено предусловие (email без @)
    std::string valid_name = "John";
    std::string invalid_email = "invalid-email";
    
    // Act
    int result = service_->createUser(valid_name, invalid_email);
    
    // Assert: постусловие - возврат -1
    EXPECT_EQ(result, -1) 
        << "CONTRACT VIOLATION: createUser must return -1 for email without @";
}

TEST_F(UserContractTest, CreateUser_Contract_NewUserIsActive) {
    // Arrange & Act
    int id = service_->createUser("John", "john@test.com");
    auto user = service_->getUser(id);
    
    // Assert: инвариант - новый пользователь всегда активен
    ASSERT_TRUE(user.has_value());
    EXPECT_TRUE(user->is_active) 
        << "CONTRACT VIOLATION: newly created user must be active";
}

// ============================================================================
// КОНТРАКТ: getUser
// Предусловие: нет
// Постусловие: возвращает User если существует, nullopt если нет
// ============================================================================

TEST_F(UserContractTest, GetUser_Contract_ExistingUser_ReturnsUser) {
    // Arrange
    int id = service_->createUser("John", "john@test.com");
    
    // Act
    auto result = service_->getUser(id);
    
    // Assert: постусловие - возвращается существующий пользователь
    ASSERT_TRUE(result.has_value()) 
        << "CONTRACT VIOLATION: getUser must return user for existing ID";
    EXPECT_EQ(result->id, id);
}

TEST_F(UserContractTest, GetUser_Contract_NonExistingUser_ReturnsNullopt) {
    // Act
    auto result = service_->getUser(99999);
    
    // Assert: постусловие - nullopt для несуществующего ID
    EXPECT_FALSE(result.has_value()) 
        << "CONTRACT VIOLATION: getUser must return nullopt for non-existing ID";
}

TEST_F(UserContractTest, GetUser_Contract_DataIntegrity) {
    // Arrange
    std::string name = "John Doe";
    std::string email = "john.doe@test.com";
    int id = service_->createUser(name, email);
    
    // Act
    auto user = service_->getUser(id);
    
    // Assert: инвариант - данные сохраняются корректно
    ASSERT_TRUE(user.has_value());
    EXPECT_EQ(user->name, name) 
        << "CONTRACT VIOLATION: user name must be preserved";
    EXPECT_EQ(user->email, email) 
        << "CONTRACT VIOLATION: user email must be preserved";
}

// ============================================================================
// КОНТРАКТ: getActiveUsers
// Предусловие: нет
// Постусловие: возвращает только активных пользователей
// ============================================================================

TEST_F(UserContractTest, GetActiveUsers_Contract_ReturnsOnlyActive) {
    // Arrange
    int id1 = service_->createUser("User1", "user1@test.com");
    int id2 = service_->createUser("User2", "user2@test.com");
    service_->deactivateUser(id1);
    
    // Act
    auto activeUsers = service_->getActiveUsers();
    
    // Assert: постусловие - все пользователи активны
    for (const auto& user : activeUsers) {
        EXPECT_TRUE(user.is_active) 
            << "CONTRACT VIOLATION: getActiveUsers must return only active users";
    }
    
    // Дополнительная проверка: деактивированный пользователь не в списке
    auto it = std::find_if(activeUsers.begin(), activeUsers.end(),
        [id1](const User& u) { return u.id == id1; });
    EXPECT_EQ(it, activeUsers.end()) 
        << "CONTRACT VIOLATION: deactivated user must not be in active users list";
}

// ============================================================================
// КОНТРАКТ: deactivateUser
// Предусловие: нет
// Постусловие: true если пользователь существовал и деактивирован, false иначе
// ============================================================================

TEST_F(UserContractTest, DeactivateUser_Contract_ExistingUser_ReturnsTrue) {
    // Arrange
    int id = service_->createUser("John", "john@test.com");
    
    // Act
    bool result = service_->deactivateUser(id);
    
    // Assert
    EXPECT_TRUE(result) 
        << "CONTRACT VIOLATION: deactivateUser must return true for existing user";
}

TEST_F(UserContractTest, DeactivateUser_Contract_NonExistingUser_ReturnsFalse) {
    // Act
    bool result = service_->deactivateUser(99999);
    
    // Assert
    EXPECT_FALSE(result) 
        << "CONTRACT VIOLATION: deactivateUser must return false for non-existing user";
}

TEST_F(UserContractTest, DeactivateUser_Contract_UserBecomesInactive) {
    // Arrange
    int id = service_->createUser("John", "john@test.com");
    
    // Act
    service_->deactivateUser(id);
    auto user = service_->getUser(id);
    
    // Assert: постусловие - пользователь становится неактивным
    ASSERT_TRUE(user.has_value());
    EXPECT_FALSE(user->is_active) 
        << "CONTRACT VIOLATION: deactivated user must have is_active=false";
}

// ============================================================================
// КОНТРАКТ: userExists
// Предусловие: нет
// Постусловие: true если пользователь существует, false иначе
// ============================================================================

TEST_F(UserContractTest, UserExists_Contract_ConsistentWithGetUser) {
    // Arrange
    int id = service_->createUser("John", "john@test.com");
    
    // Assert: инвариант - userExists согласован с getUser
    EXPECT_TRUE(service_->userExists(id));
    EXPECT_TRUE(service_->getUser(id).has_value());
    
    EXPECT_FALSE(service_->userExists(99999));
    EXPECT_FALSE(service_->getUser(99999).has_value());
}

