/**
 * @file test_atm.cpp
 * @brief Модульные тесты для библиотеки классов шаблона «Состояние» (State)
 *        на примере системы «Банкомат» (Вариант 5/1).
 */

#include "atm_state_lib.hpp"

#include <cassert>
#include <iostream>
#include <string>

using namespace atm;

#define ASSERT_TRUE(cond, msg)                                                        \
    do {                                                                              \
        if (!(cond)) {                                                                \
            std::cerr << "[FAIL] " << __func__ << ": " << (msg) << "\n";              \
            return false;                                                             \
        }                                                                             \
    } while (0)

/**
 * Тест 1: Начальные состояния при положительном и нулевом балансе.
 */
bool testInitialStates() {
    ATM atmReady("ATM-1", 5000, 0.0, "1234");
    ASSERT_TRUE(atmReady.getStateName() == "WaitingState (Ожидание)",
                "При начальной сумме > 0 начальное состояние должно быть WaitingState");

    ATM atmEmpty("ATM-2", 0, 0.0, "1234");
    ASSERT_TRUE(atmEmpty.getStateName() == "BlockedState (Заблокирован: денег нет)",
                "При начальной сумме == 0 начальное состояние должно быть BlockedState");

    std::cout << "[PASS] testInitialStates — проверка начальных состояний автомата\n";
    return true;
}

/**
 * Тест 2: Реакции в состоянии «Ожидание» (WaitingState).
 */
bool testWaitingStateReactions() {
    ATM atm("ATM-W", 10000, 0.0, "1234");

    // Снятие денег и завершение работы недопустимы в режиме ожидания
    auto r1 = atm.withdrawCash(1000);
    ASSERT_TRUE(!r1.success && atm.getStateName() == "WaitingState (Ожидание)",
                "withdrawCash в WaitingState должен отклоняться без смены состояния");

    auto r2 = atm.finishSession();
    ASSERT_TRUE(!r2.success && atm.getStateName() == "WaitingState (Ожидание)",
                "finishSession в WaitingState должен отклоняться без смены состояния");

    // Загрузка денег допустима и увеличивает баланс
    auto r3 = atm.loadCash(3000);
    ASSERT_TRUE(r3.success && atm.getTotalCash() == 13000 &&
                    atm.getStateName() == "WaitingState (Ожидание)",
                "loadCash в WaitingState должен пополнять баланс, оставаясь в WaitingState");

    std::cout << "[PASS] testWaitingStateReactions — реакции в состоянии WaitingState\n";
    return true;
}

/**
 * Тест 3: Переход в состояние «Аутентификация пользователя» (AuthenticationState)
 * при неверном PIN-коде или сбое связи и последующие переходы.
 */
bool testAuthenticationStateTransitions() {
    ATM atm("ATM-A", 10000, 0.0, "1234");

    // 1. Ввод неверного PIN-кода переводит из WaitingState в AuthenticationState
    auto r1 = atm.enterPin("9999");
    ASSERT_TRUE(!r1.success &&
                    atm.getStateName() == "AuthenticationState (Аутентификация пользователя)",
                "Неверный PIN должен переводить из WaitingState в AuthenticationState");

    // 2. Попытка снятия и инкассации в AuthenticationState отклоняется
    ASSERT_TRUE(!atm.withdrawCash(500).success, "withdrawCash в AuthenticationState запрещён");
    ASSERT_TRUE(!atm.loadCash(500).success, "loadCash в AuthenticationState запрещён");

    // 3. Повторный неверный PIN оставляет в AuthenticationState
    auto r2 = atm.enterPin("0000");
    ASSERT_TRUE(!r2.success &&
                    atm.getStateName() == "AuthenticationState (Аутентификация пользователя)",
                "Повторный неверный PIN оставляет автомат в AuthenticationState");

    // 4. Вызов finishSession возвращает автомат в WaitingState
    auto r3 = atm.finishSession();
    ASSERT_TRUE(r3.success && atm.getStateName() == "WaitingState (Ожидание)",
                "finishSession из AuthenticationState возвращает в WaitingState");

    // 5. Обрыв связи с банком (p = 1.0) задерживает автомат в AuthenticationState даже с верным PIN
    atm.setConnectionLossProbability(1.0);
    auto r4 = atm.enterPin("1234");
    ASSERT_TRUE(!r4.success &&
                    atm.getStateName() == "AuthenticationState (Аутентификация пользователя)",
                "При обрыве связи с банком автомат должен остаться в AuthenticationState");

    // 6. Восстановление связи и повторный ввод верного PIN переводит в PerformingOperationsState
    atm.setConnectionLossProbability(0.0);
    auto r5 = atm.enterPin("1234");
    ASSERT_TRUE(r5.success &&
                    atm.getStateName() == "PerformingOperationsState (Выполнение операций)",
                "При успешном повторном вводе PIN автомат переходит в PerformingOperationsState");

    std::cout << "[PASS] testAuthenticationStateTransitions — переходы состояния AuthenticationState\n";
    return true;
}

