#set text(lang: "ru")

#align(center)[
    = Лабораторная работа №4
    *Разработка программных компонент на основе шаблонов проектирования* \
    *Вариант 5/1: Паттерн «Состояние» — система «Банкомат»*
]
#align(right)[
    Выполнил Храмов Александр, 451 группа, фКНиИТ
]

#outline()

#pagebreak()

= Постановка задачи (Вариант 5/1)

Целью работы является освоение методов объектно-ориентированного проектирования программных компонент на основе шаблонов проектирования (GoF).

В соответствии с вариантом *5/1* (задание №5 «Паттерн „Состояние“», подпункт 1 «Банкомат») необходимо:
1. Разработать диаграмму конечных автоматов (диаграмму состояний) для системы «Банкомат» и описать варианты реакции экземпляра класса на операции, вызываемые в указанных состояниях.
2. Разработать на основе шаблона проектирования *«Состояние» (State)* UML-диаграмму классов реализуемой программы и указать соответствие классов на этой диаграмме классам шаблона.
3. Разработать на объектно-ориентированном языке программирования (C++) библиотеку классов, включающую класс «Конечный автомат» (`ATM`), интерфейс «Состояние» (`ATMState`) и классы «Конкретные состояния» (`WaitingState`, `AuthenticationState`, `PerformingOperationsState`, `BlockedState`).
4. Разработать консольное приложение для управления состояниями экземпляров класса «Конечный автомат» и привести результаты тестирования программы.

== Спецификация класса, атрибутов, операций и состояний по ТЗ

#table(
    columns: (1.2fr, 2fr, 1.8fr),
    align: (left, left, left),
    [*Класс*], [*Атрибуты и операции*], [*Состояния автомата*],
    [*Банкомат* (`ATM`)],
    [
        *Атрибуты:*
        - `id` --- уникальный идентификатор (ID) банкомата;
        - `totalCash` --- общая сумма денег в банкомате;
        - `connectionLossProbability` --- вероятность отсутствия связи с банком ($p in [0.0, 1.0]$).

        *Операции:*
        - `enterPin(pin)` --- ввести PIN-код;
        - `withdrawCash(amount)` --- снять заданную сумму;
        - `finishSession()` --- завершить работу;
        - `loadCash(amount)` --- загрузить деньги в банкомат.
    ],
    [
        1. *Ожидание* (`WaitingState`)
        2. *Аутентификация пользователя* (`AuthenticationState`)
        3. *Выполнение операций* (`PerformingOperationsState`)
        4. *Заблокирован (денег нет)* (`BlockedState`)
    ]
)

#pagebreak()

= Шаблон проектирования «Состояние» (State) и соотношение с моделью программы

== Сущность шаблона «Состояние» (GoF)

Поведенческий шаблон проектирования *«Состояние» (State)* позволяет объекту изменять своё поведение в зависимости от внутреннего состояния. Внешне это выглядит так, словно объект меняет свой класс. Вместо громоздких условных конструкций (`switch` / `if-else`) внутри класса контекста, проверяющих текущий режим работы в каждом методе, паттерн выносит поведение каждого состояния в отдельный полиморфный класс, реализующий общий интерфейс `State`.

== Соответствие классов программы элементам шаблона GoF

В таблице ниже приведено прямое соответствие между каноническими участниками шаблона проектирования *State* и классами разработанной программной системы на языке C++:

