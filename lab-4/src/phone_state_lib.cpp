#include "phone_state_lib.hpp"

#include <algorithm>
#include <iomanip>
#include <sstream>

namespace phone {

namespace {

/// Переводит телефон в состояние разговора по входящему (бесплатному) звонку
OperationResult acceptIncomingCall(Phone& phone,
                                   const std::string& prevState,
                                   const std::string& message) {
    phone.setIncomingCall(true);
    phone.setActivePeerNumber("Входящий абонент");
    phone.setState(std::make_shared<TalkingState>());
    return {true, message, prevState, phone.getStateName()};
}

} // namespace

// ============================================================================
// 1. Состояние «Ожидание» (WaitingState)
// ============================================================================

std::string WaitingState::getName() const {
    return "WaitingState (Ожидание)";
}

OperationResult WaitingState::makeCall(Phone& phone, const std::string& targetNumber) {
    const std::string prevState = getName();

    if (targetNumber.empty()) {
        return {
            false,
            "Отказ: номер вызываемого абонента не может быть пустым.",
            prevState,
            phone.getStateName()
        };
    }

    if (phone.getBalance() < 0) {
        phone.setState(std::make_shared<BlockedState>());
        return {
            false,
            "Отказ: баланс отрицательный (" + std::to_string(phone.getBalance()) +
                " руб.). Исходящие вызовы заблокированы.",
            prevState,
            phone.getStateName()
        };
    }

    phone.setIncomingCall(false);
    phone.setActivePeerNumber(targetNumber);
    phone.setState(std::make_shared<CallingState>());
    return {
        true,
        "Инициирован исходящий звонок абоненту «" + targetNumber + "». Идут гудки вызова.",
        prevState,
        phone.getStateName()
    };
}

OperationResult WaitingState::answerCall(Phone& phone) {
    return acceptIncomingCall(
        phone,
        getName(),
        "Принят входящий звонок. Установлено голосовое соединение (входящие вызовы бесплатны)."
    );
}

OperationResult WaitingState::endCall(Phone& phone) {
    return {
        false,
        "Отказ: нет активного вызова или разговора для завершения.",
        getName(),
        phone.getStateName()
    };
}

OperationResult WaitingState::topUpBalance(Phone& phone, int amount) {
    const std::string prevState = getName();
    if (amount <= 0) {
        return {
            false,
            "Отказ: сумма пополнения баланса должна быть строго положительной.",
            prevState,
            phone.getStateName()
        };
    }

    phone.addBalance(amount);
    return {
        true,
        "Баланс пополнен на " + std::to_string(amount) + " руб. Текущий баланс: " +
            std::to_string(phone.getBalance()) + " руб.",
        prevState,
        phone.getStateName()
    };
}

// ============================================================================
// 2. Состояние «Звонок» (CallingState)
// ============================================================================

std::string CallingState::getName() const {
    return "CallingState (Звонок)";
}

OperationResult CallingState::makeCall(Phone& phone, const std::string& /*targetNumber*/) {
    return {
        false,
        "Отказ: телефон уже находится в состоянии вызова абонента «" +
            phone.getActivePeerNumber() + "».",
        getName(),
        phone.getStateName()
    };
}

OperationResult CallingState::answerCall(Phone& phone) {
    const std::string prevState = getName();
    phone.setState(std::make_shared<TalkingState>());
    return {
        true,
        "Ответ на звонок принят. Установлено голосовое соединение с абонентом «" +
            phone.getActivePeerNumber() + "».",
        prevState,
        phone.getStateName()
    };
}

OperationResult CallingState::endCall(Phone& phone) {
    const std::string prevState = getName();
    const std::string peer = phone.getActivePeerNumber();
    phone.setActivePeerNumber("");
    phone.setIncomingCall(false);
    if (phone.getBalance() >= 0) {
        phone.setState(std::make_shared<WaitingState>());
    } else {
        phone.setState(std::make_shared<BlockedState>());
    }
    return {
        true,
        "Вызов абонента «" + peer + "» сброшен до начала разговора. Переход в состояние " +
            phone.getStateName() + ".",
        prevState,
        phone.getStateName()
    };
}

OperationResult CallingState::topUpBalance(Phone& phone, int amount) {
    const std::string prevState = getName();
    if (amount <= 0) {
        return {
            false,
            "Отказ: сумма пополнения должна быть строго больше нуля.",
            prevState,
            phone.getStateName()
        };
    }

    phone.addBalance(amount);
    return {
        true,
        "Баланс пополнен на " + std::to_string(amount) + " руб. во время вызова. Текущий баланс: " +
            std::to_string(phone.getBalance()) + " руб.",
        prevState,
        phone.getStateName()
    };
}

// ============================================================================
// 3. Состояние «Разговор» (TalkingState)
// ============================================================================

std::string TalkingState::getName() const {
    return "TalkingState (Разговор)";
}

OperationResult TalkingState::makeCall(Phone& phone, const std::string& /*targetNumber*/) {
    return {
        false,
        "Отказ: линия занята — уже идёт разговор с абонентом «" +
            phone.getActivePeerNumber() + "».",
        getName(),
        phone.getStateName()
    };
}

OperationResult TalkingState::answerCall(Phone& phone) {
    return {
        false,
        "Предупреждение: соединение с абонентом «" + phone.getActivePeerNumber() +
            "» уже установлено, идёт разговор.",
        getName(),
        phone.getStateName()
    };
}

OperationResult TalkingState::endCall(Phone& phone) {
    const std::string prevState = getName();
    const std::string peer = phone.getActivePeerNumber();
    const bool incoming = phone.isIncomingCall();
    const int cost = phone.getCallCost();

    phone.setActivePeerNumber("");
    phone.setIncomingCall(false);

    if (incoming) {
        if (phone.getBalance() < 0) {
            phone.setState(std::make_shared<BlockedState>());
            return {
                true,
                "Входящий разговор («" + peer +
                    "») завершён (0 руб. — списание происходит со счёта звонящего). Баланс отрицательный (" +
                    std::to_string(phone.getBalance()) +
                    " руб.) — исходящие звонки по-прежнему заблокированы.",
                prevState,
                phone.getStateName()
            };
        }

        phone.setState(std::make_shared<WaitingState>());
        return {
            true,
            "Входящий разговор («" + peer +
                "») завершён (0 руб. — списание происходит со счёта звонящего). Остаток на счёте: " +
                std::to_string(phone.getBalance()) + " руб.",
            prevState,
            phone.getStateName()
        };
    }

    phone.deductCallCost();

    if (phone.getBalance() < 0) {
        phone.setState(std::make_shared<BlockedState>());
        return {
            true,
            "Разговор с «" + peer + "» завершён. Списано " + std::to_string(cost) +
                " руб. Баланс стал отрицательным (" + std::to_string(phone.getBalance()) +
                " руб.) — исходящие вызовы заблокированы!",
            prevState,
            phone.getStateName()
        };
    }

    phone.setState(std::make_shared<WaitingState>());
    return {
        true,
        "Разговор с «" + peer + "» завершён. Списано " + std::to_string(cost) +
            " руб. Остаток на счёте: " + std::to_string(phone.getBalance()) + " руб.",
        prevState,
        phone.getStateName()
    };
}

OperationResult TalkingState::topUpBalance(Phone& phone, int amount) {
    const std::string prevState = getName();
    if (amount <= 0) {
        return {
            false,
            "Отказ: сумма пополнения должна быть строго больше нуля.",
            prevState,
            phone.getStateName()
        };
    }

    phone.addBalance(amount);
    return {
        true,
        "Баланс пополнен на " + std::to_string(amount) +
            " руб. во время разговора. Текущий баланс: " +
            std::to_string(phone.getBalance()) + " руб.",
        prevState,
        phone.getStateName()
    };
}

// ============================================================================
// 4. Состояние «Заблокирован (баланс отрицательный)» (BlockedState)
// ============================================================================

std::string BlockedState::getName() const {
    return "BlockedState (Заблокирован: баланс отрицательный)";
}

OperationResult BlockedState::makeCall(Phone& phone, const std::string& /*targetNumber*/) {
    return {
        false,
        "Отказ: исходящие вызовы заблокированы из-за отрицательного баланса (" +
            std::to_string(phone.getBalance()) +
            " руб.). Пополните счёт (приём входящих звонков доступен).",
        getName(),
        phone.getStateName()
    };
}

OperationResult BlockedState::answerCall(Phone& phone) {
    return acceptIncomingCall(
        phone,
        getName(),
        "Принят входящий звонок при отрицательном балансе (" +
            std::to_string(phone.getBalance()) +
            " руб.). Входящие вызовы бесплатны — установлено соединение."
    );
}

OperationResult BlockedState::endCall(Phone& phone) {
    return {
        false,
        "Отказ: телефон заблокирован для исходящих, активных вызовов нет.",
        getName(),
        phone.getStateName()
    };
}

OperationResult BlockedState::topUpBalance(Phone& phone, int amount) {
    const std::string prevState = getName();
    if (amount <= 0) {
        return {
            false,
            "Отказ: сумма пополнения должна быть строго больше нуля.",
            prevState,
            phone.getStateName()
        };
    }

    phone.addBalance(amount);
    if (phone.getBalance() >= 0) {
        phone.setState(std::make_shared<WaitingState>());
        return {
            true,
            "Счёт пополнен на " + std::to_string(amount) + " руб. (баланс: " +
                std::to_string(phone.getBalance()) +
                " руб. >= 0). Исходящие вызовы разблокированы, переход в режим ожидания.",
            prevState,
            phone.getStateName()
        };
    }

    return {
        true,
        "Счёт пополнен на " + std::to_string(amount) + " руб., но баланс всё ещё отрицательный (" +
            std::to_string(phone.getBalance()) + " руб.). Исходящие вызовы по-прежнему заблокированы.",
        prevState,
        phone.getStateName()
    };
}

// ============================================================================
// 5. Класс контекста «Телефон» (Phone)
// ============================================================================

Phone::Phone(std::string phoneNumber,
             int initialBalance,
             double callProbability,
             int callCost)
    : phoneNumber_(std::move(phoneNumber)),
      balance_(initialBalance),
      callProbability_(std::clamp(callProbability, 0.0, 1.0)),
      callCost_(std::max(1, callCost)),
      activePeerNumber_(""),
      isIncomingCall_(false) {
    if (balance_ >= 0) {
        currentState_ = std::make_shared<WaitingState>();
    } else {
        currentState_ = std::make_shared<BlockedState>();
    }

    historyLog_.push_back(
        "[Инициализация] Телефон «" + phoneNumber_ + "» создан. Начальный баланс: " +
        std::to_string(balance_) + " руб., вероятность поступления звонка: " +
        std::to_string(callProbability_) + ", тариф: " + std::to_string(callCost_) +
        " руб./разговор. Начальное состояние: " + currentState_->getName());
}

void Phone::recordHistory(const OperationResult& result, const std::string& operationDesc) {
    std::ostringstream oss;
    oss << "[" << (result.success ? "ОК" : "ОТКАЗ") << "] " << operationDesc << " | "
        << result.previousState << " -> " << result.currentState << " | " << result.message;
    historyLog_.push_back(oss.str());
}

OperationResult Phone::makeCall(const std::string& targetNumber) {
    OperationResult res = currentState_->makeCall(*this, targetNumber);
    recordHistory(res, "makeCall(\"" + targetNumber + "\")");
    return res;
}

OperationResult Phone::answerCall() {
    OperationResult res = currentState_->answerCall(*this);
    recordHistory(res, "answerCall()");
    return res;
}

OperationResult Phone::endCall() {
    OperationResult res = currentState_->endCall(*this);
    recordHistory(res, "endCall()");
    return res;
}

OperationResult Phone::topUpBalance(int amount) {
    OperationResult res = currentState_->topUpBalance(*this, amount);
    recordHistory(res, "topUpBalance(" + std::to_string(amount) + ")");
    return res;
}

void Phone::setState(std::shared_ptr<PhoneState> newState) {
    if (newState) {
        currentState_ = std::move(newState);
    }
}

std::shared_ptr<PhoneState> Phone::getState() const {
    return currentState_;
}

std::string Phone::getStateName() const {
    return currentState_ ? currentState_->getName() : "Unknown";
}

void Phone::addBalance(int amount) {
    if (amount > 0) {
        balance_ += amount;
    }
}

void Phone::deductCallCost() {
    balance_ -= callCost_;
}

const std::string& Phone::getPhoneNumber() const {
    return phoneNumber_;
}

int Phone::getBalance() const {
    return balance_;
}

double Phone::getCallProbability() const {
    return callProbability_;
}

void Phone::setCallProbability(double probability) {
    callProbability_ = std::clamp(probability, 0.0, 1.0);
}

int Phone::getCallCost() const {
    return callCost_;
}

void Phone::setCallCost(int cost) {
    callCost_ = std::max(1, cost);
}

const std::string& Phone::getActivePeerNumber() const {
    return activePeerNumber_;
}

void Phone::setActivePeerNumber(const std::string& peerNumber) {
    activePeerNumber_ = peerNumber;
}

bool Phone::isIncomingCall() const {
    return isIncomingCall_;
}

void Phone::setIncomingCall(bool incoming) {
    isIncomingCall_ = incoming;
}

const std::vector<std::string>& Phone::getHistoryLog() const {
    return historyLog_;
}

std::string Phone::getStatusSummary() const {
    std::ostringstream oss;
    oss << "Телефон [Номер: " << phoneNumber_ << " | Состояние: " << getStateName()
        << " | Баланс: " << balance_ << " руб. | Вероятность поступления звонка: " << std::fixed
        << std::setprecision(2) << callProbability_ << " | Тариф: " << callCost_ << " руб.]";
    return oss.str();
}

} // namespace phone

