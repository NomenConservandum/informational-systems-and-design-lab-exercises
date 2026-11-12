#set text(lang: "ru")

#align(center)[
    = Лабораторная работа №4
    *Разработка программных компонент на основе шаблонов проектирования* \
    *Вариант 5/1: Паттерн «Состояние» — система «Телефон»*
]
#align(right)[
    Выполнил Храмов Александр, 451 группа, фКНиИТ
]

#outline()

#pagebreak()

= Постановка задачи (Вариант 5/1)

Целью работы является освоение методов объектно-ориентированного проектирования программных компонент на основе шаблонов проектирования (GoF).

В соответствии с вариантом *5/1* (задание №5 «Паттерн „Состояние“», подпункт 1 «Телефон») необходимо:
1. Разработать диаграмму конечных автоматов (диаграмму состояний) для системы «Телефон» и описать варианты реакции экземпляра класса на операции, вызываемые в указанных состояниях.
2. Разработать на основе шаблона проектирования *«Состояние» (State)* UML-диаграмму классов реализуемой программы и указать соответствие классов на этой диаграмме классам шаблона.
3. Разработать на объектно-ориентированном языке программирования (C++) библиотеку классов, включающую класс «Конечный автомат» (`Phone`), интерфейс «Состояние» (`PhoneState`) и классы «Конкретные состояния» (`WaitingState`, `CallingState`, `TalkingState`, `BlockedState`).
4. Разработать консольное приложение для управления состояниями экземпляров класса «Конечный автомат» и привести результаты тестирования программы.

== Спецификация класса, атрибутов, операций и состояний по ТЗ

#table(
    columns: (1.2fr, 2fr, 1.8fr),
    align: (left, left, left),
    [*Класс*], [*Атрибуты и операции*], [*Состояния автомата*],
    [*Телефон* (`Phone`)],
    [
        *Атрибуты:*
        - `phoneNumber` --- абонентский номер телефона;
        - `balance` --- текущий баланс средств на счёте;
        - `callProbability` --- вероятность дозвона / поступления вызова ($p in [0.0, 1.0]$).

        *Операции:*
        - `makeCall(target)` --- позвонить;
        - `answerCall()` --- ответить на звонок;
        - `endCall()` --- завершить разговор (или сбросить вызов);
        - `topUpBalance(amount)` --- пополнить баланс.
    ],
    [
        1. *Ожидание* (`WaitingState`)
        2. *Звонок* (`CallingState`)
        3. *Разговор* (`TalkingState`)
        4. *Заблокирован (нет средств)* (`BlockedState`)
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
    [*Элемент шаблона GoF*], [*Класс / метод в программе*], [*Роль и ответственность в системе «Телефон»*],
    [`Context` \ (Контекст / Конечный автомат)],
    [Класс `phone::Phone`],
    [Хранит бизнес-атрибуты телефона (`phoneNumber_`, `balance_`, `callProbability_`, `callCost_`) и умный указатель `currentState_` на активный объект состояния. Делегирует вызовы `makeCall`, `answerCall`, `endCall`, `topUpBalance` объекту `currentState_` и предоставляет метод `setState()` для смены состояния.],
    [`State` \ (Интерфейс состояния)],
    [Абстрактный класс `phone::PhoneState`],
    [Определяет чистые виртуальные методы для всех 4 операций ТЗ (`makeCall`, `answerCall`, `endCall`, `topUpBalance`), каждый из которых принимает ссылку на контекст `Phone&`.],
    [`ConcreteStateA` \ (Конкретное состояние 1)],
    [Класс `phone::WaitingState`],
    [Реализует поведение телефона в состоянии *«Ожидание»*: при наличии средств (`balance > 0`) и срабатывании вероятности дозвона переводит аппарат в состояние `CallingState`; разрешает пополнение баланса; отклоняет `answerCall` и `endCall`.],
    [`ConcreteStateB` \ (Конкретное состояние 2)],
    [Класс `phone::CallingState`],
    [Реализует поведение в состоянии *«Звонок»*: по вызову `answerCall()` устанавливает соединение и переводит автомат в `TalkingState`; по `endCall()` сбрасывает вызов без списания средств с возвратом в `WaitingState`; запрещает повторный набор номера.],
    [`ConcreteStateC` \ (Конкретное состояние 3)],
    [Класс `phone::TalkingState`],
    [Реализует поведение в состоянии *«Разговор»*: при завершении разговора (`endCall()`) списывает стоимость вызова (`balance -= callCost`); при `balance <= 0` автоматически переводит телефон в `BlockedState`, а при `balance > 0` возвращает в `WaitingState`.],
    [`ConcreteStateD` \ (Конкретное состояние 4)],
    [Класс `phone::BlockedState`],
    [Реализует поведение в состоянии *«Заблокирован (нет средств)»*: отклоняет операции связи (`makeCall`, `answerCall`, `endCall`) до вызова `topUpBalance(amount)`, который при достижении положительного баланса (`balance > 0`) разблокирует телефон с переходом в `WaitingState`.],
    [`Client` \ (Клиент)],
    [Приложение `PhoneControllerApp` (`main.cpp`)],
    [Создаёт экземпляры конечного автомата `Phone`, управляет ими через публичный интерфейс контекста и отображает результаты переходов.]
)

