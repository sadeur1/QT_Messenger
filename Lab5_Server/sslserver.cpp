#include "sslserver.h"
using namespace server;

SslServer::SslServer(QString IP, int port) {
    serverIP = IP;
    serverPort = port;
    window = new MainWindow();
    window->setWindowTitle("IP: " + serverIP + " Port: " + QString::number(serverPort) + " Clients connected: 0");
    connect(window, SIGNAL(setServerProp(QString, int)), SLOT(changeServer(QString, int)));
    connect(window, SIGNAL(reconnect()), SLOT(changeKey()));
    connect(window, SIGNAL(start()), SLOT(startServer()));
    connect(window, SIGNAL(stop()), SLOT(stopServer()));
}


void SslServer::startServer() {
    if(isStarted)
    {
        QMessageBox::warning(window, "Warning","Server already started!");
        return;
    }
    if (listen(QHostAddress(serverIP), serverPort)) {

        window->text->appendPlainText("Server start on port:" + QString::number(serverPort));
        isStarted = true;
        emit started();
    } else {
        window->text->appendPlainText("Cant satrt server on port:"+ QString::number(serverPort));
        emit error(Error::SERVER_START_ERROR);
        return;
    }
    return;
}

void SslServer::stopServer()
{
    if(!isStarted)
    {
        QMessageBox::warning(window, "Warning","Server already stoped!");
        return;
    }
    for (int i =0; i < _users.length(); i++)
    {
        _users[i]->socket->close();
    }
    close();
    _users.clear();
    isStarted = false;
    window->text->appendPlainText("Server stopped :(");
}

void SslServer::incomingConnection(qintptr handle) {
    clientConnection = new ClientConnection(window);

    isReconnecting = false;
    if (clientConnection->setSocket(handle))
    {
        connect(clientConnection, SIGNAL(sendnames(QString)), SLOT(sendnames(QString)) );
        connect(clientConnection, SIGNAL(delname(QString)), SLOT(delname(QString)));
        connect(clientConnection, SIGNAL(changename(QString,QString)), SLOT(changename(QString, QString)));
        connect(clientConnection, SIGNAL(sendmessage(QString,QString)), SLOT(sendmessage(QString, QString)));
        connect(clientConnection, SIGNAL(sendstatus(QString,QString)), SLOT(sendstatus(QString, QString)));
        connect(clientConnection, SIGNAL(sendPic(QString,QByteArray)), SLOT(sendmessagepic(QString, QByteArray)));
        connect(clientConnection, SIGNAL(sendAvatar(QString,QByteArray)), SLOT(changeavatar(QString, QByteArray)));
        connect(clientConnection, SIGNAL(sendFile(QString,QString,QByteArray)), SLOT(sendFile(QString, QString, QByteArray)));
        _users.push_back(clientConnection);
        QThread* threadForClient = new QThread();
        clientConnection->moveToThread(threadForClient);
        threadForClient->start();
    } else {
        window->text->appendPlainText("socket setup error;");
        delete clientConnection;
        emit error(Error::SOCKET_SETUP_ERROR);
    }

}

void SslServer::sendnames(QString name)
{
    if (_users.length() <= 1)
        return;
    ClientConnection* sendto;
    QStringList l;
    for (int i =0; i <_users.length(); i++){
        if (_users[i]->_username == name)
        {
            sendto = _users[i];
        }
        else
        {
            l << _users[i]->_username + "$" + _users[i]->_timeConnected.toString() + "$" + _users[i]->_userStatus;
            QByteArray block;
            QDataStream out(&block, QIODevice::WriteOnly);
            out << (quint64)0;
            out << (quint8)SslServer::comUsersOnline;
            QString s;
            s += _users[_users.size() - 1]->_username + "$" + _users[_users.size() - 1]->_timeConnected.toString("HH:mm:ss") + "$" + _users[_users.size() - 1]->_userStatus;
            out << s;
            out.device()->seek(0);
            out << (quint64)(block.size() - sizeof(quint64));
            _users[i]->socket->write(block);
        }

    }
    QByteArray block;
    QDataStream out(&block, QIODevice::WriteOnly);
    out << (quint64)0;
    out << (quint8)SslServer::comUsersOnline;
    QString s;
    for (int i = 0; i < l.length(); ++i)
        if (l.at(i) != name)
            s += l.at(i)+(QString)",";
    s.remove(s.length()-1, 1);
    out << s;
    out.device()->seek(0);
    out << (quint64)(block.size() - sizeof(quint64));
    sendto->socket->write(block);
}

void SslServer::delname(QString name)
{
    if(isReconnecting)
        return;
    int index;
    for (int i =0; i <_users.length(); i++){
        if (_users[i]->_username != name)
        {
            QByteArray block;
            QDataStream out(&block, QIODevice::WriteOnly);
            out << (quint64)0;
            out << (quint8)SslServer::comUserLeft;
            QString s;
            s += name;
            out << s;
            out.device()->seek(0);
            out << (quint64)(block.size() - sizeof(quint64));
            _users[i]->socket->write(block);
        }
        else
        {
            index = i;
        }
    }
    _users.removeAt(index);
}

