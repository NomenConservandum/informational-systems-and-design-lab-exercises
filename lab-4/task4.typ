#set text(lang: "ru")

#align(center)[
    = Лабораторная работа №4
    *Разработка программных компонент на основе шаблонов проектирования* \
    *Вариант 5/1: Паттерн "Состояние" --- система "Телефон"*
]
#align(right)[
    Выполнил Храмов Александр, 451 группа, фКНиИТ
]

#outline()

#pagebreak()

= Постановка задачи (Вариант 5/1)

Целью работы является освоение методов объектно-ориентированного проектирования программных компонент на основе шаблонов проектирования (GoF).

В соответствии с вариантом *5/1* (задание №5 "Паттерн Состояние", подпункт 1 "Телефон") необходимо:
1. Разработать диаграмму конечных автоматов (диаграмму состояний) для системы "Телефон" и описать варианты реакции экземпляра класса на операции, вызываемые в указанных состояниях.
2. Разработать на основе шаблона проектирования *"Состояние" (State)* UML-диаграмму классов реализуемой программы и указать соответствие классов на этой диаграмме классам шаблона.
3. Разработать на объектно-ориентированном языке программирования (C++) библиотеку классов, включающую класс "Конечный автомат" (`Phone`), интерфейс "Состояние" (`PhoneState`) и классы "Конкретные состояния" (`WaitingState`, `CallingState`, `TalkingState`, `BlockedState`).
4. Разработать консольное приложение для управления состояниями экземпляров класса "Конечный автомат" и привести результаты тестирования программы.

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
        - `callProbability` --- вероятность поступления звонка ($p in [0.0, 1.0]$, атрибут модели по ТЗ; переходы детерминированы, без случайных отказов).

        *Операции:*
        - `makeCall(target)` --- позвонить (исходящий вызов, доступен при `balance >= 0`);
        - `answerCall()` --- ответить на звонок (входящий вызов бесплатен даже при `balance < 0`);
        - `endCall()` --- завершить разговор (или сбросить вызов);
        - `topUpBalance(amount)` --- пополнить баланс.
    ],
    [
        1. *Ожидание* (`WaitingState`)
        2. *Звонок* (`CallingState`)
        3. *Разговор* (`TalkingState`)
        4. *Заблокирован (баланс отрицательный)* (`BlockedState`)
    ]
)

#pagebreak()

= Шаблон проектирования "Состояние" (State) и соотношение с моделью программы

== Сущность шаблона "Состояние" (GoF)

Поведенческий шаблон проектирования *"Состояние" (State)* позволяет объекту изменять своё поведение в зависимости от внутреннего состояния. Внешне это выглядит так, словно объект меняет свой класс. Вместо громоздких условных конструкций (`switch` / `if-else`) внутри класса контекста, проверяющих текущий режим работы в каждом методе, паттерн выносит поведение каждого состояния в отдельный полиморфный класс, реализующий общий интерфейс `State`.

== Соответствие классов программы элементам шаблона GoF

В таблице ниже приведено прямое соответствие между каноническими участниками шаблона проектирования *State* и классами разработанной программной системы на языке C++:

