#ifndef SERVER_H
#define SERVER_H

#include <QTcpServer>
#include <QTcpSocket>
#include <QVector>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>

class Server : public QTcpServer
{
    Q_OBJECT

private:
    QSqlDatabase db;

public:
    Server();
    QTcpSocket *socket;

private:
    QVector<QTcpSocket*> Sockets;
    QByteArray Data;
    void SendToClient(QString str);
    bool initDatabase();

public slots:
    void incomingConnection(qintptr socketDescriptor);
    void slotReadyRead();
    void processClientMessage(const QString &message);
    void displayAllUsers();
    void signUp(const QString &login, const QString &password);
    void loginUser(const QString &login, const QString &password);
};

#endif // SERVER_H
