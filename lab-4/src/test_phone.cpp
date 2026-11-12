/**
 * @file test_phone.cpp
 * @brief Модульные тесты для библиотеки классов шаблона «Состояние» (State)
 *        системы «Телефон» (Вариант 5/1).
 */

#include "phone_state_lib.hpp"

#include <cassert>
#include <iostream>
#include <string>

namespace {

using phone::Phone;

/// Тест 1: Начальные состояния конечного автомата при положительном и нулевом/отрицательном балансе
void testInitialStates() {
    Phone activePhone("+7-900-100-00-01", 150, 1.0);
    assert(activePhone.getStateName().find("WaitingState") != std::string::npos);
    assert(activePhone.getBalance() == 150);

    Phone zeroBalancePhone("+7-900-100-00-02", 0, 1.0);
    assert(zeroBalancePhone.getStateName().find("BlockedState") != std::string::npos);

    Phone negativeBalancePhone("+7-900-100-00-03", -40, 1.0);
    assert(negativeBalancePhone.getStateName().find("BlockedState") != std::string::npos);

    std::cout << "[PASS] testInitialStates — проверка начальных состояний телефона\n";
}

/// Тест 2: Реакции автомата в состоянии «Ожидание» (WaitingState)
void testWaitingStateReactions() {
    Phone phone("+7-900-200-00-01", 100, 1.0, 50);

    // Нельзя ответить на звонок или завершить разговор в режиме ожидания
    auto r1 = phone.answerCall();
    assert(!r1.success);
    assert(phone.getStateName().find("WaitingState") != std::string::npos);

    auto r2 = phone.endCall();
    assert(!r2.success);
    assert(phone.getStateName().find("WaitingState") != std::string::npos);

    // Пополнение счёта в режиме ожидания
    auto r3 = phone.topUpBalance(50);
    assert(r3.success);
    assert(phone.getBalance() == 150);
    assert(phone.getStateName().find("WaitingState") != std::string::npos);

    // Отказ при нулевой или отрицательной сумме пополнения
    auto r4 = phone.topUpBalance(-10);
    assert(!r4.success);
    assert(phone.getBalance() == 150);

    // При вероятности вызова p = 0.0 звонок не проходит, телефон остаётся в WaitingState
    phone.setCallProbability(0.0);
    auto r5 = phone.makeCall("+7-900-999-00-00");
    assert(!r5.success);
    assert(phone.getStateName().find("WaitingState") != std::string::npos);

    std::cout << "[PASS] testWaitingStateReactions — реакции в состоянии WaitingState\n";
}

/// Тест 3: Переходы в состоянии «Звонок» (CallingState)
void testCallingStateTransitions() {
    Phone phone("+7-900-300-00-01", 100, 1.0, 50);

    // Успешный инициированный вызов: WaitingState -> CallingState
    auto r1 = phone.makeCall("+7-900-333-44-55");
    assert(r1.success);
    assert(phone.getStateName().find("CallingState") != std::string::npos);

    // Повторный вызов в состоянии CallingState отклоняется
    auto r2 = phone.makeCall("+7-900-111-11-11");
    assert(!r2.success);
    assert(phone.getStateName().find("CallingState") != std::string::npos);

    // Сброс вызова до ответа: CallingState -> WaitingState (баланс не списывается)
    auto r3 = phone.endCall();
    assert(r3.success);
    assert(phone.getStateName().find("WaitingState") != std::string::npos);
    assert(phone.getBalance() == 100);

    // Повторный звонок и ответ: WaitingState -> CallingState -> TalkingState
    assert(phone.makeCall("+7-900-333-44-55").success);
    auto r4 = phone.answerCall();
    assert(r4.success);
    assert(phone.getStateName().find("TalkingState") != std::string::npos);

    std::cout << "[PASS] testCallingStateTransitions — переходы состояния CallingState\n";
}

/// Тест 4: Состояние «Разговор» (TalkingState) и блокировка при исчерпании баланса
void testTalkingStateAndBlocking() {
    Phone phone("+7-900-400-00-01", 100, 1.0, 60);

    assert(phone.makeCall("+7-900-555-66-77").success);
    assert(phone.answerCall().success);
    assert(phone.getStateName().find("TalkingState") != std::string::npos);

    // Во время разговора нельзя набрать новый номер или повторно ответить
    assert(!phone.makeCall("+7-900-000-00-00").success);
    assert(!phone.answerCall().success);

    // Завершение первого разговора: списание 60 руб., остаток 40 руб. > 0 -> WaitingState
    auto r1 = phone.endCall();
    assert(r1.success);
    assert(phone.getBalance() == 40);
    assert(phone.getStateName().find("WaitingState") != std::string::npos);

    // Второй разговор: списание ещё 60 руб., баланс становится -20 руб. <= 0 -> BlockedState
    assert(phone.makeCall("+7-900-555-66-77").success);
    assert(phone.answerCall().success);
    auto r2 = phone.endCall();
    assert(r2.success);
    assert(phone.getBalance() == -20);
    assert(phone.getStateName().find("BlockedState") != std::string::npos);

    std::cout << "[PASS] testTalkingStateAndBlocking — разговор, списание средств и блокировка\n";
}

/// Тест 5: Ограничения в состоянии «Заблокирован» (BlockedState) и разблокировка пополнением
void testBlockedStateAndUnblocking() {
    Phone phone("+7-900-500-00-01", -20, 1.0, 50);
    assert(phone.getStateName().find("BlockedState") != std::string::npos);

    // Любые звонки в заблокированном состоянии запрещены
    assert(!phone.makeCall("+7-900-123-45-67").success);
    assert(!phone.answerCall().success);
    assert(!phone.endCall().success);

    // Частичное пополнение до 0 руб. оставляет телефон заблокированным
    auto r1 = phone.topUpBalance(20);
    assert(!r1.success);
    assert(phone.getBalance() == 0);
    assert(phone.getStateName().find("BlockedState") != std::string::npos);

    // Пополнение до положительного баланса (+100 руб.) разблокирует телефон -> WaitingState
    auto r2 = phone.topUpBalance(100);
    assert(r2.success);
    assert(phone.getBalance() == 100);
    assert(phone.getStateName().find("WaitingState") != std::string::npos);

    // Проверка работоспособности после разблокировки
    assert(phone.makeCall("+7-900-123-45-67").success);
    assert(phone.answerCall().success);
    assert(phone.endCall().success);
    assert(phone.getBalance() == 50);

    std::cout << "[PASS] testBlockedStateAndUnblocking — блокировка и разблокировка пополнением баланса\n";
}

} // namespace

int main() {
    std::cout << "================================================================================\n";
    std::cout << "    ЗАПУСК МОДУЛЬНЫХ ТЕСТОВ БИБЛИОТЕКИ КЛАССОВ «ТЕЛЕФОН» (ПАТТЕРН «СОСТОЯНИЕ»)\n";
    std::cout << "================================================================================\n";

    testInitialStates();
    testWaitingStateReactions();
    testCallingStateTransitions();
    testTalkingStateAndBlocking();
    testBlockedStateAndUnblocking();

    std::cout << "================================================================================\n";
    std::cout << " ИТОГ: Все 5 тестовых наборов успешно пройдены (100% ОК).\n";
    std::cout << "================================================================================\n";
    return 0;
}