#table(
    columns: (1.2fr, 1.4fr, 2.4fr),
    align: (left, left, left),
    [*Элемент шаблона GoF*], [*Класс / метод в программе*], [*Роль и ответственность в системе "Телефон"*],
    [`Context` \ (Контекст / Конечный автомат)],
    [Класс `phone::Phone`],
    [Хранит бизнес-атрибуты телефона (`phoneNumber_`, `balance_`, `callProbability_`, `callCost_`, флаг входящего звонка `isIncomingCall_`) и умный указатель `currentState_` на активный объект состояния. Делегирует вызовы `makeCall`, `answerCall`, `endCall`, `topUpBalance` объекту `currentState_` и предоставляет метод `setState()` для смены состояния.],
    [`State` \ (Интерфейс состояния)],
    [Абстрактный класс `phone::PhoneState`],
    [Определяет чистые виртуальные методы для всех 4 операций ТЗ (`makeCall`, `answerCall`, `endCall`, `topUpBalance`), каждый из которых принимает ссылку на контекст `Phone&`.],
    [`ConcreteStateA` \ (Конкретное состояние 1)],
    [Класс `phone::WaitingState`],
    [Реализует поведение телефона в состоянии *"Ожидание"* (`balance >= 0`): по `makeCall()` инициирует исходящий вызов с переходом в `CallingState`, по `answerCall()` принимает бесплатный входящий вызов с переходом в `TalkingState`; разрешает пополнение баланса; отклоняет `endCall()`.],
    [`ConcreteStateB` \ (Конкретное состояние 2)],
    [Класс `phone::CallingState`],
    [Реализует поведение в состоянии *"Звонок"*: по вызову `answerCall()` устанавливает соединение и переводит автомат в `TalkingState`; по `endCall()` сбрасывает вызов без списания средств с возвратом в `WaitingState` (`balance >= 0`) или `BlockedState` (`balance < 0`); запрещает повторный набор номера.],
    [`ConcreteStateC` \ (Конкретное состояние 3)],
    [Класс `phone::TalkingState`],
    [Реализует поведение в состоянии *"Разговор"*: при завершении исходящего разговора (`endCall()`) списывает стоимость вызова (`balance -= callCost`), а при завершении входящего списывает `0 руб.`; при `balance < 0` переводит телефон в `BlockedState`, а при `balance >= 0` возвращает в `WaitingState`.],
    [`ConcreteStateD` \ (Конкретное состояние 4)],
    [Класс `phone::BlockedState`],
    [Реализует поведение в состоянии *"Заблокирован (баланс отрицательный)"* (`balance < 0`): отклоняет исходящие вызовы (`makeCall`) и сброс (`endCall`), но разрешает приём бесплатных входящих вызовов (`answerCall`) с переходом в `TalkingState`; по `topUpBalance(amount)` при `balance >= 0` разблокирует телефон с переходом в `WaitingState`.],
    [`Client` \ (Клиент)],
    [Приложение `PhoneControllerApp` (`main.cpp`)],
    [Создаёт экземпляры конечного автомата `Phone`, управляет ими через публичный интерфейс контекста и отображает результаты переходов.]
)

#pagebreak()

= Диаграмма конечных автоматов (State Machine Diagram)

== Матрица реакций экземпляра класса `Phone` на операции

Поведение конечного автомата полностью определяется текущим состоянием и вызываемой операцией:

#{
  set text(size: 8.5pt)
  table(
    columns: (0.95fr, 1.15fr, 1.25fr, 1.25fr, 1.2fr),
    align: (left, left, left, left, left),
    [*Состояние*], [*`makeCall(target)`*], [*`answerCall()`*], [*`endCall()`*], [*`topUpBalance(amt)`*],
    [*`WaitingState`* \ (Ожидание)],
    [Проверка `balance >= 0`. Инициирование исходящего вызова $->$ переход в `CallingState`.],
    [Приём бесплатного входящего звонка (`0 руб.`) $->$ переход в `TalkingState`.],
    [Отказ: "Нет активного вызова или разговора". Состояние не меняется.],
    [При `amt > 0`: `balance += amt`. Остаётся в `WaitingState`.],
    [*`CallingState`* \ (Звонок)],
    [Отказ: "Телефон уже выполняет вызов". Состояние не меняется.],
    [Установление голосового соединения $->$ переход в `TalkingState`.],
    [Сброс/отмена вызова до ответа без списания $->$ `WaitingState` (`balance >= 0`) или `BlockedState` (`< 0`).],
    [При `amt > 0`: `balance += amt`. Остаётся в `CallingState`.],
    [*`TalkingState`* \ (Разговор)],
    [Отказ: "Линия занята, идёт разговор". Состояние не меняется.],
    [Предупреждение: "Соединение уже установлено". Состояние не меняется.],
    [Для исходящего списывает `callCost`, для входящего --- `0 руб.`. При `balance >= 0` $->$ `WaitingState`; при `< 0` $->$ `BlockedState`.],
    [При `amt > 0`: `balance += amt`. Остаётся в `TalkingState`.],
    [*`BlockedState`* \ (Заблокирован: баланс $< 0$)],
    [Отказ: "Исходящие вызовы заблокированы (отрицательный баланс)".],
    [Приём бесплатного входящего звонка при `balance < 0` (`0 руб.`) $->$ переход в `TalkingState`.],
    [Отказ: "Телефон заблокирован, активных разговоров нет".],
    [При `amt > 0`: `balance += amt`. При `balance >= 0` $->$ разблокировка (`WaitingState`), иначе остаётся в `BlockedState`.]
  )
}

