/**
 * @file phone_state_lib.hpp
 * @brief Библиотека классов для реализации шаблона проектирования «Состояние» (State)
 *        на примере конечного автомата системы «Телефон» (Вариант 5/1).
 *
 * Соответствие элементам шаблона GoF «State» (Состояние):
 * - Context (Контекст / Конечный автомат) -> класс Phone («Телефон»)
 * - State (Интерфейс состояния)          -> абстрактный класс PhoneState («Состояние»)
 * - ConcreteState (Конкретные состояния) -> классы:
 *     1. WaitingState  («Ожидание»)
 *     2. CallingState  («Звонок»)
 *     3. TalkingState  («Разговор»)
 *     4. BlockedState  («Заблокирован (нет средств)»)
 */

#ifndef PHONE_STATE_LIB_HPP
#define PHONE_STATE_LIB_HPP

#include <memory>
#include <random>
#include <string>
#include <vector>

namespace phone {

class Phone;

/**
 * @brief Структура результата выполнения операции телефона.
 * Содержит признак успешности операции, текстовое описание реакции автомата,
 * а также названия состояний до и после вызова операции.
 */
struct OperationResult {
    bool success;               ///< Признак успешного выполнения операции
    std::string message;        ///< Описание реакции автомата на операцию
    std::string previousState;  ///< Состояние до вызова операции
    std::string currentState;   ///< Состояние после вызова операции
};

/**
 * @class PhoneState
 * @brief Абстрактный класс (интерфейс) «Состояние» (State в шаблоне GoF).
 *
 * Определяет единый интерфейс для всех конкретных состояний конечного автомата
 * «Телефон». Каждая операция из технического задания представлена чисто
 * виртуальным методом, принимающим ссылку на контекст (экземпляр класса Phone).
 */
class PhoneState {
public:
    virtual ~PhoneState() = default;

    /// Возвращает символьное и русскоязычное название состояния
    virtual std::string getName() const = 0;

    /// Операция 1: Позвонить (инициировать вызов на номер или принять входящий вызов)
    virtual OperationResult makeCall(Phone& phone, const std::string& targetNumber) = 0;

    /// Операция 2: Ответить на звонок (установить голосовое соединение)
    virtual OperationResult answerCall(Phone& phone) = 0;

    /// Операция 3: Завершить разговор (или сбросить текущий вызов)
    virtual OperationResult endCall(Phone& phone) = 0;