/**
 * Тест 4: Работа в состоянии «Выполнение операций» (PerformingOperationsState)
 * и автоматический переход в «Заблокирован (денег нет)» (BlockedState).
 */
bool testPerformingOperationsAndBlocking() {
    ATM atm("ATM-P", 5000, 0.0, "1234");

    auto login = atm.enterPin("1234");
    ASSERT_TRUE(login.success &&
                    atm.getStateName() == "PerformingOperationsState (Выполнение операций)",
                "Вход с верным PIN и связью переводит в PerformingOperationsState");

    // Повторный ввод PIN и инкассация в PerformingOperationsState отклоняются
    ASSERT_TRUE(!atm.enterPin("1234").success, "Повторный enterPin в PerformingOperationsState отклоняется");
    ASSERT_TRUE(!atm.loadCash(1000).success, "loadCash во время сеанса клиента отклоняется");

    // Снятие части суммы (2000 из 5000)
    auto w1 = atm.withdrawCash(2000);
    ASSERT_TRUE(w1.success && atm.getTotalCash() == 3000 &&
                    atm.getStateName() == "PerformingOperationsState (Выполнение операций)",
                "Частичное снятие уменьшает остаток и сохраняет состояние PerformingOperationsState");

    // Попытка снять больше остатка (4000 > 3000)
    auto w2 = atm.withdrawCash(4000);
    ASSERT_TRUE(!w2.success && atm.getTotalCash() == 3000,
                "Запрос суммы больше остатка должен отклоняться без изменения баланса");

    // Сбой связи с банком при снятии наличных
    atm.setConnectionLossProbability(1.0);
    auto w3 = atm.withdrawCash(1000);
    ASSERT_TRUE(!w3.success && atm.getTotalCash() == 3000,
                "При отсутствии связи с банком выдача наличных отклоняется");
    atm.setConnectionLossProbability(0.0);

    // Снятие всей оставшейся суммы (3000 из 3000) -> переход в BlockedState
    auto w4 = atm.withdrawCash(3000);
    ASSERT_TRUE(w4.success && atm.getTotalCash() == 0 &&
                    atm.getStateName() == "BlockedState (Заблокирован: денег нет)",
                "При исчерпании наличных (totalCash == 0) автомат должен перейти в BlockedState");

    std::cout << "[PASS] testPerformingOperationsAndBlocking — выполнение операций и блокировка при 0 руб.\n";
    return true;
}

/**
 * Тест 5: Реакции в состоянии «Заблокирован (денег нет)» (BlockedState)
 * и разблокировка при загрузке наличных.
 */
bool testBlockedStateAndUnblocking() {
    ATM atm("ATM-B", 0, 0.0, "1234");
    ASSERT_TRUE(atm.getStateName() == "BlockedState (Заблокирован: денег нет)",
                "Банкомат без денег находится в BlockedState");

    // Все пользовательские операции отклоняются
    ASSERT_TRUE(!atm.enterPin("1234").success, "enterPin в BlockedState запрещён");
    ASSERT_TRUE(!atm.withdrawCash(100).success, "withdrawCash в BlockedState запрещён");
    ASSERT_TRUE(!atm.finishSession().success, "finishSession в BlockedState отклоняется");

    // Загрузка наличных разблокирует банкомат и переводит в WaitingState
    auto loadRes = atm.loadCash(20000);
    ASSERT_TRUE(loadRes.success && atm.getTotalCash() == 20000 &&
                    atm.getStateName() == "WaitingState (Ожидание)",
                "loadCash(amount > 0) в BlockedState переводит автомат в WaitingState");

    std::cout << "[PASS] testBlockedStateAndUnblocking — блокировка и разблокировка инкассацией\n";
    return true;
}

int main() {
    std::cout << "================================================================================\n"
              << "   ЗАПУСК МОДУЛЬНЫХ ТЕСТОВ БИБЛИОТЕКИ КЛАССОВ «БАНКОМАТ» (ПАТТЕРН «СОСТОЯНИЕ»)\n"
              << "================================================================================\n";

    bool allPassed = true;
    allPassed = allPassed && testInitialStates();
    allPassed = allPassed && testWaitingStateReactions();
    allPassed = allPassed && testAuthenticationStateTransitions();
    allPassed = allPassed && testPerformingOperationsAndBlocking();
    allPassed = allPassed && testBlockedStateAndUnblocking();

    std::cout << "================================================================================\n";
    if (allPassed) {
        std::cout << " ИТОГ: Все 5 тестовых наборов успешно пройдены (100% ОК).\n";
        std::cout << "================================================================================\n";
        return 0;
    } else {
        std::cerr << " ИТОГ: Обнаружены ошибки при выполнении тестов!\n";
        return 1;
    }
}

