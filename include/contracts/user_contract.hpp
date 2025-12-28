#pragma once

#include <string>
#include <optional>
#include <vector>

namespace contracts {

/**
 * @brief Структура данных пользователя - часть контракта
 */
struct User {
    int id;
    std::string name;
    std::string email;
    bool is_active;

    bool operator==(const User& other) const {
        return id == other.id && 
               name == other.name && 
               email == other.email && 
               is_active == other.is_active;
    }
};

/**
 * @brief Интерфейс (контракт) сервиса пользователей
 * 
 * Этот интерфейс определяет контракт, который должен соблюдаться
 * любой реализацией сервиса пользователей.
 */
class IUserService {
public:
    virtual ~IUserService() = default;

    /**
     * @brief Создать нового пользователя
     * @param name Имя пользователя (не должно быть пустым)
     * @param email Email пользователя (должен содержать @)
     * @return ID созданного пользователя или -1 при ошибке
     */
    virtual int createUser(const std::string& name, const std::string& email) = 0;

    /**
     * @brief Получить пользователя по ID
     * @param id ID пользователя
     * @return Пользователь или nullopt если не найден
     */
    virtual std::optional<User> getUser(int id) const = 0;

    /**
     * @brief Получить всех активных пользователей
     * @return Список активных пользователей
     */
    virtual std::vector<User> getActiveUsers() const = 0;

    /**
     * @brief Деактивировать пользователя
     * @param id ID пользователя
     * @return true если успешно, false если пользователь не найден
     */
    virtual bool deactivateUser(int id) = 0;

    /**
     * @brief Проверить существование пользователя
     * @param id ID пользователя
     * @return true если пользователь существует
     */
    virtual bool userExists(int id) const = 0;
};

} // namespace contracts

