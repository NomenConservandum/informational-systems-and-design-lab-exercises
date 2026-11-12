/**
 * @file atm_state_lib.hpp
 * @brief Библиотека классов для реализации шаблона проектирования «Состояние» (State)
 *        на примере конечного автомата системы «Банкомат» (Вариант 5/1).
 *
 * Соответствие элементам шаблона GoF «State» (Состояние):
 * - Context (Контекст / Конечный автомат) -> класс ATM («Банкомат»)
 * - State (Интерфейс состояния)          -> абстрактный класс ATMState («Состояние»)
 * - ConcreteState (Конкретные состояния) -> классы:
 *     1. WaitingState              («Ожидание»)
 *     2. AuthenticationState       («Аутентификация пользователя»)
 *     3. PerformingOperationsState («Выполнение операций»)
 *     4. BlockedState              («Заблокирован (денег нет)»)
 */

#ifndef ATM_STATE_LIB_HPP
#define ATM_STATE_LIB_HPP

#include <memory>
#include <random>
#include <string>
#include <vector>

namespace atm {

class ATM;

/**
 * @brief Структура результата выполнения операции банкомата.
 * Содержит флаг успешности операции, текстовое сообщение, а также
 * имена состояний до и после вызова операции.
 */
struct OperationResult {
    bool success;               ///< Признак успешного выполнения операции
    std::string message;        ///< Описание реакции автомата на операцию
    std::string previousState;  ///< Состояние до вызова операции
    std::string currentState;   ///< Состояние после вызова операции
};

/**
 * @class ATMState
 * @brief Абстрактный класс (интерфейс) «Состояние» (State в шаблоне GoF).
 *
 * Определяет единый интерфейс для всех конкретных состояний конечного автомата
 * «Банкомат». Каждая операция из технического задания представлена виртуальным
 * методом, принимающим ссылку на контекст (экземпляр класса ATM).
 */
class ATMState {
public:
    virtual ~ATMState() = default;

    /// Возвращает символьное и русскоязычное название состояния
    virtual std::string getName() const = 0;

    /// Операция 1: Ввести PIN-код
    virtual OperationResult enterPin(ATM& atm, const std::string& pin) = 0;

    /// Операция 2: Снять заданную сумму
    virtual OperationResult withdrawCash(ATM& atm, int amount) = 0;

    /// Операция 3: Завершить работу (сеанс обслуживания клиента)
    virtual OperationResult finishSession(ATM& atm) = 0;