#pagebreak()

= Диаграмма конечных автоматов (State Machine Diagram)

== Матрица реакций экземпляра класса `Phone` на операции

Поведение конечного автомата полностью определяется текущим состоянием и вызываемой операцией:

#table(
    columns: (1.05fr, 1.2fr, 1.05fr, 1.25fr, 1.45fr),
    align: (left, left, left, left, left),
    [*Состояние*], [*`makeCall(target)`*], [*`answerCall()`*], [*`endCall()`*], [*`topUpBalance(amount)`*],
    [*`WaitingState`* \ (Ожидание)],
    [Проверка `balance > 0` и вероятности вызова ($p$). При успехе $->$ переход в `CallingState`; при сбое связи остаётся в `WaitingState`.],
    [Отказ: «Нет активного вызова». Состояние не меняется.],
    [Отказ: «Нет активного вызова или разговора». Состояние не меняется.],
    [При `amount > 0`: `balance += amount`. Остаётся в `WaitingState`.],
    [*`CallingState`* \ (Звонок)],
    [Отказ: «Телефон уже выполняет вызов». Состояние не меняется.],
    [Установление голосового соединения $->$ переход в `TalkingState`.],
    [Сброс/отмена вызова до ответа собеседника $->$ переход в `WaitingState`.],
    [При `amount > 0`: `balance += amount`. Остаётся в `CallingState`.],
    [*`TalkingState`* \ (Разговор)],
    [Отказ: «Линия занята, идёт разговор». Состояние не меняется.],
    [Предупреждение: «Соединение уже установлено». Состояние не меняется.],
    [Завершение разговора и списание тарифа (`balance -= callCost`). Если `balance > 0` $->$ `WaitingState`; если `balance <= 0` $->$ `BlockedState`.],
    [При `amount > 0`: `balance += amount`. Остаётся в `TalkingState`.],
    [*`BlockedState`* \ (Заблокирован: нет средств)],
    [Отказ: «Телефон заблокирован (нет средств на счёте)».],
    [Отказ: «Приём вызовов невозможен — телефон заблокирован».],
    [Отказ: «Телефон заблокирован, активных вызовов нет».],
    [При `amount > 0`: `balance += amount`. Если `balance > 0` $->$ разблокировка и переход в `WaitingState`, иначе остаётся в `BlockedState`.]
)

== Граф переходов конечного автомата «Телефон»

При создании экземпляра `Phone` проверяется начальный баланс счёта: при `balance > 0` автомат переходит в начальное состояние `WaitingState`, а при `balance <= 0` --- сразу в состояние `BlockedState`.

#align(center)[
  #figure(
    image("state_diagram.drawio.svg", width: 100%),
    caption: [Диаграмма конечных автоматов (State Machine Diagram) системы «Телефон»]
  )
]

#pagebreak()

= Диаграмма классов реализуемой программы (Class Diagram)

Архитектура программного комплекса разделена на два уровня:
1. *Библиотека классов шаблона «Состояние» (`src/phone_state_lib.hpp`, `src/phone_state_lib.cpp`)*:
   - класс контекста `Phone`, инкапсулирующий атрибуты телефона и делегирующий вызовы объекту состояния;
   - абстрактный базовый класс `PhoneState`;
   - четыре класса конкретных состояний (`WaitingState`, `CallingState`, `TalkingState`, `BlockedState`);
   - структура `OperationResult`, возвращающая подробный отчёт о каждом вызове операции (успешность, сообщение, предыдущее и новое состояние).
2. *Управляющее консольное приложение (`src/main.cpp`)* и *модульные тесты (`src/test_phone.cpp`)*.

#align(center)[
  #figure(
    image("class_diagram.drawio.svg", width: 100%),
    caption: [Диаграмма классов (Class Diagram) библиотеки и приложения на основе шаблона «Состояние»]
  )
]

#pagebreak()

= Программная реализация на языке C++

Ниже приведены ключевые фрагменты исходного кода библиотеки классов (`src/phone_state_lib.hpp` и `src/phone_state_lib.cpp`), демонстрирующие реализацию шаблона проектирования «Состояние».