#table(
    columns: (1.2fr, 1.4fr, 2.4fr),
    align: (left, left, left),
    [*Элемент шаблона GoF*], [*Класс / метод в программе*], [*Роль и ответственность в системе «Банкомат»*],
    [`Context` \ (Контекст / Конечный автомат)],
    [Класс `atm::ATM`],
    [Хранит бизнес-атрибуты банкомата (`id_`, `totalCash_`, `connectionLossProbability_`) и умный указатель `currentState_` на активный объект состояния. Делегирует вызовы `enterPin`, `withdrawCash`, `finishSession`, `loadCash` объекту `currentState_` и предоставляет метод `setState()` для смены состояния.],
    [`State` \ (Интерфейс состояния)],
    [Абстрактный класс `atm::ATMState`],
    [Определяет чистые виртуальные методы для всех 4 операций ТЗ (`enterPin`, `withdrawCash`, `finishSession`, `loadCash`), каждый из которых принимает ссылку на контекст `ATM&`.],
    [`ConcreteStateA` \ (Конкретное состояние 1)],
    [Класс `atm::WaitingState`],
    [Реализует поведение банкомата в состоянии *«Ожидание»*: принимает ввод PIN-кода (переводя автомат в режим аутентификации) и разрешает плановую загрузку денег; блокирует выдачу средств без авторизации.],
    [`ConcreteStateB` \ (Конкретное состояние 2)],
    [Класс `atm::AuthenticationState`],
    [Реализует поведение в состоянии *«Аутентификация пользователя»*: обрабатывает повторный ввод PIN-кода после ошибки или временного сбоя связи с банком, позволяет отменить сеанс (`finishSession` $->$ `WaitingState`) и запрещает снятие наличных до успешной проверки.],
    [`ConcreteStateC` \ (Конкретное состояние 3)],
    [Класс `atm::PerformingOperationsState`],
    [Реализует поведение в состоянии *«Выполнение операций»*: проверяет связь с банком и достаточность средств при выдаче наличных; при обнулении остатка (`totalCash == 0`) автоматически переводит автомат в `BlockedState`; по `finishSession()` возвращает в `WaitingState`.],
    [`ConcreteStateD` \ (Конкретное состояние 4)],
    [Класс `atm::BlockedState`],
    [Реализует поведение в состоянии *«Заблокирован (денег нет)»*: отклоняет любые клиентские операции (`enterPin`, `withdrawCash`, `finishSession`) до вызова операции инкассации `loadCash(amount > 0)`, разблокирующей банкомат с переходом в `WaitingState`.],
    [`Client` \ (Клиент)],
    [Приложение `ATMControllerApp` (`main.cpp`)],
    [Создаёт экземпляры конечного автомата `ATM`, управляет ими через публичный интерфейс контекста и отображает результаты переходов.]
)

#pagebreak()

= Диаграмма конечных автоматов (State Machine Diagram)

== Матрица реакций экземпляра класса `ATM` на операции

Поведение конечного автомата полностью определяется текущим состоянием и вызываемой операцией:

#table(
    columns: (1.1fr, 1.3fr, 1.3fr, 1.1fr, 1.2fr),
    align: (left, left, left, left, left),
    [*Состояние*], [*`enterPin(pin)`*], [*`withdrawCash(sum)`*], [*`finishSession()`*], [*`loadCash(sum)`*],
    [*`WaitingState`* \ (Ожидание)],
    [Переход в `AuthenticationState`. Если связь с банком есть и PIN верен $->$ переход в `PerformingOperationsState`; при сбое связи или неверном PIN остаётся в `AuthenticationState`.],
    [Отказ: «Сначала введите PIN-код». Состояние не меняется.],
    [Отказ: «Активный сеанс отсутствует». Состояние не меняется.],
    [При `sum > 0`: `totalCash += sum`. Остаётся в `WaitingState`.],
    [*`AuthenticationState`* \ (Аутентификация пользователя)],
    [Повторная проверка связи и PIN: при успехе $->$ переход в `PerformingOperationsState`; при сбое связи или неверном PIN остаётся в `AuthenticationState`.],
    [Отказ: «Аутентификация не завершена». Состояние не меняется.],
    [Прерывание входа. Переход в `WaitingState`.],
    [Отказ: «Загрузка запрещена во время сеанса клиента».],
    [*`PerformingOperationsState`* \ (Выполнение операций)],
    [Предупреждение: «Пользователь уже аутентифицирован». Состояние не меняется.],
    [Проверка связи с банком ($1 - p$). При наличии связи и `sum <= totalCash`: выдача (`totalCash -= sum`). Если `totalCash == 0` $->$ переход в `BlockedState`, иначе остаётся в `PerformingOperationsState`.],
    [Завершение обслуживания клиента. Переход в `WaitingState`.],
    [Отказ: «Сначала завершите сеанс клиента».],
    [*`BlockedState`* \ (Заблокирован: денег нет)],
    [Отказ: «Банкомат заблокирован (нет наличных)».],
    [Отказ: «Банкомат заблокирован (в кассетах 0 руб.)».],
    [Отказ: «Ожидается инкассация».],
    [При `sum > 0`: `totalCash += sum`, разблокировка $->$ переход в `WaitingState`.]
)

