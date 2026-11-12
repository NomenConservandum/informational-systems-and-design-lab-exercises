/**
 * @file main.cpp
 * @brief Консольное приложение для управления состояниями экземпляров класса
 *        Конечный автомат («Банкомат») по шаблону проектирования «Состояние».
 *
 * Поддерживает два режима работы:
 * 1. Автоматический демонстрационный прогон всех состояний и операций: `./atm_app --demo`
 * 2. Интерактивный консольный режим управления экземплярами банкоматов: `./atm_app`
 */

#include "atm_state_lib.hpp"

#include <iostream>
#include <limits>
#include <string>
#include <vector>

using namespace atm;

/**
 * @brief Вспомогательная функция вывода результата операции на экран консоли.
 */
void printStep(int stepNumber, const std::string& actionDesc, const OperationResult& res, const ATM& atm) {
    std::cout << "Шаг " << stepNumber << ". Действие: " << actionDesc << "\n"
              << "   Переход:   " << res.previousState << " -> " << res.currentState << "\n"
              << "   Результат: " << (res.success ? "[УСПЕШНО] " : "[ОТКЛОНЕНО] ") << res.message << "\n"
              << "   Статус:    " << atm.getStatusSummary() << "\n"
              << "--------------------------------------------------------------------------------\n";
}

/**
 * @brief Демонстрационный сценарий, проверяющий все 4 состояния и все реакции
 *        экземпляра класса ATM на операции.
 */
void runDemonstrationScenario() {
    std::cout << "================================================================================\n"
              << "   ДЕМОНСТРАЦИЯ РАБОТЫ КОНЕЧНОГО АВТОМАТА «БАНКОМАТ» (ПАТТЕРН «СОСТОЯНИЕ»)\n"
              << "================================================================================\n\n";

    ATM atm("ATM-SGU-01", 10000, 0.0, "1234", 100);
    std::cout << "Создан экземпляр: " << atm.getStatusSummary() << "\n"
              << "--------------------------------------------------------------------------------\n";

    int step = 1;

    // 1. Попытка снять деньги в состоянии «Ожидание» (без ввода PIN-кода)
    printStep(step++, "withdrawCash(2000) в состоянии WaitingState", atm.withdrawCash(2000), atm);

    // 2. Пополнение банкомата в состоянии «Ожидание»
    printStep(step++, "loadCash(5000) в состоянии WaitingState", atm.loadCash(5000), atm);

    // 3. Ввод неверного PIN-кода -> переход в «Аутентификация пользователя»
    printStep(step++, "enterPin(\"0000\") — неверный PIN-код", atm.enterPin("0000"), atm);

    // 4. Попытка снять деньги в состоянии «Аутентификация пользователя»
    printStep(step++, "withdrawCash(1000) до завершения аутентификации", atm.withdrawCash(1000), atm);

    // 5. Отмена аутентификации -> возврат в «Ожидание»
    printStep(step++, "finishSession() — отмена входа из AuthenticationState", atm.finishSession(), atm);

    // 6. Моделирование отсутствия связи с банком при вводе PIN-кода (p = 1.0)
    atm.setConnectionLossProbability(1.0);
    printStep(step++, "enterPin(\"1234\") при обрыве связи с банком (p = 1.0)", atm.enterPin("1234"), atm);

    // 7. Восстановление связи с банком (p = 0.0) и повторный ввод верного PIN-кода
    atm.setConnectionLossProbability(0.0);
    printStep(step++, "enterPin(\"1234\") в состоянии AuthenticationState при восстановленной связи",
              atm.enterPin("1234"), atm);

    // 8. Частичное снятие наличных в состоянии «Выполнение операций»
    printStep(step++, "withdrawCash(6000) в состоянии PerformingOperationsState", atm.withdrawCash(6000), atm);

    // 9. Попытка снять сумму больше остатка в банкомате
    printStep(step++, "withdrawCash(20000) — сумма превышает остаток в банкомате", atm.withdrawCash(20000), atm);

    // 10. Снятие всего оставшегося баланса (9000 руб.) -> переход в «Заблокирован (денег нет)»
    printStep(step++, "withdrawCash(9000) — полное исчерпание средств и блокировка банкомата",
              atm.withdrawCash(9000), atm);

    // 11. Попытка ввести PIN-код в заблокированном банкомате
    printStep(step++, "enterPin(\"1234\") в состоянии BlockedState", atm.enterPin("1234"), atm);

    // 12. Попытка снять деньги в заблокированном банкомате
    printStep(step++, "withdrawCash(500) в состоянии BlockedState", atm.withdrawCash(500), atm);

    // 13. Загрузка денег службой инкассации -> разблокировка и переход в «Ожидание»
    printStep(step++, "loadCash(25000) в состоянии BlockedState (инкассация и разблокировка)",
              atm.loadCash(25000), atm);

    // 14. Успешный вход с первой попытки (WaitingState -> AuthenticationState -> PerformingOperationsState)
    printStep(step++, "enterPin(\"1234\") в состоянии WaitingState", atm.enterPin("1234"), atm);

    // 15. Штатное завершение работы клиента
    printStep(step++, "finishSession() в состоянии PerformingOperationsState", atm.finishSession(), atm);
}

/**
 * @brief Интерактивное консольное меню для управления несколькими экземплярами ATM.
 */