== Граф переходов конечного автомата "Телефон"

При создании экземпляра `Phone` проверяется начальный баланс счёта: при `balance >= 0` (включая нулевой баланс) автомат переходит в начальное состояние `WaitingState`, а при `balance < 0` --- сразу в состояние `BlockedState`. Входящие звонки (`answerCall`) разрешены как при неотрицательном, так и при отрицательном балансе и не приводят к списанию средств со счёта абонента.

#align(center)[
  #figure(
    image("state_diagram.drawio.svg", width: 100%),
    caption: [Диаграмма конечных автоматов (State Machine Diagram) системы "Телефон"]
  )
]

#pagebreak()

= Диаграмма классов реализуемой программы (Class Diagram)

Библиотека классов шаблона "Состояние" (`src/phone_state_lib.hpp`, `src/phone_state_lib.cpp`) состоит из:
- класс контекста `Phone`, инкапсулирующий атрибуты телефона и делегирующий вызовы объекту состояния;
- абстрактный базовый класс `PhoneState`;
- четыре класса конкретных состояний (`WaitingState`, `CallingState`, `TalkingState`, `BlockedState`);
- структура `OperationResult`, возвращающая подробный отчёт о каждом вызове операции (успешность, сообщение, предыдущее и новое состояние).

#align(center)[
  #figure(
    image("class_diagram.drawio.svg", width: 100%),
    caption: [Диаграмма классов (Class Diagram) библиотеки и приложения на основе шаблона "Состояние"]
  )
]

#pagebreak()

= Программная реализация на языке C++

Ниже приведены ключевые фрагменты исходного кода библиотеки классов (`src/phone_state_lib.hpp` и `src/phone_state_lib.cpp`), демонстрирующие реализацию шаблона проектирования "Состояние".

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

/// Абстрактный класс (интерфейс) "Состояние" (State в шаблоне GoF)
class PhoneState {
public:
    virtual ~PhoneState() = default;
    virtual std::string getName() const = 0;
    virtual OperationResult makeCall(Phone& phone, const std::string& targetNumber) = 0;
    virtual OperationResult answerCall(Phone& phone) = 0;
    virtual OperationResult endCall(Phone& phone) = 0;
    virtual OperationResult topUpBalance(Phone& phone, int amount) = 0;
};

/// Класс "Телефон" / "Конечный автомат" (Context в шаблоне GoF)
class Phone {
private:
    std::string phoneNumber_;                  ///< Номер телефона абонента
    int balance_;                              ///< Текущий баланс (в руб.)
    double callProbability_;                   ///< Вероятность звонка (ТЗ)
    int callCost_;                             ///< Тариф исходящего звонка
    std::string activePeerNumber_;             ///< Номер собеседника
    bool isIncomingCall_;                      ///< Флаг входящего звонка
    std::shared_ptr<PhoneState> currentState_; ///< Текущее состояние автомата
    std::vector<std::string> historyLog_;      ///< Журнал переходов

public:
    Phone(std::string phoneNumber, int initialBalance,
          double callProbability = 1.0, int callCost = 50);

