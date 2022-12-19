#ifndef SSLSERVER_H
#define SSLSERVER_H

#include <QTcpServer>
#include <QDebug>
#include <QThread>
#include "type.h"
#include "clientconnection.h"
#include "mainwindow.h"
#include "gost.h"

namespace server {

    class SslServer : public QTcpServer {
        Q_OBJECT
    public:
        SslServer(QString IP, int port);
        MainWindow* window;
    signals:
        void error(Error errorCode);
        void started();
    private slots:
        void startServer();
        void stopServer();
        void sendnames(QString name);
        void delname(QString name);
        void changename(QString name, QString n_name);
        void sendmessage(QString name, QString message);
        void sendstatus(QString name, QString status);
        void changeServer(QString nIP, int nPort);
        void sendmessagepic(QString name, QByteArray img);
        void changeavatar(QString name, QByteArray img);
        void changeKey();
        void sendFile(QString name, QString filename, QByteArray file);
    private:
        bool isStarted = false;
        void sendavatarname(QString name, QSslSocket* user);
        bool isReconnecting = false;
        QString serverIP;
        int serverPort;
        void incomingConnection(qintptr handle);
        ClientConnection* clientConnection;
        QList<ClientConnection*> _users;
        enum arguments{
            comAutchReq = (quint8)1,
            comUsersOnline = (quint8)2,
            comUserRename = (quint8)3,
            comUserLeft = (quint8)4,
            comMessageToAll = (quint8)5,
            comMessageToUsers = (quint8)6,
            comUserStatus = (quint8)7,
            comMessagePic = (quint8)8,
            comUserAvatar = (quint8)9,
            comUserFile = (quint8)10,
            comUserAvatarName = (quint8)201,
            comReconnect = (quint8)202
        };

    };
}

#endif // SSLSERVER_H
