# Клиент-Серверное Приложение на Си

## Описание
Это простое клиент-серверное приложение, реализующее функциональность калькулятора. Клиент отправляет выражения на сервер, который вычисляет результат и записывает его в файл. Приложение использует сокеты Беркли и многопоточность для одновременной обработки нескольких клиентов.

## Состав проекта

- **client/** - папка с исходным кодом клиента
  - `client.c` - исходный код клиента
- **server/** - папка с исходным кодом сервера
  - `server.c` - исходный код сервера
- **Makefile** - файл для сборки проекта

## Требования

- GCC (GNU Compiler Collection)
- Библиотека pthread
- FreeBSD или Linux (например, Arch Linux)

## Сборка

Для сборки проекта выполните следующие шаги:

1. Клонируйте репозиторий:
   ```sh
   git clone https://github.com/yourusername/yourrepository.git
   cd yourrepository
   ```
2. Скомпилируйте проект
   ```sh
   make all
   ```

## Запуск
### Сервер 

Запустите сервер, указав порт:
```sh
./server/server <порт>
```

### Клиент

Запустите клиента, указав адрес сервера и порт:
```sh
./client/client <адрес сервера> <порт>
```

## Использование
1. Запустите сервер.
2. Запустите клиента и введите выражение в формате `операнд1 оператор 
   операнд2`, например:
   ```sh 
   5 + 3
   ```
3. Для выхода из клиента введите `exit`.

### Пример взаимодействия
#### Клиент:
```sh
Enter an expression (operand1 operator operand2) or 'exit' to finish
Expression: 5 + 3
Result: 8.000000
```

#### Сервер:
```sh 
Connection from 127.0.0.1 at Mon Jun 17 17:03:13 2024
5.000000 + 3.000000 = 8.000000
```

## Примечания

- Убедитесь, что библиотека pthread установлена и доступна на вашей системе.
- Если вы используете FreeBSD, убедитесь, что все необходимые библиотеки    установлены и доступны.

## Лицензия

Этот проект лицензирован под лицензией MIT. Подробнее см. в LICENSE.