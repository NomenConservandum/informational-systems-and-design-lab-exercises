/**
 * @file main.cpp
 * @brief Консольное приложение для управления состояниями экземпляров
 *        класса «Конечный автомат» (Phone / «Телефон») — Вариант 5/1.
 *
 * Поддерживает два режима работы:
 * 1) Автоматический демонстрационный сценарий (`./build/phone_app --demo`),
 *    последовательно проверяющий все состояния, переходы и реакции автомата.
 * 2) Интерактивное консольное меню (`./build/phone_app`) для создания нескольких
 *    экземпляров класса Phone и ручного вызова операций.
 */

#include "phone_state_lib.hpp"

#include <iomanip>
#include <iostream>
#include <limits>
#include <string>
#include <vector>

namespace {

using phone::OperationResult;
using phone::Phone;

/// Печать результата одиночной операции
void printOperationReport(int stepNumber,
                          const std::string& actionTitle,
                          const OperationResult& res,
                          const Phone& device) {
    std::cout << "--------------------------------------------------------------------------------\n";
    if (stepNumber > 0) {
        std::cout << "Шаг " << stepNumber << ". ";
    }
    std::cout << "Операция: " << actionTitle << "\n";
    std::cout << "  Статус выполнения : " << (res.success ? "УСПЕШНО" : "ОТКЛОНЕНО / ОШИБКА") << "\n";
    std::cout << "  Переход состояний : " << res.previousState << "  -->  " << res.currentState << "\n";
    std::cout << "  Реакция автомата  : " << res.message << "\n";
    std::cout << "  Текущий баланс    : " << device.getBalance() << " руб.\n";
}

/**
 * @brief Класс консольного контроллера для управления экземплярами класса Phone.
 */
class PhoneControllerApp {
private:
    std::vector<Phone> phones_;
    std::size_t activeIdx_ = 0;

public:
    PhoneControllerApp() {
        phones_.emplace_back("+7-917-100-01-01", 100, 1.00, 50, 101);
        phones_.emplace_back("+7-917-200-02-02", 0, 0.80, 50, 202);
    }