== Интерфейс состояния и класс конечного автомата (`src/phone_state_lib.hpp`)

```cpp
namespace phone {

class Phone;

/// Структура результата выполнения операции телефона
struct OperationResult {
    bool success;               ///< Признак успешного выполнения операции
    std::string message;        ///< Описание реакции автомата на операцию
    std::string previousState;  ///< Состояние до вызова операции
    std::string currentState;   ///< Состояние после вызова операции
};

/// Абстрактный класс (интерфейс) «Состояние» (State в шаблоне GoF)
class PhoneState {
public:
    virtual ~PhoneState() = default;
    virtual std::string getName() const = 0;
    virtual OperationResult makeCall(Phone& phone, const std::string& targetNumber) = 0;
    virtual OperationResult answerCall(Phone& phone) = 0;
    virtual OperationResult endCall(Phone& phone) = 0;
    virtual OperationResult topUpBalance(Phone& phone, int amount) = 0;
};

/// Класс «Телефон» / «Конечный автомат» (Context в шаблоне GoF)
class Phone {
private:
    std::string phoneNumber_;                  ///< Номер телефона абонента
    int balance_;                              ///< Текущий баланс счёта (в руб.)
    double callProbability_;                   ///< Вероятность дозвона / вызова
    int callCost_;                             ///< Стоимость одного разговора
    std::string activePeerNumber_;             ///< Номер собеседника
    std::shared_ptr<PhoneState> currentState_; ///< Текущее состояние автомата
    std::mt19937 rng_;                         ///< Генератор случайных чисел
    std::vector<std::string> historyLog_;      ///< Журнал переходов

public:
    Phone(std::string phoneNumber, int initialBalance, double callProbability,
          int callCost = 50, unsigned int randomSeed = 42);

    // Делегирование операций текущему состоянию
    OperationResult makeCall(const std::string& targetNumber = "+7-900-000-00-00");
    OperationResult answerCall();
    OperationResult endCall();
    OperationResult topUpBalance(int amount);

    void setState(std::shared_ptr<PhoneState> newState);
    std::shared_ptr<PhoneState> getState() const;
    std::string getStateName() const;
    bool checkCallProbability();
    void addBalance(int amount);
    void deductCallCost();
};

} // namespace phone
```

== Реализация переходов в конкретных состояниях (`src/phone_state_lib.cpp`)

```cpp
// Инициирование вызова в состоянии «Ожидание» (WaitingState)
OperationResult WaitingState::makeCall(Phone& phone, const std::string& targetNumber) {
    const std::string prevState = getName();
    if (phone.getBalance() <= 0) {
        phone.setState(std::make_shared<BlockedState>());
        return {false, "Недостаточно средств на счёте. Телефон заблокирован.",
                prevState, phone.getStateName()};
    }
    if (!phone.checkCallProbability()) {
        return {false, "Вызов не состоялся (сбой сети/абонент недоступен).",
                prevState, phone.getStateName()};
    }
    phone.setActivePeerNumber(targetNumber);
    phone.setState(std::make_shared<CallingState>());
    return {true, "Инициирован звонок абоненту «" + targetNumber + "».",
            prevState, phone.getStateName()};
}

// Завершение разговора в состоянии «Разговор» (TalkingState)
OperationResult TalkingState::endCall(Phone& phone) {
    const std::string prevState = getName();
    const std::string peer = phone.getActivePeerNumber();
    phone.deductCallCost();
    phone.setActivePeerNumber("");

    if (phone.getBalance() <= 0) {
        phone.setState(std::make_shared<BlockedState>());
        return {true, "Разговор с «" + peer + "» завершён. Баланс исчерпан — блокировка!",
                prevState, phone.getStateName()};
    }
    phone.setState(std::make_shared<WaitingState>());
    return {true, "Разговор с «" + peer + "» завершён.", prevState, phone.getStateName()};
}
```

#pagebreak()

= Результаты тестирования программы

Тестирование программного комплекса проводилось в двух режимах: автоматическое модульное тестирование (`build/test_phone`) и сквозной демонстрационный сценарий (`build/phone_app --demo`).

== Протокол модульного тестирования (`make test`)

