#include "server.h"
#include <QDebug>
#include <QStringList>
#include <QSqlError>
#include <QSqlQuery>
#include <QSqlRecord>

Server::Server()
{
    // Инициализировать БД
    if (!initDatabase())
    {
        qDebug() << "Failed to initialize the database";
    }

    // Подключение к порту
    if (this->listen(QHostAddress::Any, 33333))
    {
        qDebug() << "Server starts";
    } else {
        qDebug() << "Error starting the server";
    }
}

bool Server::initDatabase()
{
    // Подключение к БД
    db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName("Test.db");

    if (!db.open())
    {
        qDebug() << db.lastError().text();
        return false;
    }

    // Создание таблицы, если её не существует
    QSqlQuery query(db);
    query.exec("CREATE TABLE IF NOT EXISTS User("
               "login VARCHAR(20) NOT NULL, "
               "password VARCHAR(20) NOT NULL"
               ")");
    return true;
}

void Server::incomingConnection(qintptr socketDescriptor)
{
    socket = new QTcpSocket;
    socket->setSocketDescriptor(socketDescriptor);
    connect(socket, &QTcpSocket::readyRead, this, &Server::slotReadyRead);
    connect(socket, &QTcpSocket::disconnected, socket, &QTcpSocket::deleteLater);

    Sockets.push_back(socket);
    qDebug() << "Client connected" << socketDescriptor;
}

void Server::slotReadyRead()
{
    QTcpSocket* senderSocket = static_cast<QTcpSocket*>(QObject::sender());
    QByteArray data = senderSocket->readAll();
    QString str(data);
    processClientMessage(str); // Обработка сообщения от клиента
}

void Server::processClientMessage(const QString &message)
{
    // Разделение команды по частям
    QStringList parts = message.split(" ");

    if (parts[0] == "sign_up" && parts.size() == 3)
    {
        QString login = parts[1];
        QString password = parts[2];
        signUp(login, password);
    } else if (parts[0] == "login" && parts.size() == 3)
    {
        QString login = parts[1];
        QString password = parts[2];
        loginUser(login, password);
    } else if (parts[0] == "get_users")
    {
        displayAllUsers();
    } else {
        SendToClient("Неизвестная команда: " + parts[0]);
    }
}

// Отображение всех записей
void Server::displayAllUsers()
{
    QSqlQuery query("SELECT login FROM User");
    QString usersList;

    // Сборка логинов с БД
    while (query.next())
    {
        usersList += query.value(0).toString() + "\n";
    }

    if (usersList.isEmpty())
    {
        usersList = "Нет пользователей.";
    }

    SendToClient(usersList); // Отображение логинов
}

void Server::signUp(const QString& login, const QString& password)
{
    // Проверка существует ли уже пользователь с таким логином
    QSqlQuery query(db);
    query.prepare("SELECT login FROM User WHERE login = :login");
    query.bindValue(":login", login);
    query.exec();

    if (query.next())
    {
        SendToClient("Пользователь с таким логином уже существует.");
    } else
    {
        // Создание новой запси
        query.prepare("INSERT INTO User(login, password) VALUES (:login, :password)");
        query.bindValue(":login", login);
        query.bindValue(":password", password);

        if (query.exec())
        {
            SendToClient("Запись зарегистрирована.");
        } else
        {
            SendToClient("Ошибка при регистрации.");
        }
    }
}

void Server::loginUser(const QString& login, const QString& password)
{
    // Проверка существуют ли логин и пароль в базе данных
    QSqlQuery query(db);
    query.prepare("SELECT password FROM User WHERE login = :login");
    query.bindValue(":login", login);
    query.exec();

    if (query.next())
    {
        QString storedPassword = query.value(0).toString();
        if (storedPassword == password)
        {
            SendToClient("Вход выполнен.");
        } else
        {
            SendToClient("Неправильный пароль.");
        }
    } else
    {
        SendToClient("Пользователь не найден.");
    }
}

void Server::SendToClient(QString str)
{
    QByteArray data;
    QDataStream out(&data, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_6_2);
    out << str;

    for (QTcpSocket* socket : Sockets)
    {
        socket->write(data);
    }
}