== Граф переходов конечного автомата «Банкомат»

При создании экземпляра `ATM` проверяется начальный запас наличных: при `totalCash > 0` автомат переходит в начальное состояние `WaitingState`, а при `totalCash == 0` --- сразу в состояние `BlockedState`.

#align(center)[
  #figure(
    image("state_diagram.drawio.svg", width: 100%),
    caption: [Диаграмма конечных автоматов (State Machine Diagram) системы «Банкомат»]
  )
]

#pagebreak()

= Диаграмма классов реализуемой программы (Class Diagram)

Архитектура программного комплекса разделена на два уровня:
1. *Библиотека классов шаблона «Состояние» (`src/atm_state_lib.hpp`, `src/atm_state_lib.cpp`)*:
   - класс контекста `ATM`, инкапсулирующий состояние и бизнес-правила;
   - абстрактный базовый класс `ATMState`;
   - четыре класса конкретных состояний (`WaitingState`, `AuthenticationState`, `PerformingOperationsState`, `BlockedState`);
   - структура `OperationResult`, возвращающая подробный отчёт о каждом вызове операции (успешность, сообщение, предыдущее и новое состояние).
2. *Управляющее консольное приложение (`src/main.cpp`)* и *модульные тесты (`src/test_atm.cpp`)*.

#align(center)[
  #figure(
    image("class_diagram.drawio.svg", width: 100%),
    caption: [Диаграмма классов (Class Diagram) библиотеки и приложения на основе шаблона «Состояние»]
  )
]

#pagebreak()

= Программная реализация на языке C++

Ниже приведены ключевые фрагменты исходного кода библиотеки классов (`src/atm_state_lib.hpp` и `src/atm_state_lib.cpp`), демонстрирующие реализацию шаблона проектирования «Состояние».

== Интерфейс состояния и класс конечного автомата (`src/atm_state_lib.hpp`)

```cpp
namespace atm {

class ATM;

/// Структура результата выполнения операции банкомата
struct OperationResult {
    bool success;               ///< Признак успешного выполнения операции
    std::string message;        ///< Описание реакции автомата на операцию
    std::string previousState;  ///< Состояние до вызова операции
    std::string currentState;   ///< Состояние после вызова операции
};

/// Абстрактный класс (интерфейс) «Состояние» (State в шаблоне GoF)
class ATMState {
public:
    virtual ~ATMState() = default;
    virtual std::string getName() const = 0;
    virtual OperationResult enterPin(ATM& atm, const std::string& pin) = 0;
    virtual OperationResult withdrawCash(ATM& atm, int amount) = 0;
    virtual OperationResult finishSession(ATM& atm) = 0;
    virtual OperationResult loadCash(ATM& atm, int amount) = 0;
};

/// Класс «Банкомат» / «Конечный автомат» (Context в шаблоне GoF)
class ATM {
private:
    std::string id_;                          ///< ID банкомата
    int totalCash_;                           ///< Общая сумма денег в банкомате
    double connectionLossProbability_;        ///< Вероятность отсутствия связи с банком
    std::string validPin_;                    ///< Эталонный PIN-код
    std::shared_ptr<ATMState> currentState_;  ///< Текущее состояние автомата
    std::mt19937 rng_;                        ///< Генератор случайных чисел
    std::vector<std::string> historyLog_;     ///< Журнал переходов

public:
    ATM(std::string id, int initialCash, double connectionLossProb,
        std::string validPin = "1234", unsigned int randomSeed = 42);

    // Делегирование операций текущему состоянию
    OperationResult enterPin(const std::string& pin);
    OperationResult withdrawCash(int amount);
    OperationResult finishSession();
    OperationResult loadCash(int amount);

    void setState(std::shared_ptr<ATMState> newState);
    std::shared_ptr<ATMState> getState() const;
    std::string getStateName() const;
    bool checkBankConnection();
    bool verifyPin(const std::string& pin) const;
    void addCash(int amount);
    void deductCash(int amount);
};

} // namespace atm
```