    // Делегирование операций текущему состоянию
    OperationResult makeCall(const std::string& targetNumber = "+7-900-000-00-00");
    OperationResult answerCall();
    OperationResult endCall();
    OperationResult topUpBalance(int amount);

    void setState(std::shared_ptr<PhoneState> newState);
    std::shared_ptr<PhoneState> getState() const;
    std::string getStateName() const;
    void addBalance(int amount);
    void deductCallCost();
    bool isIncomingCall() const;
    void setIncomingCall(bool incoming);
};

} // namespace phone
```

== Реализация переходов в конкретных состояниях (`src/phone_state_lib.cpp`)

```cpp
// Инициирование исходящего вызова в состоянии "Ожидание" (WaitingState)
OperationResult WaitingState::makeCall(Phone& phone, const std::string& targetNumber) {
    const std::string prevState = getName();
    if (phone.getBalance() < 0) {
        phone.setState(std::make_shared<BlockedState>());
        return {false, "Отрицательный баланс: исходящие вызовы заблокированы.",
                prevState, phone.getStateName()};
    }
    phone.setIncomingCall(false);
    phone.setActivePeerNumber(targetNumber);
    phone.setState(std::make_shared<CallingState>());
    return {true, "Инициирован исходящий звонок абоненту " + targetNumber,
            prevState, phone.getStateName()};
}

// Приём входящего звонка при отрицательном балансе (BlockedState)
OperationResult BlockedState::answerCall(Phone& phone) {
    return acceptIncomingCall(
        phone, getName(),
        "Принят входящий звонок при отрицательном балансе (бесплатно).");
}