void runInteractiveMenu() {
    std::vector<ATM> atms;
    atms.emplace_back("ATM-01", 15000, 0.15, "1234", 42);
    atms.emplace_back("ATM-02", 0, 0.05, "4321", 77);

    std::size_t activeIdx = 0;

    while (true) {
        ATM& currentAtm = atms[activeIdx];
        std::cout << "\n================================================================================\n"
                  << " УПРАВЛЕНИЕ КОНЕЧНЫМИ АВТОМАТАМИ «БАНКОМАТ» (Активный: #" << (activeIdx + 1) << ")\n"
                  << " " << currentAtm.getStatusSummary() << "\n"
                  << "================================================================================\n"
                  << " 1. Ввести PIN-код (enterPin)\n"
                  << " 2. Снять заданную сумму (withdrawCash)\n"
                  << " 3. Завершить работу (finishSession)\n"
                  << " 4. Загрузить деньги в банкомат (loadCash)\n"
                  << " 5. Изменить вероятность отсутствия связи с банком\n"
                  << " 6. Показать журнал переходов текущего банкомата\n"
                  << " 7. Переключить активный банкомат / создать новый экземпляр\n"
                  << " 8. Запустить полный демонстрационный сценарий\n"
                  << " 0. Выход\n"
                  << "Выберите действие: ";

        int choice = -1;
        if (!(std::cin >> choice)) {
            if (std::cin.eof()) {
                std::cout << "\nЗавершение работы приложения.\n";
                break;
            }
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            std::cout << "Некорректный ввод. Введите номер пункта меню.\n";
            continue;
        }

        if (choice == 0) {
            std::cout << "Завершение работы приложения.\n";
            break;
        }

        switch (choice) {
            case 1: {
                std::string pin;
                std::cout << "Введите PIN-код (эталонный для этого банкомата: "
                          << currentAtm.getValidPin() << "): ";
                std::cin >> pin;
                OperationResult res = currentAtm.enterPin(pin);
                std::cout << "-> [" << res.previousState << " -> " << res.currentState << "] "
                          << res.message << "\n";
                break;
            }
            case 2: {
                int amount = 0;
                std::cout << "Введите сумму для снятия (руб.): ";
                std::cin >> amount;
                OperationResult res = currentAtm.withdrawCash(amount);
                std::cout << "-> [" << res.previousState << " -> " << res.currentState << "] "
                          << res.message << "\n";
                break;
            }
            case 3: {
                OperationResult res = currentAtm.finishSession();
                std::cout << "-> [" << res.previousState << " -> " << res.currentState << "] "
                          << res.message << "\n";
                break;
            }
            case 4: {
                int amount = 0;
                std::cout << "Введите сумму загрузки наличных (руб.): ";
                std::cin >> amount;
                OperationResult res = currentAtm.loadCash(amount);
                std::cout << "-> [" << res.previousState << " -> " << res.currentState << "] "
                          << res.message << "\n";
                break;
            }
            case 5: {
                double prob = 0.0;
                std::cout << "Введите вероятность отсутствия связи [0.0 .. 1.0]: ";
                std::cin >> prob;
                currentAtm.setConnectionLossProbability(prob);
                std::cout << "-> Установлена вероятность обрыва связи: "
                          << currentAtm.getConnectionLossProbability() << "\n";
                break;
            }
            case 6: {
                std::cout << "\n--- Журнал переходов банкомата [" << currentAtm.getId() << "] ---\n";
                for (const auto& entry : currentAtm.getHistoryLog()) {
                    std::cout << " • " << entry << "\n";
                }
                break;
            }
            case 7: {
                std::cout << "\nСписок экземпляров класса ATM:\n";
                for (std::size_t i = 0; i < atms.size(); ++i) {
                    std::cout << " [" << (i + 1) << "] " << atms[i].getStatusSummary() << "\n";
                }
                std::cout << "Введите номер существующего банкомата или 0 для создания нового: ";
                int sub = 0;
                std::cin >> sub;
                if (sub == 0) {
                    std::string id, pin;
                    int cash = 0;
                    double prob = 0.0;
                    std::cout << "ID нового банкомата: ";
                    std::cin >> id;
                    std::cout << "Начальная сумма наличных (руб.): ";
                    std::cin >> cash;
                    std::cout << "Вероятность обрыва связи [0.0..1.0]: ";
                    std::cin >> prob;
                    std::cout << "PIN-код по умолчанию: ";
                    std::cin >> pin;
                    atms.emplace_back(id, cash, prob, pin);
                    activeIdx = atms.size() - 1;
                    std::cout << "-> Создан и выбран новый экземпляр [" << id << "].\n";
                } else if (sub >= 1 && static_cast<std::size_t>(sub) <= atms.size()) {
                    activeIdx = static_cast<std::size_t>(sub - 1);
                    std::cout << "-> Выбран банкомат [" << atms[activeIdx].getId() << "].\n";
                }
                break;
            }
            case 8: {
                runDemonstrationScenario();
                break;
            }
            default:
                std::cout << "Неизвестная команда.\n";
                break;
        }
    }
}

int main(int argc, char* argv[]) {
    if (argc > 1 && std::string(argv[1]) == "--demo") {
        runDemonstrationScenario();
        return 0;
    }

    runInteractiveMenu();
    return 0;
}

