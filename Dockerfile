FROM ubuntu:latest

# Обновляем списки пакетов и устанавливаем Qt6 и необходимые зависимости
RUN apt-get update && apt-get -y upgrade
RUN apt-get update && apt-get -y install --fix-missing qt6-base-dev qt6-tools-dev qt6-tools-dev-tools build-essential

# Устанавливаем утилиту для поиска файлов
RUN apt-get install -y findutils

# Поиск qmake и создание символической ссылки
RUN ln -s /usr/lib/qt6/bin/qmake /usr/bin/qmake

# Копируем исходный код в контейнер
COPY . /code

# Проверяем версию qmake
RUN qmake --version

# Используем qmake для сборки проекта
RUN qmake /code/server/server.pro
RUN make

# Открываем порт 33333
EXPOSE 33333

CMD ["./server"]