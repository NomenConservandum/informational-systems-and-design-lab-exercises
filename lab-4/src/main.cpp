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
        phones_.emplace_back("+7-917-100-01-01", 100, 1.00, 50);
        phones_.emplace_back("+7-917-200-02-02", -20, 0.80, 50);
    }

    /// Запуск полного демонстрационного сценария (для отчёта и проверки)
    void runDemonstrationScenario() {
        std::cout << "================================================================================\n";
        std::cout << "  ДЕМОНСТРАЦИЯ РАБОТЫ ШАБЛОНА «СОСТОЯНИЕ» (STATE) — СИСТЕМА «ТЕЛЕФОН» (ВАР. 5/1)\n";
        std::cout << "================================================================================\n\n";

        Phone demoPhone("+7-917-555-01-01", 100, 1.0, 50);
        std::cout << "Создан экземпляр конечного автомата:\n  " << demoPhone.getStatusSummary() << "\n\n";

        int step = 1;

        // 1. Попытка завершить несуществующий разговор в режиме ожидания
        printOperationReport(step++, "endCall() — попытка завершить несуществующий разговор",
                             demoPhone.endCall(), demoPhone);

        // 2. Пополнение баланса в состоянии ожидания
        printOperationReport(step++, "topUpBalance(50) — пополнение счёта в режиме ожидания",
                             demoPhone.topUpBalance(50), demoPhone);

        // 3. Приём бесплатного входящего звонка в WaitingState (WaitingState -> TalkingState)
        printOperationReport(step++, "answerCall() — приём входящего звонка в WaitingState",
                             demoPhone.answerCall(), demoPhone);

        // 4. Завершение входящего разговора без списания (TalkingState -> WaitingState, баланс 150 руб.)
        printOperationReport(step++, "endCall() — завершение бесплатного входящего разговора",
                             demoPhone.endCall(), demoPhone);

        // 5. Инициирование исходящего вызова (WaitingState -> CallingState)
        printOperationReport(step++, "makeCall(\"+7-900-111-22-33\") — начало исходящего звонка",
                             demoPhone.makeCall("+7-900-111-22-33"), demoPhone);

        // 6. Попытка позвонить повторно во время активного гудка
        printOperationReport(step++, "makeCall(\"+7-900-999-88-77\") во время активного звонка",
                             demoPhone.makeCall("+7-900-999-88-77"), demoPhone);

        // 7. Сброс вызова до ответа (CallingState -> WaitingState)
        printOperationReport(step++, "endCall() — отмена/сброс вызова до ответа собеседника",
                             demoPhone.endCall(), demoPhone);

        // 8. Повторный набор + ответ собеседника (WaitingState -> CallingState -> TalkingState)
        demoPhone.makeCall("+7-900-111-22-33");
        printOperationReport(step++, "makeCall() + answerCall() — соединение по исходящему звонку",
                             demoPhone.answerCall(), demoPhone);

        // 9. Завершение исходящего разговора до ровно нулевого баланса (тариф 150 -> баланс 0 руб. >= 0 -> WaitingState)
        demoPhone.setCallCost(150);
        printOperationReport(step++, "endCall() при тарифе 150 руб. (баланс 0 руб. >= 0 -> остаётся WaitingState)",
                             demoPhone.endCall(), demoPhone);

        // 10. При нулевом балансе (0 руб.) входящие звонки доступны и бесплатны
        demoPhone.answerCall();
        printOperationReport(step++, "answerCall() + endCall() при 0 руб. (бесплатный входящий -> WaitingState)",
                             demoPhone.endCall(), demoPhone);

        // 11. Исходящий звонок при 0 руб. уводит баланс в -50 руб. (< 0 -> BlockedState)
        demoPhone.setCallCost(50);
        demoPhone.makeCall("+7-900-444-55-66");
        demoPhone.answerCall();
        printOperationReport(step++, "endCall() при 0 руб. и тарифе 50 руб. (баланс -50 < 0 -> BlockedState)",
                             demoPhone.endCall(), demoPhone);

        // 12. В состоянии BlockedState (-50 руб.) исходящие вызовы запрещены
        printOperationReport(step++, "makeCall(\"+7-900-111-22-33\") в состоянии BlockedState (-50 руб.)",
                             demoPhone.makeCall("+7-900-111-22-33"), demoPhone);

        // 13. В состоянии BlockedState (-50 руб.) входящий звонок ПРИНИМАЕТСЯ (BlockedState -> TalkingState)
        printOperationReport(step++, "answerCall() в состоянии BlockedState (-50 руб.) — приём входящего вызова",
                             demoPhone.answerCall(), demoPhone);

        // 14. Завершение входящего разговора без списания (баланс -50 руб. < 0 -> обратно в BlockedState)
        printOperationReport(step++, "endCall() — завершение входящего при -50 руб. (0 руб. -> BlockedState)",
                             demoPhone.endCall(), demoPhone);

        // 15. Пополнение баланса до >= 0 (+200 -> 150 руб. -> разблокировка в WaitingState)
        printOperationReport(step++, "topUpBalance(200) — выход из отрицательного баланса (150 руб. -> WaitingState)",
                             demoPhone.topUpBalance(200), demoPhone);

        // 16. Контрольный исходящий звонок после разблокировки (-50 -> 100 руб.)
        demoPhone.makeCall("+7-900-777-00-00");
        demoPhone.answerCall();
        printOperationReport(step++, "Контрольный исходящий звонок + разговор + endCall() после разблокировки",
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
            std::cout << " 5. Изменить параметр вероятности поступления звонка (setCallProbability)\n";
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
                    std::cout << "Введите вероятность поступления звонка [0.0 .. 1.0]: ";
                    double p = 1.0;
                    std::cin >> p;
                    activePhone.setCallProbability(p);
                    std::cout << "Установлено значение атрибута: " << activePhone.getCallProbability() << "\n";
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
                    std::cout << "Введите вероятность поступления звонка [0.0 .. 1.0]: ";
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
