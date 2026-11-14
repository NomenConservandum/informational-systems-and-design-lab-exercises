#ifndef PHONE_STATE_LIB_HPP
#define PHONE_STATE_LIB_HPP

#include <memory>
#include <string>
#include <vector>

namespace phone {

class Phone;

/**
 * Структура результата выполнения операции телефона.
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
 * PhoneState
 * Абстрактный класс (интерфейс) "Состояние" (State в шаблоне GoF).
 *
 * Определяет единый интерфейс для всех конкретных состояний конечного автомата
 * "Телефон". Каждая операция из технического задания представлена чисто
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
 * WaitingState
 * Конкретное состояние "Ожидание" (ConcreteState в шаблоне GoF).
 *
 * Телефон включён, баланс неотрицательный (balance >= 0), линия свободна.
 * Допустимые операции:
 * - makeCall(targetNumber): инициирует исходящий вызов и переводит автомат в
 *   состояние CallingState ("Звонок").
 * - answerCall(): принимает бесплатный входящий звонок и переводит автомат
 *   в состояние TalkingState ("Разговор").
 * - topUpBalance(amount): увеличивает баланс, оставаясь в WaitingState.
 * - endCall(): отклоняется, так как активного звонка нет.
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
 * CallingState
 * Конкретное состояние "Звонок" (ConcreteState в шаблоне GoF).
 *
 * На телефоне идёт исходящий вызов (гудки ожидания ответа абонента).
 * Допустимые операции:
 * - answerCall(): устанавливает соединение и переводит автомат в состояние
 *   TalkingState ("Разговор").
 * - endCall(): отменяет/сбрасывает вызов без списания средств и возвращает
 *   автомат в WaitingState (при balance >= 0) или в BlockedState (при balance < 0).
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
 * TalkingState
 * Конкретное состояние "Разговор" (ConcreteState в шаблоне GoF).
 *
 * Голосовое соединение установлено, идёт разговор с абонентом.
 * Допустимые операции:
 * - endCall(): завершает разговор. Для исходящего вызова списывается стоимость
 *   звонка (callCost), для входящего вызова списание равно 0 руб.
 *   Если после завершения balance < 0, телефон переходит в состояние
 *   BlockedState ("Заблокирован (баланс отрицательный)"); если balance >= 0 —
 *   возвращается в состояние WaitingState ("Ожидание").
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
 * BlockedState
 * Конкретное состояние "Заблокирован (баланс отрицательный)" (ConcreteState в шаблоне GoF).
 *
 * Баланс телефона строго отрицателен (balance < 0).
 * Платные исходящие вызовы заблокированы, однако приём входящих звонков
 * доступен и бесплатен (списание происходит со счёта звонящего).
 * Допустимые операции:
 * - topUpBalance(amount): пополняет баланс на сумму amount > 0. Если итоговый
 *   balance >= 0, телефон разблокируется и переходит в WaitingState ("Ожидание");
 *   если balance < 0 — остаётся в BlockedState.
 * - answerCall(): принимает бесплатный входящий вызов и переводит телефон в TalkingState.
 * - makeCall(), endCall(): отклоняются.
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
 * Phone
 * Класс "Телефон" / "Конечный автомат" (Context в шаблоне GoF).
 *
 * Инкапсулирует атрибуты предметной области по ТЗ:
 * - номер телефона (phoneNumber_);
 * - текущий баланс средств на счёте (balance_);
 * - вероятность поступления звонка (callProbability_).
 * Хранит ссылку на объект текущего состояния (currentState_) и делегирует ему
 * выполнение всех операций жизненного цикла.
 */
class Phone {
private:
    std::string phoneNumber_;                  ///< Номер телефона абонента
    int balance_;                              ///< Текущий баланс счёта (в руб.)
    double callProbability_;                   ///< Вероятность поступления звонка [0.0..1.0]
    int callCost_;                             ///< Стоимость одного исходящего разговора (в руб.)
    std::string activePeerNumber_;             ///< Номер собеседника в текущем вызове/разговоре
    bool isIncomingCall_;                      ///< Признак входящего (бесплатного) звонка
    std::shared_ptr<PhoneState> currentState_; ///< Текущее состояние автомата
    std::vector<std::string> historyLog_;      ///< Журнал переходов и операций

    void recordHistory(const OperationResult& result, const std::string& operationDesc);

public:
    /**
     * Конструктор конечного автомата "Телефон".
     * phoneNumber Номер телефона.
     * initialBalance Начальный баланс счёта (при < 0 переходит в BlockedState).
     * callProbability Вероятность поступления звонка в [0.0, 1.0] (атрибут модели по ТЗ).
     * callCost Тариф за один исходящий разговор (по умолчанию 50 руб.).
     */
    Phone(std::string phoneNumber,
          int initialBalance,
          double callProbability = 1.0,
          int callCost = 50);

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
    bool isIncomingCall() const;
    void setIncomingCall(bool incoming);
    const std::vector<std::string>& getHistoryLog() const;
    std::string getStatusSummary() const;
};

} // namespace phone

#endif // PHONE_STATE_LIB_HPP

