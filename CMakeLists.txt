cmake_minimum_required(VERSION 3.14)
project(ContractIntegrationTesting VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Основная библиотека
add_library(services STATIC
    src/user_service.cpp
    src/order_service.cpp
    src/database.cpp
)

target_include_directories(services PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# Google Test
include(FetchContent)
FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG v1.14.0
)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

enable_testing()

# Unit тесты
add_executable(unit_tests
    tests/unit/user_service_test.cpp
    tests/unit/order_service_test.cpp
)
target_link_libraries(unit_tests PRIVATE services GTest::gtest_main GTest::gmock)

# Контрактные тесты
add_executable(contract_tests
    tests/contract/user_contract_test.cpp
    tests/contract/order_contract_test.cpp
)
target_link_libraries(contract_tests PRIVATE services GTest::gtest_main GTest::gmock)

# Интеграционные тесты
add_executable(integration_tests
    tests/integration/user_order_integration_test.cpp
)
target_link_libraries(integration_tests PRIVATE services GTest::gtest_main GTest::gmock)

include(GoogleTest)
gtest_discover_tests(unit_tests)
gtest_discover_tests(contract_tests)
gtest_discover_tests(integration_tests)