    /// Операция 4: Пополнить баланс телефона на заданную сумму
    virtual OperationResult topUpBalance(Phone& phone, int amount) = 0;
};

/**
 * @class WaitingState
 * @brief Конкретное состояние «Ожидание» (ConcreteState в шаблоне GoF).
 *
 * Телефон включён, баланс положительный (balance > 0), линия свободна.
 * Допустимые операции:
 * - makeCall(targetNumber): проверяет доступность связи/вызова с учётом вероятности
 *   callProbability; при успехе переводит автомат в состояние CallingState («Звонок»).
 * - topUpBalance(amount): увеличивает баланс, оставаясь в WaitingState.
 * - answerCall(), endCall(): отклоняются, так как активного звонка нет.
 */
class WaitingState : public PhoneState {
public:
    std::string getName() const override;
    OperationResult makeCall(Phone& phone, const std::string& targetNumber) override;
    OperationResult answerCall(Phone& phone) override;
    OperationResult endCall(Phone& phone) override;
    OperationResult topUpBalance(Phone& phone, int amount) override;
};

/**
 * @class CallingState
 * @brief Конкретное состояние «Звонок» (ConcreteState в шаблоне GoF).
 *
 * На телефоне идёт исходящий или входящий вызов (гудки вызова).
 * Допустимые операции:
 * - answerCall(): устанавливает соединение и переводит автомат в состояние
 *   TalkingState («Разговор»).
 * - endCall(): отменяет/сбрасывает вызов без списания средств и возвращает
 *   автомат в состояние WaitingState («Ожидание»).
 * - topUpBalance(amount): пополняет счёт без прерывания вызова.
 * - makeCall(): отклоняется (линия уже занята вызовом).
 */
class CallingState : public PhoneState {
public:
    std::string getName() const override;
    OperationResult makeCall(Phone& phone, const std::string& targetNumber) override;
    OperationResult answerCall(Phone& phone) override;
    OperationResult endCall(Phone& phone) override;
    OperationResult topUpBalance(Phone& phone, int amount) override;
};

/**
 * @class TalkingState
 * @brief Конкретное состояние «Разговор» (ConcreteState в шаблоне GoF).
 *
 * Голосовое соединение установлено, идёт разговор с абонентом.
 * Допустимые операции:
 * - endCall(): завершает разговор и списывает стоимость звонка (callCost).
 *   Если после списания balance <= 0, телефон автоматически переходит в
 *   состояние BlockedState («Заблокирован (нет средств)»); если balance > 0 —
 *   возвращается в состояние WaitingState («Ожидание»).
 * - topUpBalance(amount): пополняет баланс во время разговора.
 * - makeCall(), answerCall(): отклоняются (разговор уже активен).
 */
class TalkingState : public PhoneState {
public:
    std::string getName() const override;
    OperationResult makeCall(Phone& phone, const std::string& targetNumber) override;
    OperationResult answerCall(Phone& phone) override;
    OperationResult endCall(Phone& phone) override;
    OperationResult topUpBalance(Phone& phone, int amount) override;
};

/**
 * @class BlockedState
 * @brief Конкретное состояние «Заблокирован (нет средств)» (ConcreteState в шаблоне GoF).
 *
 * Баланс телефона равен нулю или отрицателен (balance <= 0).
 * Допустимые операции:
 * - topUpBalance(amount): пополняет баланс на сумму amount > 0. Если итоговый
 *   balance > 0, телефон разблокируется и переходит в WaitingState («Ожидание»);
 *   если balance <= 0 — остаётся в BlockedState.
 * - makeCall(), answerCall(), endCall(): отклоняются до пополнения баланса.
 */
class BlockedState : public PhoneState {
public:
    std::string getName() const override;
    OperationResult makeCall(Phone& phone, const std::string& targetNumber) override;
    OperationResult answerCall(Phone& phone) override;
    OperationResult endCall(Phone& phone) override;
    OperationResult topUpBalance(Phone& phone, int amount) override;
};

/**
 * @class Phone
 * @brief Класс «Телефон» / «Конечный автомат» (Context в шаблоне GoF).
 *
 * Инкапсулирует атрибуты предметной области по ТЗ:
 * - номер телефона (phoneNumber_);
 * - текущий баланс средств на счёте (balance_);
 * - вероятность успешного вызова / поступления звонка (callProbability_).
 * Хранит ссылку на объект текущего состояния (currentState_) и делегирует ему
 * выполнение всех операций жизненного цикла.
 */
class Phone {
private:
    std::string phoneNumber_;                  ///< Номер телефона абонента
    int balance_;                              ///< Текущий баланс счёта (в руб.)
    double callProbability_;                   ///< Вероятность дозвона / поступления вызова [0.0..1.0]
    int callCost_;                             ///< Стоимость одного разговора (в руб.)
    std::string activePeerNumber_;             ///< Номер собеседника в текущем вызове/разговоре
    std::shared_ptr<PhoneState> currentState_; ///< Текущее состояние автомата
    std::mt19937 rng_;                         ///< Генератор псевдослучайных чисел
    std::vector<std::string> historyLog_;      ///< Журнал переходов и операций

    void recordHistory(const OperationResult& result, const std::string& operationDesc);

public:
    /**
     * @brief Конструктор конечного автомата «Телефон».
     * @param phoneNumber Номер телефона.
     * @param initialBalance Начальный баланс счёта (при <= 0 переходит в BlockedState).
     * @param callProbability Вероятность успешного вызова / поступления звонка в [0.0, 1.0].
     * @param callCost Тариф за один сеанс разговора (по умолчанию 50 руб.).
     * @param randomSeed Зерно ГПСЧ для воспроизводимости тестов.
     */
    Phone(std::string phoneNumber,
          int initialBalance,
          double callProbability,
          int callCost = 50,
          unsigned int randomSeed = 42);

    // Публичные операции ТЗ (делегируются текущему объекту состояния)
    OperationResult makeCall(const std::string& targetNumber = "+7-900-000-00-00");
    OperationResult answerCall();
    OperationResult endCall();
    OperationResult topUpBalance(int amount);

    // Методы управления состоянием (используются классами ConcreteState)
    void setState(std::shared_ptr<PhoneState> newState);
    std::shared_ptr<PhoneState> getState() const;
    std::string getStateName() const;

    // Вспомогательные методы контекста
    bool checkCallProbability();
    void addBalance(int amount);
    void deductCallCost();

    // Геттеры и сеттеры атрибутов
    const std::string& getPhoneNumber() const;
    int getBalance() const;
    double getCallProbability() const;
    void setCallProbability(double probability);
    int getCallCost() const;
    void setCallCost(int cost);
    const std::string& getActivePeerNumber() const;
    void setActivePeerNumber(const std::string& peerNumber);
    const std::vector<std::string>& getHistoryLog() const;
    std::string getStatusSummary() const;
};

} // namespace phone

#endif // PHONE_STATE_LIB_HPP

