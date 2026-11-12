/**
 * @file atm_state_lib.cpp
 * @brief Реализация библиотеки классов шаблона проектирования «Состояние» (State)
 *        для системы «Банкомат» (Вариант 5/1).
 */

#include "atm_state_lib.hpp"

#include <algorithm>
#include <iomanip>
#include <sstream>

namespace atm {

// ============================================================================
// 1. Состояние «Ожидание» (WaitingState)
// ============================================================================

std::string WaitingState::getName() const {
    return "WaitingState (Ожидание)";
}

OperationResult WaitingState::enterPin(ATM& atm, const std::string& pin) {
    const std::string prevState = getName();

    // При вводе PIN-кода из состояния ожидания сеанс открывается
    // и автомат переходит в состояние «Аутентификация пользователя»
    atm.setState(std::make_shared<AuthenticationState>());

    // Проверка наличия связи с банком
    if (!atm.checkBankConnection()) {
        return {
            false,
            "Сбой связи с банком при проверке PIN-кода. Автомат ожидает повторного ввода PIN или завершения сеанса.",
            prevState,
            atm.getStateName()
        };
    }

    // Проверка корректности введённого PIN-кода
    if (!atm.verifyPin(pin)) {
        return {
            false,
            "Неверный PIN-код («" + pin + "»). Повторите ввод PIN-кода или завершите работу.",
            prevState,
            atm.getStateName()
        };
    }

    // При наличии связи и верном PIN-коде автомат переходит к выполнению операций
    atm.setState(std::make_shared<PerformingOperationsState>());
    return {
        true,
        "Аутентификация пройдена успешно. Доступно выполнение операций.",
        prevState,
        atm.getStateName()
    };
}

OperationResult WaitingState::withdrawCash(ATM& atm, int /*amount*/) {
    return {
        false,
        "Отказ: невозможно снять наличные в режиме ожидания. Сначала введите PIN-код.",
        getName(),
        atm.getStateName()
    };
}

OperationResult WaitingState::finishSession(ATM& atm) {
    return {
        false,
        "Отказ: активный сеанс пользователя отсутствует (банкомат уже в режиме ожидания).",
        getName(),
        atm.getStateName()
    };
}

OperationResult WaitingState::loadCash(ATM& atm, int amount) {
    const std::string prevState = getName();
    if (amount <= 0) {
        return {
            false,
            "Отказ: сумма загрузки должна быть строго положительной.",
            prevState,
            atm.getStateName()
        };
    }

    atm.addCash(amount);
    return {
        true,
        "Инкассация выполнена: загружено " + std::to_string(amount) +
            " руб. Текущий остаток: " + std::to_string(atm.getTotalCash()) + " руб.",
        prevState,
        atm.getStateName()
    };
}

// ============================================================================
// 2. Состояние «Аутентификация пользователя» (AuthenticationState)
// ============================================================================

std::string AuthenticationState::getName() const {
    return "AuthenticationState (Аутентификация пользователя)";
}

OperationResult AuthenticationState::enterPin(ATM& atm, const std::string& pin) {
    const std::string prevState = getName();

    // Проверка связи с банком при повторной попытке аутентификации
    if (!atm.checkBankConnection()) {
        return {
            false,
            "Сбой связи с банком при повторной проверке PIN-кода. Автомат остаётся в состоянии аутентификации.",
            prevState,
            atm.getStateName()
        };
    }

    // Проверка корректности PIN-кода
    if (!atm.verifyPin(pin)) {
        return {
            false,
            "Повторный ввод: неверный PIN-код («" + pin + "»). Автомат остаётся в состоянии аутентификации.",
            prevState,
            atm.getStateName()
        };
    }

    // Успешная аутентификация -> переход в состояние «Выполнение операций»
    atm.setState(std::make_shared<PerformingOperationsState>());
    return {
        true,
        "Повторный ввод PIN-кода успешен. Пользователь аутентифицирован.",
        prevState,
        atm.getStateName()
    };
}

OperationResult AuthenticationState::withdrawCash(ATM& atm, int /*amount*/) {
    return {
        false,
        "Отказ: аутентификация пользователя ещё не завершена. Введите корректный PIN-код.",
        getName(),
        atm.getStateName()
    };
}

OperationResult AuthenticationState::finishSession(ATM& atm) {
    const std::string prevState = getName();
    atm.setState(std::make_shared<WaitingState>());
    return {
        true,
        "Процесс аутентификации прерван пользователем. Возврат в режим ожидания.",
        prevState,
        atm.getStateName()
    };
}

OperationResult AuthenticationState::loadCash(ATM& atm, int /*amount*/) {
    return {
        false,
        "Отказ: загрузка наличных запрещена во время активного пользовательского сеанса.",
        getName(),
        atm.getStateName()
    };
}

// ============================================================================
// 3. Состояние «Выполнение операций» (PerformingOperationsState)
// ============================================================================

std::string PerformingOperationsState::getName() const {
    return "PerformingOperationsState (Выполнение операций)";
}

OperationResult PerformingOperationsState::enterPin(ATM& atm, const std::string& /*pin*/) {
    return {
        false,
        "Предупреждение: пользователь уже аутентифицирован и находится в режиме выполнения операций.",
        getName(),
        atm.getStateName()
    };
}

OperationResult PerformingOperationsState::withdrawCash(ATM& atm, int amount) {
    const std::string prevState = getName();

    if (amount <= 0) {
        return {
            false,
            "Отказ: запрашиваемая сумма для снятия должна быть больше нуля.",
            prevState,
            atm.getStateName()
        };
    }

    // Проверка наличия связи с банком для авторизации транзакции
    if (!atm.checkBankConnection()) {
        return {
            false,
            "Ошибка транзакции: отсутствует связь с банком. Выдача наличных отклонена.",
            prevState,
            atm.getStateName()
        };
    }

    // Проверка достаточности средств в банкомате
    if (amount > atm.getTotalCash()) {
        return {
            false,
            "Отказ: в банкомате недостаточно средств (запрошено " + std::to_string(amount) +
                " руб., доступно " + std::to_string(atm.getTotalCash()) + " руб.).",
            prevState,
            atm.getStateName()
        };
    }

    // Выдача наличных
    atm.deductCash(amount);

    // Если в банкомате закончились деньги, он переходит в состояние «Заблокирован (денег нет)»
    if (atm.getTotalCash() == 0) {
        atm.setState(std::make_shared<BlockedState>());
        return {
            true,
            "Выдано " + std::to_string(amount) +
                " руб. Наличные в банкомате исчерпаны (остаток 0 руб.) — банкомат переведён в состояние блокировки.",
            prevState,
            atm.getStateName()
        };
    }

    return {
        true,
        "Выдано " + std::to_string(amount) +
            " руб. Остаток средств в банкомате: " + std::to_string(atm.getTotalCash()) + " руб.",
        prevState,
        atm.getStateName()
    };
}

OperationResult PerformingOperationsState::finishSession(ATM& atm) {
    const std::string prevState = getName();
    atm.setState(std::make_shared<WaitingState>());
    return {
        true,
        "Сеанс обслуживания завершён. Карта возвращена клиенту, банкомат переведён в режим ожидания.",
        prevState,
        atm.getStateName()
    };
}

OperationResult PerformingOperationsState::loadCash(ATM& atm, int /*amount*/) {
    return {
        false,
        "Отказ: инкассация невозможна во время обслуживания клиента. Сначала завершите работу.",
        getName(),
        atm.getStateName()
    };
}

// ============================================================================
// 4. Состояние «Заблокирован (денег нет)» (BlockedState)
// ============================================================================

std::string BlockedState::getName() const {
    return "BlockedState (Заблокирован: денег нет)";
}

OperationResult BlockedState::enterPin(ATM& atm, const std::string& /*pin*/) {
    return {
        false,
        "Отказ: банкомат заблокирован (нет наличных денег). Обслуживание временно недоступно.",
        getName(),
        atm.getStateName()
    };
}

OperationResult BlockedState::withdrawCash(ATM& atm, int /*amount*/) {
    return {
        false,
        "Отказ: банкомат заблокирован (в кассетах 0 руб.). Выдача невозможна.",
        getName(),
        atm.getStateName()
    };
}

OperationResult BlockedState::finishSession(ATM& atm) {
    return {
        false,
        "Банкомат заблокирован (денег нет). Ожидается загрузка наличных службой инкассации.",
        getName(),
        atm.getStateName()
    };
}

OperationResult BlockedState::loadCash(ATM& atm, int amount) {
    const std::string prevState = getName();
    if (amount <= 0) {
        return {
            false,
            "Отказ: сумма загрузки для разблокировки должна быть больше нуля.",
            prevState,
            atm.getStateName()
        };
    }

    atm.addCash(amount);
    atm.setState(std::make_shared<WaitingState>());
    return {
        true,
        "Загружено " + std::to_string(amount) +
            " руб. Банкомат разблокирован и переведён в состояние ожидания.",
        prevState,
        atm.getStateName()
    };
}

// ============================================================================
// 5. Класс конечного автомата «Банкомат» (ATM — Context)
// ============================================================================

ATM::ATM(std::string id,
         int initialCash,
         double connectionLossProb,
         std::string validPin,
         unsigned int randomSeed)
    : id_(std::move(id)),
      totalCash_(std::max(0, initialCash)),
      connectionLossProbability_(std::clamp(connectionLossProb, 0.0, 1.0)),
      validPin_(std::move(validPin)),
      rng_(randomSeed) {
    // Если изначально денег нет, автомат стартует в состоянии «Заблокирован (денег нет)»
    if (totalCash_ > 0) {
        currentState_ = std::make_shared<WaitingState>();
    } else {
        currentState_ = std::make_shared<BlockedState>();
    }
    historyLog_.push_back("Инициализация банкомата [" + id_ + "]: остаток = " +
                          std::to_string(totalCash_) + " руб., состояние = " + getStateName());
}

OperationResult ATM::enterPin(const std::string& pin) {
    OperationResult res = currentState_->enterPin(*this, pin);
    recordLog("enterPin(\"" + pin + "\")", res);
    return res;
}

OperationResult ATM::withdrawCash(int amount) {
    OperationResult res = currentState_->withdrawCash(*this, amount);
    recordLog("withdrawCash(" + std::to_string(amount) + ")", res);
    return res;
}

OperationResult ATM::finishSession() {
    OperationResult res = currentState_->finishSession(*this);
    recordLog("finishSession()", res);
    return res;
}

OperationResult ATM::loadCash(int amount) {
    OperationResult res = currentState_->loadCash(*this, amount);
    recordLog("loadCash(" + std::to_string(amount) + ")", res);
    return res;
}

void ATM::setState(std::shared_ptr<ATMState> newState) {
    if (newState) {
        currentState_ = std::move(newState);
    }
}

std::shared_ptr<ATMState> ATM::getState() const {
    return currentState_;
}

std::string ATM::getStateName() const {
    return currentState_ ? currentState_->getName() : "Unknown";
}

bool ATM::checkBankConnection() {
    if (connectionLossProbability_ <= 0.0) {
        return true;
    }
    if (connectionLossProbability_ >= 1.0) {
        return false;
    }
    std::uniform_real_distribution<double> dist(0.0, 1.0);
    double sample = dist(rng_);
    return sample >= connectionLossProbability_;
}

bool ATM::verifyPin(const std::string& pin) const {
    return pin == validPin_;
}

void ATM::addCash(int amount) {
    if (amount > 0) {
        totalCash_ += amount;
    }
}

void ATM::deductCash(int amount) {
    if (amount > 0 && amount <= totalCash_) {
        totalCash_ -= amount;
    }
}

const std::string& ATM::getId() const {
    return id_;
}

int ATM::getTotalCash() const {
    return totalCash_;
}

double ATM::getConnectionLossProbability() const {
    return connectionLossProbability_;
}

void ATM::setConnectionLossProbability(double probability) {
    connectionLossProbability_ = std::clamp(probability, 0.0, 1.0);
}

const std::string& ATM::getValidPin() const {
    return validPin_;
}

const std::vector<std::string>& ATM::getHistoryLog() const {
    return historyLog_;
}

std::string ATM::getStatusSummary() const {
    std::ostringstream oss;
    oss << "Банкомат [ID: " << id_ << "] | Состояние: " << getStateName()
        << " | Наличные: " << totalCash_ << " руб."
        << " | Вероятность обрыва связи: " << std::fixed << std::setprecision(2)
        << connectionLossProbability_;
    return oss.str();
}

void ATM::recordLog(const std::string& operation, const OperationResult& result) {
    std::ostringstream oss;
    oss << "[" << operation << "] "
        << result.previousState << " -> " << result.currentState
        << " | " << (result.success ? "OK: " : "ОТКАЗ: ") << result.message;
    historyLog_.push_back(oss.str());
}

} // namespace atm

