#ifndef CLIENTCONNECTION_H
#define CLIENTCONNECTION_H


#include <QSslSocket>
#include <QSslKey>
#include <QSslConfiguration>
#include <QList>
#include <QTime>
#include "mainwindow.h"

class ClientConnection : public QObject
{
    Q_OBJECT
public:
    explicit ClientConnection(MainWindow* window, QObject *parent = nullptr);
    int setSocket(int socketID);
    QString _username;
    QString _userStatus;
    QTime _timeConnected;
    QSslSocket* socket;
signals:
    void clientencrypted();
    QString sendnames(QString);
    QString delname(QString);
    QString changename(QString, QString);
    QString sendmessage(QString, QString);
    QString sendstatus(QString, QString);
    QString sendPic(QString, QByteArray);
    QString sendAvatar(QString, QByteArray);
    QString sendFile(QString, QString, QByteArray);
private slots:
    void socketError(QAbstractSocket::SocketError error);
    void socketSslErrors(const QList<QSslError> & list);
    void connected();
    void readyClientRead();
    void disconnected();
private:
    bool isconnect = false;
    MainWindow* _window;
    int _soketID;
    quint64 _nextBlockSize;
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

#endif // CLIENTCONNECTION_H