== Реализация переходов в конкретных состояниях (`src/atm_state_lib.cpp`)

```cpp
// Ввод PIN-кода в состоянии «Ожидание» (WaitingState)
OperationResult WaitingState::enterPin(ATM& atm, const std::string& pin) {
    const std::string prevState = getName();
    // Переход в состояние «Аутентификация пользователя»
    atm.setState(std::make_shared<AuthenticationState>());

    if (!atm.checkBankConnection()) {
        return {false, "Сбой связи с банком при проверке PIN-кода.", prevState, atm.getStateName()};
    }
    if (!atm.verifyPin(pin)) {
        return {false, "Неверный PIN-код («" + pin + "»).", prevState, atm.getStateName()};
    }

    // При наличии связи и верном PIN — переход в «Выполнение операций»
    atm.setState(std::make_shared<PerformingOperationsState>());
    return {true, "Аутентификация пройдена успешно.", prevState, atm.getStateName()};
}

// Выдача наличных в состоянии «Выполнение операций» (PerformingOperationsState)
OperationResult PerformingOperationsState::withdrawCash(ATM& atm, int amount) {
    const std::string prevState = getName();
    if (amount <= 0) {
        return {false, "Отказ: сумма должна быть больше нуля.", prevState, atm.getStateName()};
    }
    if (!atm.checkBankConnection()) {
        return {false, "Ошибка транзакции: отсутствует связь с банком.", prevState, atm.getStateName()};
    }
    if (amount > atm.getTotalCash()) {
        return {false, "Отказ: недостаточно средств в банкомате.", prevState, atm.getStateName()};
    }

    atm.deductCash(amount);
    if (atm.getTotalCash() == 0) {
        atm.setState(std::make_shared<BlockedState>());
        return {true, "Выдано " + std::to_string(amount) + " руб. Остаток 0 руб. — банкомат заблокирован.",
                prevState, atm.getStateName()};
    }
    return {true, "Выдано " + std::to_string(amount) + " руб.", prevState, atm.getStateName()};
}
```

#pagebreak()

= Результаты тестирования программы

Тестирование программного комплекса проводилось в двух режимах: автоматическое модульное тестирование (`build/test_atm`) и сквозной демонстрационный сценарий (`build/atm_app --demo`).

== Протокол модульного тестирования (`make test`)

```text
$ make -C lab-4 test
g++ -std=c++17 -Wall -Wextra -pedantic -O2 -c src/atm_state_lib.cpp -o build/atm_state_lib.o
ar rcs build/libatm_state.a build/atm_state_lib.o
g++ -std=c++17 -Wall -Wextra -pedantic -O2 src/test_atm.cpp build/libatm_state.a -o build/test_atm
./build/test_atm
================================================================================
   ЗАПУСК МОДУЛЬНЫХ ТЕСТОВ БИБЛИОТЕКИ КЛАССОВ «БАНКОМАТ» (ПАТТЕРН «СОСТОЯНИЕ»)
================================================================================
[PASS] testInitialStates — проверка начальных состояний автомата
[PASS] testWaitingStateReactions — реакции в состоянии WaitingState
[PASS] testAuthenticationStateTransitions — переходы состояния AuthenticationState
[PASS] testPerformingOperationsAndBlocking — выполнение операций и блокировка при 0 руб.
[PASS] testBlockedStateAndUnblocking — блокировка и разблокировка инкассацией
================================================================================
 ИТОГ: Все 5 тестовых наборов успешно пройдены (100% ОК).
================================================================================
```

