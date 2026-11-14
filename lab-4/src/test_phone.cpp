#include "phone_state_lib.hpp"
#include <cassert>
#include <iostream>
#include <string>

namespace {

using phone::Phone;

/// Тест 1: Начальные состояния конечного автомата при неотрицательном (вкл. 0) и отрицательном балансе
void testInitialStates() {
    Phone activePhone("+7-900-100-00-01", 150, 1.0);
    assert(activePhone.getStateName().find("WaitingState") != std::string::npos);
    assert(activePhone.getBalance() == 150);

    // Нулевой баланс НЕ является отрицательным -> телефон в режиме WaitingState
    Phone zeroBalancePhone("+7-900-100-00-02", 0, 1.0);
    assert(zeroBalancePhone.getStateName().find("WaitingState") != std::string::npos);

    // Отрицательный баланс -> BlockedState (баланс отрицательный)
    Phone negativeBalancePhone("+7-900-100-00-03", -40, 1.0);
    assert(negativeBalancePhone.getStateName().find("BlockedState") != std::string::npos);

    std::cout << "[PASS] testInitialStates — проверка начальных состояний телефона (0 >= 0 -> Ожидание, < 0 -> Заблокирован)\n";
}

/// Тест 2: Реакции автомата в состоянии \"Ожидание\" (WaitingState)
void testWaitingStateReactions() {
    Phone phone("+7-900-200-00-01", 100, 1.0, 50);

    // Нельзя завершить разговор в режиме ожидания
    auto r1 = phone.endCall();
    assert(!r1.success);
    assert(phone.getStateName().find("WaitingState") != std::string::npos);

    // Пополнение счёта в режиме ожидания
    auto r2 = phone.topUpBalance(50);
    assert(r2.success);
    assert(phone.getBalance() == 150);
    assert(phone.getStateName().find("WaitingState") != std::string::npos);

    // Отказ при нулевой или отрицательной сумме пополнения
    auto r3 = phone.topUpBalance(-10);
    assert(!r3.success);
    assert(phone.getBalance() == 150);

    // Детерминированное поведение: даже при callProbability == 0.0 случайных отказов нет
    phone.setCallProbability(0.0);
    assert(phone.getCallProbability() == 0.0);

    // Входящий звонок в WaitingState принимается без случайных отказов (WaitingState -> TalkingState)
    // и при завершении разговора баланс НЕ уменьшается (0 руб.)
    auto r4 = phone.answerCall();
    assert(r4.success);
    assert(phone.getStateName().find("TalkingState") != std::string::npos);

    auto r5 = phone.endCall();
    assert(r5.success);
    assert(phone.getBalance() == 150);
    assert(phone.getStateName().find("WaitingState") != std::string::npos);

    // Исходящий звонок также детерминирован и не блокируется значением callProbability
    auto r6 = phone.makeCall("+7-900-999-00-00");
    assert(r6.success);
    assert(phone.getStateName().find("CallingState") != std::string::npos);
    assert(phone.endCall().success);
    assert(phone.getStateName().find("WaitingState") != std::string::npos);

    // Проверка при нулевом балансе (0 руб.): можно как принимать входящие, так и позвонить
    Phone zeroPhone("+7-900-200-00-02", 0, 1.0, 50);
    assert(zeroPhone.answerCall().success);
    assert(zeroPhone.endCall().success);
    assert(zeroPhone.getBalance() == 0);
    assert(zeroPhone.getStateName().find("WaitingState") != std::string::npos);

    std::cout << "[PASS] testWaitingStateReactions — реакции в состоянии WaitingState и бесплатные входящие\n";
}

/// Тест 3: Переходы в состоянии \"Звонок\" (CallingState)
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

/// Тест 4: Состояние \"Разговор\" (TalkingState): 0 руб. оставляет в WaitingState, < 0 -> BlockedState
void testTalkingStateAndBlocking() {
    Phone phone("+7-900-400-00-01", 100, 1.0, 50);

    assert(phone.makeCall("+7-900-555-66-77").success);
    assert(phone.answerCall().success);
    assert(phone.getStateName().find("TalkingState") != std::string::npos);

    // Во время разговора нельзя набрать новый номер или повторно ответить
    assert(!phone.makeCall("+7-900-000-00-00").success);
    assert(!phone.answerCall().success);

    // Завершение первого исходящего разговора: списание 50 руб., остаток 50 >= 0 -> WaitingState
    auto r1 = phone.endCall();
    assert(r1.success);
    assert(phone.getBalance() == 50);
    assert(phone.getStateName().find("WaitingState") != std::string::npos);

    // Второй исходящий разговор: списание 50 руб., баланс ровно 0 >= 0 -> ещё WaitingState
    assert(phone.makeCall("+7-900-555-66-77").success);
    assert(phone.answerCall().success);
    auto r2 = phone.endCall();
    assert(r2.success);
    assert(phone.getBalance() == 0);
    assert(phone.getStateName().find("WaitingState") != std::string::npos);

    // Третий исходящий разговор при нулевом балансе уводит баланс в -50 < 0 -> BlockedState
    assert(phone.makeCall("+7-900-555-66-77").success);
    assert(phone.answerCall().success);
    auto r3 = phone.endCall();
    assert(r3.success);
    assert(phone.getBalance() == -50);
    assert(phone.getStateName().find("BlockedState") != std::string::npos);

    std::cout << "[PASS] testTalkingStateAndBlocking — тарификация исходящих и блокировка при balance < 0\n";
}

/// Тест 5: Блокировка при отрицательном балансе, приём входящих звонков и разблокировка при balance >= 0
void testBlockedStateAndUnblocking() {
    Phone phone("+7-900-500-00-01", -40, 1.0, 50);
    assert(phone.getStateName().find("BlockedState") != std::string::npos);

    // При отрицательном балансе исходящие звонки и endCall без разговора запрещены
    assert(!phone.makeCall("+7-900-123-45-67").success);
    assert(!phone.endCall().success);

    // При отрицательном балансе (-40 руб.) входящий звонок ПРИНИМАЕТСЯ детерминированно (BlockedState -> TalkingState)
    phone.setCallProbability(0.0);
    assert(phone.answerCall().success);
    assert(phone.getStateName().find("TalkingState") != std::string::npos);

    // Завершение входящего разговора не списывает средства (0 руб.) и возвращает в BlockedState (-40 руб.)
    assert(phone.endCall().success);
    assert(phone.getBalance() == -40);
    assert(phone.getStateName().find("BlockedState") != std::string::npos);

    // Частичное пополнение (+20 руб.): баланс -20 < 0 -> по-прежнему заблокирован для исходящих
    auto r1 = phone.topUpBalance(20);
    assert(r1.success);
    assert(phone.getBalance() == -20);
    assert(phone.getStateName().find("BlockedState") != std::string::npos);

    // Приём входящего звонка + пополнение во время разговора до 0 руб. (>= 0) -> разблокировка в WaitingState
    assert(phone.answerCall().success);
    assert(phone.topUpBalance(20).success);
    assert(phone.getBalance() == 0);
    assert(phone.endCall().success);
    assert(phone.getStateName().find("WaitingState") != std::string::npos);

    // Проверка прямого выхода из BlockedState пополнением до 0 и > 0
    Phone blockedTwo("+7-900-500-00-02", -30, 1.0, 50);
    assert(blockedTwo.topUpBalance(30).success);
    assert(blockedTwo.getBalance() == 0);
    assert(blockedTwo.getStateName().find("WaitingState") != std::string::npos);

    // После разблокировки исходящий звонок снова доступен
    blockedTwo.topUpBalance(100);
    assert(blockedTwo.makeCall("+7-900-123-45-67").success);
    assert(blockedTwo.answerCall().success);
    assert(blockedTwo.endCall().success);
    assert(blockedTwo.getBalance() == 50);

    std::cout << "[PASS] testBlockedStateAndUnblocking — блокировка при balance < 0, бесплатные входящие и разблокировка\n";
}

} // namespace

int main() {
    std::cout << "================================================================================\n";
    std::cout << "    ЗАПУСК МОДУЛЬНЫХ ТЕСТОВ БИБЛИОТЕКИ КЛАССОВ \"ТЕЛЕФОН\" (ПАТТЕРН \"СОСТОЯНИЕ\")\n";
    std::cout << "================================================================================\n";

    testInitialStates();
    testWaitingStateReactions();
    testCallingStateTransitions();
    testTalkingStateAndBlocking();
    testBlockedStateAndUnblocking();

    std::cout << "================================================================================\n";
    std::cout << " Завершение пяти тестовых наборов.\n";
    std::cout << "================================================================================\n";
    return 0;
}