// Завершение разговора в состоянии "Разговор" (TalkingState)
OperationResult TalkingState::endCall(Phone& phone) {
    const std::string prevState = getName();
    const std::string peer = phone.getActivePeerNumber();
    const bool wasIncoming = phone.isIncomingCall();
    if (!wasIncoming) {
        phone.deductCallCost();
    }
    phone.setIncomingCall(false);
    phone.setActivePeerNumber("");

    if (phone.getBalance() < 0) {
        phone.setState(std::make_shared<BlockedState>());
        return {true, "Разговор с " + peer + " завершён. Баланс отрицательный — блокировка!",
                prevState, phone.getStateName()};
    }
    phone.setState(std::make_shared<WaitingState>());
    return {true, "Разговор с " + peer + " завершён.", prevState, phone.getStateName()};
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
    ЗАПУСК МОДУЛЬНЫХ ТЕСТОВ БИБЛИОТЕКИ КЛАССОВ "ТЕЛЕФОН" (ПАТТЕРН "СОСТОЯНИЕ")
================================================================================
[PASS] testInitialStates — проверка начальных состояний телефона (0 >= 0 -> Ожидание, < 0 -> Заблокирован)
[PASS] testWaitingStateReactions — реакции в состоянии WaitingState и бесплатные входящие
[PASS] testCallingStateTransitions — переходы состояния CallingState
[PASS] testTalkingStateAndBlocking — тарификация исходящих и блокировка при balance < 0
[PASS] testBlockedStateAndUnblocking — блокировка при balance < 0, бесплатные входящие и разблокировка
================================================================================
 Завершение пяти тестовых наборов.
================================================================================
```

== Протокол демонстрационного сценария (`./build/phone_app --demo`)

В ходе демонстрационного сценария последовательно проверяются все 16 детерминированных переходов и реакций конечного автомата `+7-917-555-01-01`:

#{
  set text(size: 8.5pt)
  table(
    columns: (0.4fr, 1.55fr, 1.55fr, 1.5fr),
    align: (center, left, left, left),
    [*Шаг*], [*Вызываемая операция*], [*Переход состояний*], [*Результат и баланс*],
    [1], [`endCall()`], [`WaitingState` $->$ `WaitingState`], [ОТКЛОНЕНО: нет активного разговора (100 руб.)],
    [2], [`topUpBalance(50)`], [`WaitingState` $->$ `WaitingState`], [УСПЕШНО: пополнение +50 руб. (150 руб.)],
    [3], [`answerCall()` (входящий)], [`WaitingState` $->$ `TalkingState`], [УСПЕШНО: принят входящий звонок (150 руб.)],
    [4], [`endCall()` (входящий)], [`TalkingState` $->$ `WaitingState`], [УСПЕШНО: списано 0 руб., остаток 150 руб.],
    [5], [`makeCall(...)`], [`WaitingState` $->$ `CallingState`], [УСПЕШНО: идут гудки исходящего вызова (150 руб.)],
    [6], [`makeCall(...)` повторно], [`CallingState` $->$ `CallingState`], [ОТКЛОНЕНО: линия уже занята вызовом],
    [7], [`endCall()` до ответа], [`CallingState` $->$ `WaitingState`], [УСПЕШНО: сброс вызова без списания (150 руб.)],
    [8], [`makeCall()` + `answerCall()`], [`CallingState` $->$ `TalkingState`], [УСПЕШНО: соединение установлено (150 руб.)],
    [9], [`endCall()` (тариф 150 руб.)], [`TalkingState` $->$ `WaitingState`], [УСПЕШНО: списано 150 руб., остаток 0 руб. (>= 0)],
    [10], [`answerCall()` + `endCall()` (0 руб.)], [`TalkingState` $->$ `WaitingState`], [УСПЕШНО: бесплатный входящий, остаток 0 руб.],
    [11], [`makeCall()` + `answerCall()` + `endCall()` (50 руб.)], [`TalkingState` $->$ `BlockedState`], [УСПЕШНО: баланс -50 руб. (< 0) $->$ блокировка],
    [12], [`makeCall(...)` при -50 руб.], [`BlockedState` $->$ `BlockedState`], [ОТКЛОНЕНО: исходящие вызовы заблокированы],
    [13], [`answerCall()` при -50 руб.], [`BlockedState` $->$ `TalkingState`], [УСПЕШНО: принят бесплатный входящий (-50 руб.)],
    [14], [`endCall()` (входящий, -50 руб.)], [`TalkingState` $->$ `BlockedState`], [УСПЕШНО: списано 0 руб., баланс -50 руб.],
    [15], [`topUpBalance(200)`], [`BlockedState` $->$ `WaitingState`], [УСПЕШНО: баланс 150 руб. (>= 0), разблокировка],
    [16], [Звонок + разговор + `endCall()` (50 руб.)], [`TalkingState` $->$ `WaitingState`], [УСПЕШНО: списано 50 руб., остаток 100 руб.]
  )
}

= Заключение

В рамках лабораторной работы №4 по варианту 5/1 спроектирована и реализована на языке C++ программная система "Телефон" на основе поведенческого шаблона проектирования "Состояние" (State):
- Разработана диаграмма конечных автоматов и специфицирована матрица реакций экземпляра класса `Phone` на операции `makeCall`, `answerCall`, `endCall` и `topUpBalance` в четырёх состояниях (`WaitingState`, `CallingState`, `TalkingState`, `BlockedState`).
- Построена UML-диаграмма классов и установлено взаимно-однозначное соответствие между классами программы и элементами шаблона GoF (`Context`, `State`, `ConcreteState`).
- Реализована библиотека классов `libphone_state.a`, консольное приложение `phone_app` и набор модульных тестов `test_phone`, подтвердивших детерминированную корректность всех переходов и граничных условий (бесплатный приём входящих вызовов в `WaitingState` и `BlockedState`, сброс вызова до ответа, тарификация исходящих разговоров, блокировка исходящих при `balance < 0` и разблокировка при достижении `balance >= 0`).
