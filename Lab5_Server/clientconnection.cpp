#include "clientconnection.h"

ClientConnection::ClientConnection(MainWindow* window, QObject *parent)
    : QObject{parent}, _nextBlockSize(0)
{
    _window = window;
}
int ClientConnection::setSocket(int socketID) {
    this->socket = new QSslSocket(this);

    if (this->socket->setSocketDescriptor(socketID)) {
        this->_soketID = socketID;

        connect(this->socket, SIGNAL(readyRead()), this, SLOT(readyClientRead()));
        connect(socket, SIGNAL(disconnected()), SLOT(disconnected()));
        connect(this, SIGNAL(clientencrypted()),this, SLOT(connected()));
        connect(this->socket, SIGNAL(sslErrors(QList<QSslError>)), this, SLOT(socketSslErrors(QList<QSslError>)));
        connect(this->socket, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(socketError(QAbstractSocket::SocketError)));

        QByteArray key;
        QFile keyfile(QStringLiteral(":/key.key"));
        keyfile.open(QIODevice::ReadOnly);
        key = keyfile.readAll();
        QSslKey sslKey(key, QSsl::Rsa);

        QByteArray cert;
        QFile certfile(QStringLiteral(":/key.pem"));
        certfile.open(QIODevice::ReadOnly);
        cert = certfile.readAll();
        QSslCertificate sslCert(cert);

        this->socket->setLocalCertificate(sslCert);
        this->socket->setPrivateKey(sslKey);
        QSslConfiguration cfg = socket->sslConfiguration();
        cfg.caCertificates();
        socket->setPeerVerifyMode(QSslSocket::VerifyNone);
        this->socket->setProtocol(QSsl::TlsV1_2);

        this->socket->startServerEncryption();

        return 1;
    } else {
        delete this->socket;
        return 0;
    }
}

void ClientConnection::socketSslErrors(const QList<QSslError> & list)
{
    qDebug()<<"soket ssl error";
    foreach (QSslError item, list)
    {
        qDebug()<< item.errorString();
    }
}

void ClientConnection::socketError(QAbstractSocket::SocketError error)
{
    qDebug() << "socketError: "<< error;
}

void ClientConnection::connected()
{
    if (isconnect)
    {
        for (int i = 0; i < _window->lwUsers->count(); ++i)
        {
            if (_window->lwUsers->item(i)->text() == _username)
            {
                _window->text->appendPlainText(_window->lwUsers->item(i)->text()+" changed his nickname to "+_username);
                _window->lwUsers->takeItem(i);
                break;
            }

        }
        _window->lwUsers->addItem(_username);
        _window->update();
        return;
    }
    _window->text->appendPlainText(_username+" connected");
    _window->lwUsers->addItem(_username);
    _window->clientsConnected++;
    QString tmp = _window->windowTitle();
    tmp.replace(tmp.indexOf("d: ") + 3, 10, QString::number(_window->clientsConnected));
    _window->setWindowTitle(tmp);
    _window->update();
    isconnect=true;
}

void ClientConnection::readyClientRead()
{
    QSslSocket* clientSocket = (QSslSocket*)sender();
    QDataStream in(clientSocket);
    in.setVersion(QDataStream::Qt_5_6);
    for (;;)
    {
        if (!_nextBlockSize) {
            if (clientSocket->bytesAvailable() < sizeof(quint64)) {

                break;
            }

            in >> _nextBlockSize;
        }
        if (clientSocket->bytesAvailable() < _nextBlockSize) {

            break;
        }
        else
            _nextBlockSize = 0;
        quint8 command;
        in >> command;
        switch(command)
        {
        case comAutchReq:
        {
            if (isconnect)
            {
                QString name;
                QString status;
                in >> name >> status;
                _nextBlockSize = 0;
                for (int i = 0; i < _window->lwUsers->count(); ++i)
                {
                    if (_window->lwUsers->item(i)->text() == _username)
                    {
                        _window->text->appendPlainText(_window->lwUsers->item(i)->text()+" changed his nickname to "+name);
                        _window->lwUsers->takeItem(i);
                        break;
                    }

                }
                _window->lwUsers->addItem(name);
                _window->update();
                emit changename(_username, name);
                _username = name;
                return;
            }
            QString name;
            QString status;
            in >> name >> status;
            _username = name;
            _userStatus = status;
            _nextBlockSize = 0;
            _timeConnected = QTime::currentTime();
            emit sendnames(_username);
            emit clientencrypted();
        }
            break;
        case comMessageToAll:
        {
            QString str;
            in >> str;
            QString strMessage =
                    _username+" " +"has sent — " + str;
            _window->text->appendPlainText(strMessage);
            _window->update();
            _nextBlockSize = 0;
            emit sendmessage(_username, str);

        }
            break;
        case comUserStatus:
        {
            QString str;
            in >> str;
            _nextBlockSize = 0;
            _userStatus = str;
            emit sendstatus(_username, str);

        }
            break;
        case comMessagePic:
        {
            QByteArray img;
            in >> img;
            _nextBlockSize = 0;
            emit sendPic(_username, img);
        }
            break;
        case comUserAvatar:
        {
            QByteArray img;
            in >> img;
            _nextBlockSize = 0;
            emit sendAvatar(_username, img);

        }
            break;

        case comUserFile:
        {
            QString fileName;
            in >> fileName;
            qDebug() << fileName;
            QByteArray line = socket->readAll();
//            QString filePath = "./Files";
//            fileName = fileName.section("/", -1);
//            QFile target(filePath + "/" + fileName);
//            if (!target.open(QIODevice::WriteOnly)) {
//                qDebug() << "Can't open file for written";
//                return;
//            }
//            target.write(line);
//            target.close();
            emit sendFile(_username, fileName, line);
        }
            break;
        }
    }

}

void ClientConnection::disconnected()
{
    emit delname(_username);
    if (socket->state() == QAbstractSocket::ClosingState)
    {
        _window->text->appendPlainText(_username+" disconnected");
        for (int i = 0; i < _window->lwUsers->count(); ++i)
        {
            if (_window->lwUsers->item(i)->text() == _username)
            {
                _window->lwUsers->takeItem(i);
                break;
            }
        }
        _window->clientsConnected--;
        QString tmp = _window->windowTitle();
        tmp.replace(tmp.indexOf("d: ") + 3, 10, QString::number(_window->clientsConnected));
        _window->setWindowTitle(tmp);
        _window->update();
        connect(socket, SIGNAL(disconnected()), SLOT(deleteLater()));
    }
    else
    {
        _window->text->appendPlainText(_username+" disconnected");
        for (int i = 0; i < _window->lwUsers->count(); ++i)
        {
            if (_window->lwUsers->item(i)->text() == _username)
            {
                _window->lwUsers->takeItem(i);
                break;
            }
        }
        _window->clientsConnected--;
        QString tmp = _window->windowTitle();
        tmp.replace(tmp.indexOf("d: ") + 3, 10, QString::number(_window->clientsConnected));
        _window->setWindowTitle(tmp);
        _window->update();
        deleteLater();
        return;
    }
}