== Протокол демонстрационного сценария (`./build/atm_app --demo`)

В ходе демонстрационного сценария последовательно проверяются все 15 переходов и реакций конечного автомата `ATM-SGU-01`:

#table(
    columns: (0.4fr, 1.5fr, 1.6fr, 1.5fr),
    align: (center, left, left, left),
    [*Шаг*], [*Вызываемая операция*], [*Переход состояний*], [*Результат и остаток средств*],
    [1], [`withdrawCash(2000)`], [`WaitingState` $->$ `WaitingState`], [ОТКЛОНЕНО: сначала введите PIN (10 000 руб.)],
    [2], [`loadCash(5000)`], [`WaitingState` $->$ `WaitingState`], [УСПЕШНО: инкассация +5000 руб. (15 000 руб.)],
    [3], [`enterPin("0000")` (неверный)], [`WaitingState` $->$ `AuthenticationState`], [ОТКЛОНЕНО: неверный PIN, ожидание повтора],
    [4], [`withdrawCash(1000)`], [`AuthenticationState` $->$ `AuthenticationState`], [ОТКЛОНЕНО: вход ещё не завершён],
    [5], [`finishSession()`], [`AuthenticationState` $->$ `WaitingState`], [УСПЕШНО: отмена входа, возврат в ожидание],
    [6], [`enterPin("1234")` при $p = 1.0$], [`WaitingState` $->$ `AuthenticationState`], [ОТКЛОНЕНО: сбой связи с банком],
    [7], [`enterPin("1234")` при $p = 0.0$], [`AuthenticationState` $->$ `PerformingOperationsState`], [УСПЕШНО: повторный ввод PIN прошёл],
    [8], [`withdrawCash(6000)`], [`PerformingOperationsState` $->$ `PerformingOperationsState`], [УСПЕШНО: выдано 6000 руб. (остаток 9000 руб.)],
    [9], [`withdrawCash(20000)`], [`PerformingOperationsState` $->$ `PerformingOperationsState`], [ОТКЛОНЕНО: запрошено больше остатка],
    [10], [`withdrawCash(9000)`], [`PerformingOperationsState` $->$ `BlockedState`], [УСПЕШНО: выдано 9000 руб., остаток 0 руб. $->$ блокировка],
    [11], [`enterPin("1234")`], [`BlockedState` $->$ `BlockedState`], [ОТКЛОНЕНО: банкомат заблокирован (нет денег)],
    [12], [`withdrawCash(500)`], [`BlockedState` $->$ `BlockedState`], [ОТКЛОНЕНО: в кассетах 0 руб.],
    [13], [`loadCash(25000)`], [`BlockedState` $->$ `WaitingState`], [УСПЕШНО: загружено 25 000 руб., разблокировка],
    [14], [`enterPin("1234")`], [`WaitingState` $->$ `PerformingOperationsState`], [УСПЕШНО: вход с первой попытки],
    [15], [`finishSession()`], [`PerformingOperationsState` $->$ `WaitingState`], [УСПЕШНО: завершение сеанса клиента]
)

= Заключение

В рамках лабораторной работы №4 по варианту 5/1 спроектирована и реализована на языке C++ программная система «Банкомат» на основе поведенческого шаблона проектирования «Состояние» (State):
- Разработана диаграмма конечных автоматов и специфицирована матрица реакций экземпляра класса `ATM` на операции `enterPin`, `withdrawCash`, `finishSession` и `loadCash` в четырёх состояниях (`WaitingState`, `AuthenticationState`, `PerformingOperationsState`, `BlockedState`).
- Построена UML-диаграмма классов и установлено взаимно-однозначное соответствие между классами программы и элементами шаблона GoF (`Context`, `State`, `ConcreteState`).
- Реализована библиотека классов `libatm_state.a`, консольное приложение `atm_app` и набор модульных тестов `test_atm`, подтвердивших корректность всех переходов и граничных условий (неверный PIN-код, потеря связи с банком, исчерпание средств и последующая инкассация).