    /// Запуск полного демонстрационного сценария (для отчёта и проверки)
    void runDemonstrationScenario() {
        std::cout << "================================================================================\n";
        std::cout << "  ДЕМОНСТРАЦИЯ РАБОТЫ ШАБЛОНА «СОСТОЯНИЕ» (STATE) — СИСТЕМА «ТЕЛЕФОН» (ВАР. 5/1)\n";
        std::cout << "================================================================================\n\n";

        Phone demoPhone("+7-917-555-01-01", 100, 1.0, 50, 42);
        std::cout << "Создан экземпляр конечного автомата:\n  " << demoPhone.getStatusSummary() << "\n\n";

        int step = 1;

        // 1. Попытка ответить на звонок в режиме ожидания
        printOperationReport(step++, "answerCall() — попытка ответить без входящего вызова",
                             demoPhone.answerCall(), demoPhone);

        // 2. Попытка завершить разговор в режиме ожидания
        printOperationReport(step++, "endCall() — попытка завершить несуществующий разговор",
                             demoPhone.endCall(), demoPhone);

        // 3. Пополнение баланса в состоянии ожидания
        printOperationReport(step++, "topUpBalance(50) — пополнение счёта в режиме ожидания",
                             demoPhone.topUpBalance(50), demoPhone);

        // 4. Сбой вызова при вероятности дозвона p = 0.0
        demoPhone.setCallProbability(0.0);
        printOperationReport(step++, "makeCall(\"+7-900-111-22-33\") при p = 0.0 (сбой вызова)",
                             demoPhone.makeCall("+7-900-111-22-33"), demoPhone);

        // 5. Успешный вызов при p = 1.0 (WaitingState -> CallingState)
        demoPhone.setCallProbability(1.0);
        printOperationReport(step++, "makeCall(\"+7-900-111-22-33\") при p = 1.0 (начало звонка)",
                             demoPhone.makeCall("+7-900-111-22-33"), demoPhone);

        // 6. Попытка позвонить повторно во время активного гудка
        printOperationReport(step++, "makeCall(\"+7-900-999-88-77\") во время активного звонка",
                             demoPhone.makeCall("+7-900-999-88-77"), demoPhone);

        // 7. Сброс звонка до ответа (CallingState -> WaitingState)
        printOperationReport(step++, "endCall() — отмена/сброс вызова до ответа собеседника",
                             demoPhone.endCall(), demoPhone);

        // 8. Повторный набор номера (WaitingState -> CallingState)
        printOperationReport(step++, "makeCall(\"+7-900-111-22-33\") — повторный вызов",
                             demoPhone.makeCall("+7-900-111-22-33"), demoPhone);

        // 9. Ответ на звонок (CallingState -> TalkingState)
        printOperationReport(step++, "answerCall() — собеседник ответил, начало разговора",
                             demoPhone.answerCall(), demoPhone);

        // 10. Завершение первого разговора (списание 50 руб., остаток 100 руб. > 0 -> WaitingState)
        printOperationReport(step++, "endCall() — завершение 1-го разговора (списание 50 руб.)",
                             demoPhone.endCall(), demoPhone);

        // 11. Новый звонок, ответ и завершение с обнулением баланса (стоимость 100 руб. -> BlockedState)
        demoPhone.setCallCost(100);
        demoPhone.makeCall("+7-900-444-55-66");
        demoPhone.answerCall();
        printOperationReport(step++, "endCall() при тарифе 100 руб. (исчерпание баланса -> блокировка)",
                             demoPhone.endCall(), demoPhone);

        // 12. Попытка позвонить с заблокированного телефона
        printOperationReport(step++, "makeCall(\"+7-900-111-22-33\") в состоянии BlockedState",
                             demoPhone.makeCall("+7-900-111-22-33"), demoPhone);

        // 13. Попытка ответить на звонок в состоянии BlockedState
        printOperationReport(step++, "answerCall() в состоянии BlockedState",
                             demoPhone.answerCall(), demoPhone);

        // 14. Пополнение баланса для разблокировки (BlockedState -> WaitingState)
        demoPhone.setCallCost(50);
        printOperationReport(step++, "topUpBalance(200) — пополнение счёта и разблокировка",
                             demoPhone.topUpBalance(200), demoPhone);

        // 15. Контрольный полный цикл звонка после разблокировки
        demoPhone.makeCall("+7-900-777-00-00");
        demoPhone.answerCall();
        printOperationReport(step++, "Контрольный звонок + разговор + endCall() после разблокировки",
                             demoPhone.endCall(), demoPhone);

        std::cout << "================================================================================\n";
        std::cout << "Итоговое состояние автомата:\n  " << demoPhone.getStatusSummary() << "\n";
        std::cout << "================================================================================\n";
    }