void SslServer::changename(QString name, QString n_name)
{
    for (int i =0; i <_users.length(); i++)
    {
        if (_users[i]->_username != name)
        {
            QByteArray block;
            QDataStream out(&block, QIODevice::WriteOnly);
            out << (quint64)0;
            out << (quint8)SslServer::comUserRename;
            QString s;
            s += name +(QString)",";
            s += n_name;
            out << s;
            out.device()->seek(0);
            out << (quint64)(block.size() - sizeof(quint64));
            _users[i]->socket->write(block);
        }
    }
}

void SslServer::sendmessage(QString name, QString message)
{
    for (int i =0; i <_users.length(); i++)
    {
        if (_users[i]->_username != name)
        {
            QByteArray block;
            QDataStream out(&block, QIODevice::WriteOnly);
            out << (quint64)0;
            out << (quint8)SslServer::comMessageToUsers;
            QString s;
            s += name +(QString)"✉";
            s += message;
            out << s;
            out.device()->seek(0);
            out << (quint64)(block.size() - sizeof(quint64));
            qDebug() << (quint64)(block.size() - sizeof(quint64));
             _users[i]->socket->write(block);
        }
    }
}
void SslServer::sendmessagepic(QString name, QByteArray img)
{
    for (int i =0; i <_users.length(); i++)
    {
        if (_users[i]->_username != name)
        {
            sendmessage(name, "");
            QByteArray block;
            QDataStream out(&block, QIODevice::WriteOnly);
            out << (quint64)0;
            out << (quint8)SslServer::comMessagePic;
            out << img;
            out.device()->seek(0);
            out << (quint64)(block.size() - sizeof(quint64));
            _users[i]->socket->write(block);
        }
    }
}
void SslServer::sendstatus(QString name, QString status)
{
    for (int i =0; i <_users.length(); i++)
    {
        if (_users[i]->_username != name)
        {
            QByteArray block;
            QDataStream out(&block, QIODevice::WriteOnly);
            out << (quint64)0;
            out << (quint8)SslServer::comUserStatus;
            QString s;
            s += name +(QString)"✉";
            s += status;
            out << s;
            out.device()->seek(0);
            out << (quint64)(block.size() - sizeof(quint64));
            _users[i]->socket->write(block);
        }
    }
}

void SslServer::changeavatar(QString name, QByteArray img)
{
    for (int i =0; i <_users.length(); i++)
    {
        if (_users[i]->_username != name)
        {
            sendavatarname(name, _users[i]->socket);
            QByteArray block;
            QDataStream out(&block, QIODevice::WriteOnly);
            out << (quint64)0;
            out << (quint8)SslServer::comUserAvatar;
            out << img;
            out.device()->seek(0);
            out << (quint64)(block.size() - sizeof(quint64));
            _users[i]->socket->write(block);
        }
    }
}
void SslServer::sendavatarname(QString name, QSslSocket* user)
{
    QByteArray block;
    QDataStream out(&block, QIODevice::WriteOnly);
    out << (quint64)0;
    out << (quint8)SslServer::comUserAvatarName;
    QString s;
    s = name;
    out << s;
    out.device()->seek(0);
    out << (quint64)(block.size() - sizeof(quint64));
    user->write(block);
}

void SslServer::sendFile(QString name, QString filename, QByteArray file)
{
    for (int i =0; i <_users.length(); i++)
    {
        if (_users[i]->_username != name)
        {
            sendmessage(name, filename);
            QByteArray block;
            QDataStream out(&block, QIODevice::WriteOnly);
            out << (quint64)0;
            out << (quint8)SslServer::comUserFile;
            out << filename;
            block.append(file);
            out.device()->seek(0);
            out << (quint64)(block.size() - sizeof(quint64));
            _users[i]->socket->write(block);
        }
    }
}

void SslServer::changeServer(QString nIP, int nPort)
{
    for (int i =0; i < _users.length(); i++)
    {
        _users[i]->socket->close();
    }
    close();
    isStarted = false;
    serverIP = nIP;
    serverPort = nPort;
    window->text->appendPlainText("IP & Port of the server has been changed: IP - " + serverIP + " Port - " + QString::number(serverPort));
    _users.clear();
    window->setWindowTitle("IP: " + serverIP + " Port: " + QString::number(serverPort) + " Clients connected: 0");
}

void SslServer::changeKey()
{
    isReconnecting = true;
    for (int i =0; i < _users.length(); i++)
    {
        QByteArray block;
        QDataStream out(&block, QIODevice::WriteOnly);
        out << (quint64)0;
        out << (quint8)SslServer::comReconnect;
        out.device()->seek(0);
        out << quint64(block.size() - sizeof(quint64));
        _users[i]->socket->write(block);
        QThread::sleep(2);
    }
    _users.clear();
    window->lwUsers->clear();

    int ret = 0;
    RSA* r = NULL;
    BIGNUM* bne = NULL;
    BIO* bp_public = NULL, *bp_private = NULL;

    int bits = 2048;
    unsigned long e = RSA_F4;

    bne = BN_new();
    ret = BN_set_word(bne,e);
    if(ret != 1){
        goto free_all;
    }

    r = RSA_new();
    ret = RSA_generate_key_ex(r, bits, bne, NULL);
    if(ret != 1){
        goto free_all;
    }

    bp_private = BIO_new_file("key.pem", "w+");
    ret = PEM_write_bio_RSAPrivateKey(bp_private, r, NULL, NULL, 0, NULL, NULL);

free_all:

    BIO_free_all(bp_public);
    BIO_free_all(bp_private);
    RSA_free(r);
    BN_free(bne);
}