```text
$ make -C lab-4 test
g++ -std=c++17 -Wall -Wextra -pedantic -O2 -c src/phone_state_lib.cpp -o build/phone_state_lib.o
ar rcs build/libphone_state.a build/phone_state_lib.o
g++ -std=c++17 -Wall -Wextra -pedantic -O2 src/test_phone.cpp build/libphone_state.a -o build/test_phone
./build/test_phone
================================================================================
    ЗАПУСК МОДУЛЬНЫХ ТЕСТОВ БИБЛИОТЕКИ КЛАССОВ «ТЕЛЕФОН» (ПАТТЕРН «СОСТОЯНИЕ»)
================================================================================
[PASS] testInitialStates — проверка начальных состояний телефона
[PASS] testWaitingStateReactions — реакции в состоянии WaitingState
[PASS] testCallingStateTransitions — переходы состояния CallingState
[PASS] testTalkingStateAndBlocking — разговор, списание средств и блокировка
[PASS] testBlockedStateAndUnblocking — блокировка и разблокировка пополнением баланса
================================================================================
 ИТОГ: Все 5 тестовых наборов успешно пройдены (100% ОК).
================================================================================
```

== Протокол демонстрационного сценария (`./build/phone_app --demo`)

В ходе демонстрационного сценария последовательно проверяются все 15 переходов и реакций конечного автомата `+7-917-555-01-01`:

#table(
    columns: (0.4fr, 1.5fr, 1.6fr, 1.5fr),
    align: (center, left, left, left),
    [*Шаг*], [*Вызываемая операция*], [*Переход состояний*], [*Результат и баланс*],
    [1], [`answerCall()`], [`WaitingState` $->$ `WaitingState`], [ОТКЛОНЕНО: нет входящего вызова (100 руб.)],
    [2], [`endCall()`], [`WaitingState` $->$ `WaitingState`], [ОТКЛОНЕНО: нет активного разговора (100 руб.)],
    [3], [`topUpBalance(50)`], [`WaitingState` $->$ `WaitingState`], [УСПЕШНО: пополнение +50 руб. (150 руб.)],
    [4], [`makeCall(...)` при $p = 0.0$], [`WaitingState` $->$ `WaitingState`], [ОТКЛОНЕНО: сбой вызова (150 руб.)],
    [5], [`makeCall(...)` при $p = 1.0$], [`WaitingState` $->$ `CallingState`], [УСПЕШНО: идут гудки вызова (150 руб.)],
    [6], [`makeCall(...)` повторно], [`CallingState` $->$ `CallingState`], [ОТКЛОНЕНО: линия уже занята вызовом],
    [7], [`endCall()` до ответа], [`CallingState` $->$ `WaitingState`], [УСПЕШНО: сброс вызова без списания (150 руб.)],
    [8], [`makeCall(...)` повторно], [`WaitingState` $->$ `CallingState`], [УСПЕШНО: повторный вызов абонента],
    [9], [`answerCall()`], [`CallingState` $->$ `TalkingState`], [УСПЕШНО: соединение установлено (150 руб.)],
    [10], [`endCall()` (тариф 50 руб.)], [`TalkingState` $->$ `WaitingState`], [УСПЕШНО: списано 50 руб., остаток 100 руб.],
    [11], [`endCall()` (тариф 100 руб.)], [`TalkingState` $->$ `BlockedState`], [УСПЕШНО: остаток 0 руб. $->$ блокировка],
    [12], [`makeCall(...)`], [`BlockedState` $->$ `BlockedState`], [ОТКЛОНЕНО: телефон заблокирован (0 руб.)],
    [13], [`answerCall()`], [`BlockedState` $->$ `BlockedState`], [ОТКЛОНЕНО: приём вызовов заблокирован],
    [14], [`topUpBalance(200)`], [`BlockedState` $->$ `WaitingState`], [УСПЕШНО: баланс 200 руб., разблокировка],
    [15], [Звонок + разговор + `endCall()`], [`TalkingState` $->$ `WaitingState`], [УСПЕШНО: списано 50 руб., остаток 150 руб.]
)

= Заключение

В рамках лабораторной работы №4 по варианту 5/1 спроектирована и реализована на языке C++ программная система «Телефон» на основе поведенческого шаблона проектирования «Состояние» (State):
- Разработана диаграмма конечных автоматов и специфицирована матрица реакций экземпляра класса `Phone` на операции `makeCall`, `answerCall`, `endCall` и `topUpBalance` в четырёх состояниях (`WaitingState`, `CallingState`, `TalkingState`, `BlockedState`).
- Построена UML-диаграмма классов и установлено взаимно-однозначное соответствие между классами программы и элементами шаблона GoF (`Context`, `State`, `ConcreteState`).
- Реализована библиотека классов `libphone_state.a`, консольное приложение `phone_app` и набор модульных тестов `test_phone`, подтвердивших корректность всех переходов и граничных условий (вероятность недозвона, сброс вызова до ответа, тарификация разговора, блокировка при исчерпании средств и разблокировка при пополнении счёта).