    /// Интерактивное меню для ручного управления экземплярами класса Phone
    void runInteractiveMenu() {
        while (true) {
            const Phone& current = phones_.at(activeIdx_);
            std::cout << "\n================================================================================\n";
            std::cout << " УПРАВЛЕНИЕ КОНЕЧНЫМИ АВТОМАТАМИ «ТЕЛЕФОН» (ПАТТЕРН «СОСТОЯНИЕ»)\n";
            std::cout << "================================================================================\n";
            std::cout << " Активный телефон [" << activeIdx_ << "]: " << current.getStatusSummary() << "\n";
            std::cout << "--------------------------------------------------------------------------------\n";
            std::cout << " 1. Позвонить (makeCall)\n";
            std::cout << " 2. Ответить на звонок (answerCall)\n";
            std::cout << " 3. Завершить разговор / сбросить вызов (endCall)\n";
            std::cout << " 4. Пополнить баланс (topUpBalance)\n";
            std::cout << " 5. Изменить вероятность успешного вызова (setCallProbability)\n";
            std::cout << " 6. Показать журнал переходов текущего телефона\n";
            std::cout << " 7. Список всех телефонов / переключить активный телефон\n";
            std::cout << " 8. Добавить новый экземпляр телефона\n";
            std::cout << " 9. Запустить автоматический демонстрационный сценарий\n";
            std::cout << " 0. Выход\n";
            std::cout << "--------------------------------------------------------------------------------\n";
            std::cout << "Выберите пункт меню: ";

            int choice = -1;
            if (!(std::cin >> choice)) {
                if (std::cin.eof()) {
                    std::cout << "\nЗавершение работы.\n";
                    break;
                }
                std::cin.clear();
                std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                std::cout << "Некорректный ввод.\n";
                continue;
            }

            if (choice == 0) {
                std::cout << "Выход из программы.\n";
                break;
            }

            Phone& activePhone = phones_.at(activeIdx_);
            switch (choice) {
                case 1: {
                    std::cout << "Введите номер вызываемого абонента: ";
                    std::string target;
                    std::cin >> target;
                    auto res = activePhone.makeCall(target);
                    printOperationReport(0, "makeCall(\"" + target + "\")", res, activePhone);
                    break;
                }
                case 2: {
                    auto res = activePhone.answerCall();
                    printOperationReport(0, "answerCall()", res, activePhone);
                    break;
                }
                case 3: {
                    auto res = activePhone.endCall();
                    printOperationReport(0, "endCall()", res, activePhone);
                    break;
                }
                case 4: {
                    std::cout << "Введите сумму пополнения (руб.): ";
                    int amount = 0;
                    std::cin >> amount;
                    auto res = activePhone.topUpBalance(amount);
                    printOperationReport(0, "topUpBalance(" + std::to_string(amount) + ")", res, activePhone);
                    break;
                }
                case 5: {
                    std::cout << "Введите вероятность дозвона [0.0 .. 1.0]: ";
                    double p = 1.0;
                    std::cin >> p;
                    activePhone.setCallProbability(p);
                    std::cout << "Установлена вероятность: " << activePhone.getCallProbability() << "\n";
                    break;
                }
                case 6: {
                    std::cout << "\n--- Журнал переходов телефона " << activePhone.getPhoneNumber() << " ---\n";
                    for (const auto& entry : activePhone.getHistoryLog()) {
                        std::cout << "  " << entry << "\n";
                    }
                    break;
                }
                case 7: {
                    std::cout << "\nДоступные экземпляры класса Phone:\n";
                    for (std::size_t i = 0; i < phones_.size(); ++i) {
                        std::cout << "  [" << i << "] " << phones_[i].getStatusSummary()
                                  << (i == activeIdx_ ? "  <-- АКТИВНЫЙ" : "") << "\n";
                    }
                    std::cout << "Введите индекс телефона для переключения: ";
                    std::size_t idx = 0;
                    std::cin >> idx;
                    if (idx < phones_.size()) {
                        activeIdx_ = idx;
                    } else {
                        std::cout << "Некорректный индекс.\n";
                    }
                    break;
                }
                case 8: {
                    std::string number;
                    int balance = 0;
                    double prob = 1.0;
                    std::cout << "Введите номер нового телефона: ";
                    std::cin >> number;
                    std::cout << "Введите начальный баланс (руб.): ";
                    std::cin >> balance;
                    std::cout << "Введите вероятность вызова [0.0 .. 1.0]: ";
                    std::cin >> prob;
                    phones_.emplace_back(number, balance, prob);
                    activeIdx_ = phones_.size() - 1;
                    std::cout << "Создан и выбран телефон: " << phones_.back().getStatusSummary() << "\n";
                    break;
                }
                case 9: {
                    runDemonstrationScenario();
                    break;
                }
                default:
                    std::cout << "Неизвестная команда.\n";
                    break;
            }
        }
    }
};

} // namespace

int main(int argc, char* argv[]) {
    PhoneControllerApp app;
    if (argc > 1 && std::string(argv[1]) == "--demo") {
        app.runDemonstrationScenario();
        return 0;
    }
    app.runInteractiveMenu();
    return 0;
}