    /// Операция 4: Загрузить деньги в банкомат (инкассация)
    virtual OperationResult loadCash(ATM& atm, int amount) = 0;
};

/**
 * @class WaitingState
 * @brief Конкретное состояние «Ожидание» (ConcreteState в шаблоне GoF).
 *
 * Банкомат исправен, содержит наличные средства (totalCash > 0) и ожидает клиента.
 * Допустимые операции:
 * - enterPin(pin): инициирует сеанс и переводит автомат в AuthenticationState;
 *   при наличии связи с банком и верном PIN сразу переводит в PerformingOperationsState,
 *   а при сбое связи или неверном PIN оставляет в AuthenticationState.
 * - loadCash(amount): пополняет кассету банкомата, оставаясь в WaitingState.
 * - withdrawCash(amount), finishSession(): отклоняются.
 */
class WaitingState : public ATMState {
public:
    std::string getName() const override;
    OperationResult enterPin(ATM& atm, const std::string& pin) override;
    OperationResult withdrawCash(ATM& atm, int amount) override;
    OperationResult finishSession(ATM& atm) override;
    OperationResult loadCash(ATM& atm, int amount) override;
};

/**
 * @class AuthenticationState
 * @brief Конкретное состояние «Аутентификация пользователя» (ConcreteState в шаблоне GoF).
 *
 * Сеанс клиента инициирован, но проверка подлинности ещё не завершена
 * (был введён неверный PIN-код или произошёл временный сбой связи с банком).
 * Допустимые операции:
 * - enterPin(pin): повторная проверка связи с банком и PIN-кода; при успехе
 *   переводит автомат в состояние PerformingOperationsState.
 * - finishSession(): отменяет аутентификацию и возвращает автомат в WaitingState.
 * - withdrawCash(amount), loadCash(amount): отклоняются.
 */
class AuthenticationState : public ATMState {
public:
    std::string getName() const override;
    OperationResult enterPin(ATM& atm, const std::string& pin) override;
    OperationResult withdrawCash(ATM& atm, int amount) override;
    OperationResult finishSession(ATM& atm) override;
    OperationResult loadCash(ATM& atm, int amount) override;
};

/**
 * @class PerformingOperationsState
 * @brief Конкретное состояние «Выполнение операций» (ConcreteState в шаблоне GoF).
 *
 * Пользователь успешно прошёл аутентификацию и может снимать наличные деньги.
 * Допустимые операции:
 * - withdrawCash(amount): проверяет связь с банком и достаточность средств;
 *   списывает сумму. Если после выдачи totalCash == 0, переводит банкомат
 *   в состояние BlockedState («Заблокирован (денег нет)»).
 * - finishSession(): завершает сеанс клиента и переводит банкомат в WaitingState.
 * - enterPin(pin), loadCash(amount): отклоняются.
 */
class PerformingOperationsState : public ATMState {
public:
    std::string getName() const override;
    OperationResult enterPin(ATM& atm, const std::string& pin) override;
    OperationResult withdrawCash(ATM& atm, int amount) override;
    OperationResult finishSession(ATM& atm) override;
    OperationResult loadCash(ATM& atm, int amount) override;
};

/**
 * @class BlockedState
 * @brief Конкретное состояние «Заблокирован (денег нет)» (ConcreteState в шаблоне GoF).
 *
 * В банкомате отсутствуют наличные средства (totalCash == 0). Обслуживание
 * клиентов заблокировано до проведения инкассации.
 * Допустимые операции:
 * - loadCash(amount): при amount > 0 пополняет баланс банкомата и переводит
 *   его в состояние WaitingState («Ожидание»).
 * - enterPin(pin), withdrawCash(amount), finishSession(): отклоняются.
 */
class BlockedState : public ATMState {
public:
    std::string getName() const override;
    OperationResult enterPin(ATM& atm, const std::string& pin) override;
    OperationResult withdrawCash(ATM& atm, int amount) override;
    OperationResult finishSession(ATM& atm) override;
    OperationResult loadCash(ATM& atm, int amount) override;
};

/**
 * @class ATM
 * @brief Класс «Банкомат» / «Конечный автомат» (Context в шаблоне GoF «State»).
 *
 * Хранит атрибуты банкомата по ТЗ (ID, общая сумма денег, вероятность отсутствия
 * связи с банком) и ссылку на объект текущего состояния (currentState_).
 * Делегирует выполнение всех операций объекту текущего состояния.
 */
class ATM {
private:
    std::string id_;                          ///< Уникальный идентификатор банкомата (ID)
    int totalCash_;                           ///< Общая сумма денег в банкомате
    double connectionLossProbability_;        ///< Вероятность отсутствия связи с банком [0.0..1.0]
    std::string validPin_;                    ///< Эталонный PIN-код для проверки аутентификации
    std::shared_ptr<ATMState> currentState_;  ///< Текущее состояние конечного автомата
    std::mt19937 rng_;                        ///< Генератор псевдослучайных чисел для моделирования связи
    std::vector<std::string> historyLog_;     ///< Журнал переходов и выполненных операций

public:
    /**
     * @brief Конструктор конечного автомата «Банкомат».
     * @param id Идентификатор банкомата.
     * @param initialCash Начальная сумма денег в банкомате (>= 0).
     * @param connectionLossProb Вероятность отсутствия связи с банком в диапазоне [0.0, 1.0].
     * @param validPin ПИН-код карты по умолчанию (например, "1234").
     * @param randomSeed Зерно ГПСЧ для воспроизводимости тестов.
     */
    ATM(std::string id,
        int initialCash,
        double connectionLossProb,
        std::string validPin = "1234",
        unsigned int randomSeed = 42);

    // --- Основные операции конечного автомата (делегируются currentState_) ---

    /// Операция: Ввести PIN-код
    OperationResult enterPin(const std::string& pin);

    /// Операция: Снять заданную сумму
    OperationResult withdrawCash(int amount);

    /// Операция: Завершить работу
    OperationResult finishSession();

    /// Операция: Загрузить деньги в банкомат
    OperationResult loadCash(int amount);

    // --- Методы управления состоянием и служебные методы контекста ---

    /// Устанавливает новое состояние конечного автомата
    void setState(std::shared_ptr<ATMState> newState);

    /// Возвращает указатель на объект текущего состояния
    std::shared_ptr<ATMState> getState() const;

    /// Возвращает строковое имя текущего состояния
    std::string getStateName() const;

    /// Проверяет наличие связи с банком с учетом вероятности отсутствия связи
    bool checkBankConnection();

    /// Проверяет совпадение введённого PIN-кода с эталонным
    bool verifyPin(const std::string& pin) const;

    /// Увеличивает сумму наличных в банкомате
    void addCash(int amount);

    /// Уменьшает сумму наличных в банкомате
    void deductCash(int amount);

    // --- Геттеры и сеттеры атрибутов ---

    const std::string& getId() const;
    int getTotalCash() const;
    double getConnectionLossProbability() const;
    void setConnectionLossProbability(double probability);
    const std::string& getValidPin() const;
    const std::vector<std::string>& getHistoryLog() const;

    /// Формирует сводную строку статуса экземпляра банкомата
    std::string getStatusSummary() const;

private:
    void recordLog(const std::string& operation, const OperationResult& result);
};

} // namespace atm

#endif // ATM_STATE_LIB_HPP

