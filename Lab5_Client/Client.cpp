#include "Client.h"

Client::Client(QWidget *parent)
    : QMainWindow(parent), _nextBlockSize(0)
{
    if (_settings->value("name").toString() != "")
        _username = _settings->value("name").toString();
    if (_settings->value("status").toString() != "")
        _status = _settings->value("status").toString();
    if (_settings->value("IP").toString() != "")
        _ip = _settings->value("IP").toString();
    if (_settings->value("port").toInt() != 0)
        _port = _settings->value("port").toInt();
    if (_settings->value("showIP").toString() != "")
        _showIP = _settings->value("showIP").toBool();
    if (_settings->value("showTime").toString() != "")
        _showTime = _settings->value("showTime").toBool();
    if (_settings->value("autoLoad").toString() != "")
        autoLoad = _settings->value("autoLoad").toBool();
    if (_settings->value("BGColor").toString() != "")
    {
        const QColor color(_settings->value("BGColor").toString());
        QPalette* pallete = new QPalette;
        pallete->setColor(backgroundRole(), color);
        setPalette(*pallete);
    }
    if (_settings->value("MsgColor").toString() != "")
        _color = new QColor(_settings->value("MsgColor").toString());
    setWindowTitle("ServerIP: " + _ip +"    ServerPort: "+ QString::number(_port)+"    Nikname: "+_username+"    Status: "+_status);
    _widget = new QWidget;
    setCentralWidget(_widget);
    //----------------------------------------------------------------- Файл
    QMenu* fileMenu = menuBar()->addMenu("File");
    QAction* join = new QAction("Connect", this);
    connect(join, SIGNAL(triggered()), SLOT(onConnection()));
    fileMenu->addAction(join);
    QAction* disconnect = new QAction("Disconnect", this);
    connect(disconnect, SIGNAL(triggered()), SLOT(onDisconnection()));
    fileMenu->addAction(disconnect);
    QAction* saveToXMLFile = new QAction("Save to XML-File", this);
    connect(saveToXMLFile, SIGNAL(triggered()), SLOT(saveXML()));
    fileMenu->addAction(saveToXMLFile);
    QAction* exit = new QAction("Exit", this);
    connect(exit, SIGNAL(triggered()), SLOT(onExit()));
    fileMenu->addAction(exit);
    //----------------------------------------------------------------- Настройки
    QMenu* settingsMenu = menuBar()->addMenu("Settings");
    QAction* server = new QAction("Server", this);
    connect(server, SIGNAL(triggered()), SLOT(server()));
    settingsMenu->addAction(server);
    QAction* autoonof = new QAction("Auto load for file on/off", this);
    autoonof->setCheckable(true);
    autoonof->setChecked(autoLoad);
    connect(autoonof, &QAction::toggled, [=](bool i){
        autoLoad = i;
        _settings->setValue("autoLoad", autoLoad);
    });
    settingsMenu->addAction(autoonof);
    QAction* name = new QAction("User Name", this);
    connect(name, SIGNAL(triggered()), SLOT(userName()));
    settingsMenu->addAction(name);
    QMenu* statusBar = new QMenu("Status", this);
    QAction* online = new QAction("Online", this);
    connect(online, SIGNAL(triggered()), SLOT(isOnline()));
    statusBar->addAction(online);
    QAction* offline = new QAction("Offline", this);
    connect(offline, SIGNAL(triggered()), SLOT(isOffline()));
    statusBar->addAction(offline);
    QAction* not_active = new QAction("Not Active", this);
    connect(not_active, SIGNAL(triggered()), SLOT(isNotActive()));
    statusBar->addAction(not_active);
    if(_status != "Online" && _status != "Offline" && _status != "Not Active")
    {
        if (_status.size() > 16)
            yourStatus = new QAction(_status.remove(16, _status.size()) + "...", this);
        else
            yourStatus = new QAction(_status, this);
    }
    else
        yourStatus = new QAction("Your status", this);
    connect(yourStatus, SIGNAL(triggered()), SLOT(statusYour()));
    statusBar->addAction(yourStatus);
    settingsMenu->addMenu(statusBar);
    QAction* setPhoto = new QAction("Set Avatar", this);
    connect(setPhoto, SIGNAL(triggered()), SLOT(setAvatar()));
    settingsMenu->addAction(setPhoto);
    //----------------------------------------------------------------- Справка
    QMenu* about = menuBar()->addMenu("About");
    QAction* about_me = new QAction("About me", this);
    connect(about_me, SIGNAL(triggered()), SLOT(aboutMe()));
    about->addAction(about_me);
    //----------------------------------------------------------------- Вид
    QMenu* view = menuBar()->addMenu("View");
    QAction* backgroundColor = new QAction("Background color", this);
    connect(backgroundColor, SIGNAL(triggered()), SLOT(ChangeBgColor()));
    view->addAction(backgroundColor);
    QAction* messageColor = new QAction("Change message color");
    connect(messageColor, SIGNAL(triggered()), SLOT(ChangeMessageColor()));
    view->addAction(messageColor);
    QAction* shIP = new QAction("Show/Hide IP");
    shIP->setCheckable(true);
    shIP->setChecked(_showIP);
    connect(shIP, SIGNAL(toggled(bool)), SLOT(shIP(bool)));
    view->addAction(shIP);
    QAction* shTime = new QAction("Show/Hide message time");
    shTime->setCheckable(true);
    shTime->setChecked(_showTime);
    connect(shTime, SIGNAL(toggled(bool)), SLOT(shTime(bool)));
    view->addAction(shTime);
    //----------------------------------------------------------------- Окно
    _info = new QListWidget;
    _info->setIconSize(QSize(320, 240));
    _input = new QLineEdit;

    _input->setDisabled(true);

    _button = new QToolButton;
    _button->setPopupMode(QToolButton::MenuButtonPopup);
    QAction* send = new QAction("Send");
    QAction* sendpic = new QAction("Send Photo");
    QAction* sendFile = new QAction("Send File");

    _actions.push_back(send);
    _actions.push_back(sendpic);
    _actions.push_back(sendFile);

    connect(send, SIGNAL(triggered()), SLOT(sendToServer()));
    connect(sendpic, SIGNAL(triggered()), SLOT(sendPic()));
    connect(sendFile, SIGNAL(triggered()), SLOT(sendFile()));

    _button->setDefaultAction(send);
    _button->addAction(sendpic);
    _button->addAction(sendFile);

    _button->setDisabled(true);

    QGridLayout* l = new QGridLayout;
    _users = new QListWidget();
    l->addWidget(new QLabel("<H1>CLient<H1>"), 1, 1,1, 2, Qt::AlignLeft);
    l->addWidget(_info, 2 ,1, 1, 4);
    l->addWidget(_users, 2, 5, 1, 2, Qt::AlignLeft);
    l->addWidget(_input, 3, 1, 1, 3);
    l->addWidget(_button, 3, 4);
    _widget->setLayout(l);

    perDataSize = 64 * 1024;
    totalBytes = 0;
    bytestoWrite = 0;
    bytesWritten = 0;
    bytesReceived = 0;
    filenameSize = 0;

    _users->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(_users, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(showContextMenu(QPoint)));
    _info->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(_info, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(ContextMenuTextInfo(QPoint)));

}

void Client::onConnection()
{
    if (isconnect)
    {
        QMessageBox::warning(this, "Warning","You are already connected!");
        return;
    }

    _socket = new QSslSocket;

    _socket->setProtocol(QSsl::TlsV1_2);
    QByteArray key;
    QFile keyfile(QStringLiteral(":/key.key"));
    keyfile.open(QIODevice::ReadOnly);
    key = keyfile.readAll();
    QSslKey sslKey(key, QSsl::Rsa);
    _socket->setPrivateKey(sslKey);
    connect(_socket, SIGNAL(sslErrors(QList<QSslError>)), this, SLOT(sslError(QList<QSslError>)));
    _socket->connectToHostEncrypted(_ip,_port);
    if (_socket->waitForEncrypted())
    {
        isconnect = true;
        _input->setDisabled(false);
        _button->setDisabled(false);
        connect(_socket, SIGNAL(readyRead()), SLOT(ready2Read()));
        connect(_socket, SIGNAL(disconnected()), SLOT(disconnected()));
        connect(_socket, SIGNAL(error(QAbstractSocket::SocketError)), SLOT(Error(QAbstractSocket::SocketError)));
        sendname();
    }
    else {
        if (_socket->errorString() == "Connection refused")
            QMessageBox::warning(this, "Warning","No such host!");
        qDebug()<<"error:"<< _socket->errorString();
    }
}
void Client::sendname()
{
    QByteArray block;
    QDataStream out(&block, QIODevice::WriteOnly);
    out << (quint64)0;
    out << (quint8)Client::comAutchReq;
    out << _username << _status;
    out.device()->seek(0);
    out << (quint64)(block.size() - sizeof(quint64));
    _socket->write(block);
}

void Client::disconnected()
{
    isconnect = false;
    if (!fromdisocnnect)
        QMessageBox::warning(this, "Warning","Server closed!");
    fromdisocnnect = false;
    _input->setDisabled(true);
    _socket = nullptr;
    setWindowTitle("ServerIP: " + _ip +"    ServerPort: "+ QString::number(_port)+"    Nikname: "+_username+"    Status: "+_status);
    _button->setDisabled(true);
    _users->clear();
}
void Client::onDisconnection()
{
    fromdisocnnect =true;
    if (!isconnect )
    {
        QMessageBox::warning(this, "Warning","You are not connected!");
        return;
    }
    _input->setDisabled(true);
    _socket->disconnectFromHost();
    _socket->close();
    isconnect = false;
    _button->setDisabled(true);
    usersStatus.clear();
    usersname.clear();
    usersIP.clear();
    usersTime.clear();
    _users->clear();
}
void Client::saveXML()
{
    QString pass;
    if (_info->count() == 0)
    {
        QMessageBox::warning(this, "Warning","No messages to save!");
        return;
    }
    SaveToXML* save = new SaveToXML;
    if (save->isok)
    {
        pass = save->password->text();
    }
    else
    {
        return;
    }
    QFile file("./messages.xml");
    file.open(QIODevice::WriteOnly);
    QXmlStreamWriter xmlWriter(&file);
    xmlWriter.setAutoFormatting(true);
    xmlWriter.writeStartDocument();
    xmlWriter.writeStartElement("messages");
    int k=1;
    int q=0; // костыль - счетчик для файлов
    for (int i = 0; i < _info->count(); ++i)
    {
        xmlWriter.writeStartElement("message");
        xmlWriter.writeAttribute("number",QString().setNum(k));
        int f =0;
        QStringList g = _info->item(i)->text().split(":");
        qDebug() << _showIP+_showTime;
        for (auto j: QString("date ip name text").split(" "))
        {
            if (j != "text")
            {
                if (!_showIP && j == "ip")
                {
                    if ("you"==g[_showIP+_showTime])
                    {
                        xmlWriter.writeStartElement(j);
                        xmlWriter.writeCharacters("127.0.0.1");
                        xmlWriter.writeEndElement();
                        continue;
                    }
                    for (int ij = 0; ij < usersname.size(); ++ij)
                    {
                        if (usersname[ij] == g[_showIP+_showTime])
                        {
                            xmlWriter.writeStartElement(j);
                            xmlWriter.writeCharacters(usersIP[ij]);
                            xmlWriter.writeEndElement();
                            break;
                        }
                    }
                    continue;
                }
                if(!_showTime &&  j == "date")
                {
                    xmlWriter.writeStartElement(j);
                    xmlWriter.writeCharacters(usersMessageTime[i]);
                    xmlWriter.writeEndElement();
                    continue;
                }
                xmlWriter.writeStartElement(j);
                xmlWriter.writeCharacters(g[f]);
                xmlWriter.writeEndElement();
                f++;
            }
            else if (j == "text")
            {
                if(i + 1 < _info->count() && _info->item(i + 1)->text() == "")
                {
                    if (_info->item(i + 1)->whatsThis() != "file")
                    {
                        xmlWriter.writeStartElement("photo");
                        QImage image(_info->item(i + 1)->icon().pixmap(1920,1080).toImage());
                        QByteArray byteArray;
                        QBuffer buffer(&byteArray);
                        image.save(&buffer, "PNG");
                        QString iconBase64 = QString::fromLatin1(byteArray.toBase64().data());
                        xmlWriter.writeCharacters(iconBase64);
                        xmlWriter.writeEndElement();
                        f++;
                        i++;

                    }
                    else
                    {
                        QByteArray files;
                        int t=0;
                        for (int j =0; j < _info->count(); j++)
                        {
                            if (_info->item(j)->text() == "" && _info->item(j)->whatsThis() == "file")
                                t++;

                            if (_info->item(i+1) == _info->item(j))
                            {
                                files = _files[t-1];
                                break;
                            }
                        }
                        QString filename = _info->item(i)->text().remove(0, _info->item(i)->text().lastIndexOf(":") + 2);
                        QCryptographicHash hash(QCryptographicHash::Md5);
                        hash.addData(files);
                        xmlWriter.writeStartElement("file");
                        xmlWriter.writeAttribute("filename",filename);
                        xmlWriter.writeCharacters(hash.result().toHex());
                        xmlWriter.writeEndElement();
                        f++;
                        i++;
                    }

                }
            }
        }
        k++;
        xmlWriter.writeEndElement();
    }
    xmlWriter.writeEndElement();
    xmlWriter.writeEndDocument();
    file.close();
}

void Client::onExit()
{
    fromdisocnnect = true;
    if(isconnect)
        _socket->disconnectFromHost();
    this->close();
}

void Client::server()
{
    ServerSettingsDialog* server = new ServerSettingsDialog(_ip, _port);
    if (server->isSaved)
    {
        _ip = server->_ip;
        _port = server->_port;
        _settings->setValue("IP", _ip);
        _settings->setValue("port", _port);
    }
}

void Client::userName()
{
    QInputDialog* dialog = new QInputDialog;
    dialog->setInputMode(QInputDialog::TextInput);
    if (dialog->exec() == 1)
    {
        _username = dialog->textValue();
        _settings->setValue("name", _username);
        setWindowTitle("ServerIP: " + _ip +"    ServerPort: "+ QString::number(_port)+"    Nikname: "+_username+"    Status: "+_status);
        if(isconnect)
            sendname();
    }
}

void Client::isOnline()
{
    _status = "Online";
    _settings->setValue("status", _status);
    setWindowTitle("ServerIP: " + _ip +"    ServerPort: "+ QString::number(_port)+"    Nikname: "+_username+"    Status: "+_status);
    if(isconnect)
        sendstatus();
}

void Client::isOffline()
{
    _status = "Offline";
    _settings->setValue("status", _status);
    setWindowTitle("ServerIP: " + _ip +"    ServerPort: "+ QString::number(_port)+"    Nikname: "+_username+"    Status: "+_status);
    if(isconnect)
        sendstatus();
}

void Client::isNotActive()
{
    _status = "Not Active";
    _settings->setValue("status", _status);
    setWindowTitle("ServerIP: " + _ip +"    ServerPort: "+ QString::number(_port)+"    Nikname: "+_username+"    Status: "+_status);
    if(isconnect)
        sendstatus();
}

void Client::statusYour()
{
    QInputDialog* dialog = new QInputDialog;
    dialog->setInputMode(QInputDialog::TextInput);
    if (dialog->exec() == 1)
    {
        _status = dialog->textValue();
        if (_status.size() > 16)
            yourStatus->setText(_status.remove(16, _status.size()) + "...");
        else
            yourStatus->setText(_status);
        _settings->setValue("status", _status);
        setWindowTitle("ServerIP: " + _ip +"    ServerPort: "+ QString::number(_port)+"    Nikname: "+_username+"    Status: "+_status);
        if(isconnect)
            sendstatus();
    }
}

void Client::aboutMe()

{
    QDialog* about = new QDialog;
    QLabel* label = new QLabel("");
    QLabel* label2 = new QLabel("SADEUR");
    QLabel* label3 = new QLabel("Дата сборки: 9.11.2022 Версия Qt :"+QString(qVersion()) );
    QPushButton* close = new QPushButton("Close");
    connect(close, SIGNAL(clicked()), about,  SLOT(close()));
    QGridLayout* lay = new QGridLayout();
    label->setPixmap(QPixmap(":/1.png" ).scaled(320, 320));
    lay->addWidget(label, 0 , 1, 1, 2,Qt::AlignCenter);
    lay->addWidget(label2, 1,1,1,2,Qt::AlignCenter);
    lay->addWidget(label3, 2,1,1,2,Qt::AlignCenter);
    lay->addWidget(close, 3,1,1,2,Qt::AlignCenter);
    about->setLayout(lay);
    about->exec();

}

void Client::Error(QAbstractSocket::SocketError err)
{
    QString strError = "Error: " + (err == QAbstractSocket::HostNotFoundError ?
                                        "The host was not found." :
                                        err == QAbstractSocket::RemoteHostClosedError ?
                                            "The remote host is closed." :
                                            err == QAbstractSocket::ConnectionRefusedError ?
                                                "The connection was refused." :
                                                QString(_socket->errorString())
                                                );
    qDebug() << strError;
}

void Client::sendToServer()
{
    if(_showIP && _showTime)
        _info->addItem(QDateTime::currentDateTime().toString("dd.MM.yy HH|mm|ss")+":127.0.0.1:"+"you: " + _input->text());
    else if (!_showIP && _showTime)
        _info->addItem(QDateTime::currentDateTime().toString("dd.MM.yy HH|mm|ss")+":"+"you: " + _input->text());
    else if (_showIP && !_showTime)
        _info->addItem("127.0.0.1:" + (QString)"you: " + _input->text());
    else
        _info->addItem("you: " + _input->text());
    if(_color != nullptr)
        _info->item(_info->count() - 1)->setForeground(*_color);
    usersMessageTime.push_back(QDateTime::currentDateTime().toString("dd.MM.yy HH|mm|ss"));
    QByteArray arrBlock;
    QDataStream out(&arrBlock, QIODevice::WriteOnly);
    out << (quint64)0;
    out << (quint8)Client::comMessageToAll;
    out << _input->text();
    out.device()->seek(0);
    out << quint64(arrBlock.size() - sizeof(quint64));
    _input->setText("");
    _socket->write(arrBlock);
    _button->setDefaultAction(_actions[0]);

}

void Client::sendPic()
{
    _button->setDefaultAction(_actions[1]);
    if(_showIP && _showTime)
        _info->addItem(QDateTime::currentDateTime().toString("dd.MM.yy HH|mm|ss")+":127.0.0.1:"+"you: ");
    else if (!_showIP && _showTime)
        _info->addItem(QDateTime::currentDateTime().toString("dd.MM.yy HH|mm|ss")+":"+"you: ");
    else if (_showIP && !_showTime)
        _info->addItem("127.0.0.1:" + (QString)"you: ");
    else
        _info->addItem("you: ");
    if(_color != nullptr)
        _info->item(_info->count() - 1)->setForeground(*_color);
    usersMessageTime.push_back(QDateTime::currentDateTime().toString("dd.MM.yy HH|mm|ss"));
    usersMessageTime.push_back("");

    QByteArray arrBlock;
    QDataStream out(&arrBlock, QIODevice::WriteOnly);
    out << (quint64)0;
    out << (quint8)Client::comMessagePic;
    QFileDialog* fdialog = new QFileDialog(this, tr("Open image..."));
    fdialog->setAcceptMode(QFileDialog::AcceptOpen);
    fdialog->setFileMode(QFileDialog::ExistingFile);
    fdialog->setMimeTypeFilters({"image/jpeg", "image/png"});
    if(fdialog->exec() != QDialog::Accepted)
        return;
    QString fn = fdialog->selectedFiles().constFirst();
    QPixmap pic(fn);
    pictures.push_back(pic);
    QByteArray imageData;
    QBuffer buffer(&imageData);
    pic.toImage().save(&buffer, "PNG");
    out << imageData.toBase64();
    out.device()->seek(0);
    out << quint64(arrBlock.size() - sizeof(quint64));
    _socket->write(arrBlock);
    QListWidgetItem* item = new QListWidgetItem(QIcon(pic.scaled(320, 240)), "");
    _button->setDefaultAction(_actions[0]);
    _info->addItem(item);
}

void Client::sendFile()
{
    _button->setDefaultAction(_actions[2]);
    QByteArray block;
    QDataStream out(&block, QIODevice::WriteOnly);
    out << (quint64)0;
    out << (quint8)Client::comUserFile;
    QFileDialog* fdialog = new QFileDialog(this, tr("Open file..."));
    fdialog->setAcceptMode(QFileDialog::AcceptOpen);
    fdialog->setFileMode(QFileDialog::ExistingFile);
    if(fdialog->exec() != 1)
        return;
    QFile file = fdialog->selectedFiles().constFirst();
    if (!file.open(QIODevice::ReadOnly))
    {
        QMessageBox::warning(this, "Warning","Can't open the file");
        return;
    }
    totalBytes = file.size();
    out << file.fileName().remove(0, file.fileName().lastIndexOf("/") + 1);
    QByteArray q = file.readAll();
    block.append(q);
    file.close();
    totalBytes += block.size();
    out.device()->seek(0);
    out << (quint64)(block.size() - sizeof(quint64));
    bytestoWrite = totalBytes - _socket->write(block);

    if(_showIP && _showTime)
        _info->addItem(QDateTime::currentDateTime().toString("dd.MM.yy HH|mm|ss")+":127.0.0.1:"+"you: " + file.fileName().remove(0, file.fileName().lastIndexOf("/") + 1));
    else if (!_showIP && _showTime)
        _info->addItem(QDateTime::currentDateTime().toString("dd.MM.yy HH|mm|ss")+":"+"you: " + file.fileName().remove(0, file.fileName().lastIndexOf("/") + 1));
    else if (_showIP && !_showTime)
        _info->addItem("127.0.0.1:" + (QString)"you: " + file.fileName().remove(0, file.fileName().lastIndexOf("/") + 1));
    else
        _info->addItem("you: " + file.fileName().remove(0, file.fileName().lastIndexOf("/") + 1));
    if(_color != nullptr)
        _info->item(_info->count() - 1)->setForeground(*_color);
    usersMessageTime.push_back(QDateTime::currentDateTime().toString("dd.MM.yy HH|mm|ss"));
    usersMessageTime.push_back("");
    QListWidgetItem* item = new QListWidgetItem(QIcon(QPixmap(":/file.png").scaled(160, 120)), "");
    item->setWhatsThis("file");
    _info->addItem(item);
    _files.push_back(q);
}

void Client::ready2Read()
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
            qDebug() << _nextBlockSize;
        }
        if (clientSocket->bytesAvailable() < _nextBlockSize) {

            break;
        }
        else
            _nextBlockSize = 0;
        quint8 command;
        in >> command;
        qDebug() << command;
        switch(command)
        {
        case comUsersOnline:
        {
            QString users;
            in >> users;
            if (users == "")
                return;
            QStringList l =  users.split(",");
            for (int i = 0; i < l.length(); ++i)
            {
                QStringList a = l[i].split("$");
                _users->addItem(a[0]);
                usersname.push_back(a[0]);
                usersTime.push_back(a[1]);
                usersStatus.push_back(a[2]);
                usersIP.push_back("127.0.0.1");
            }
        }
            break;
        case comUserLeft:
        {
            QString user;
            in >> user;
            if (user == "")
                return;
            for (int i = 0; i < _users->count(); ++i)
            {
                if (_users->item(i)->text() == user)
                {
                    _users->takeItem(i);
                    break;
                }
            }
        }
            break;
        case comUserRename:
        {
            QString users;
            in >> users;
            if (users == "")
                return;
            QStringList l =  users.split(",");
            for (int i = 0; i < _users->count(); ++i)
            {
                if (_users->item(i)->text() == l[0])
                {
                    _users->item(i)->setText(l[1]);
                    break;
                }
            }
        }
            break;
        case comMessageToUsers:
        {
            QString users;
            in >> users;
            if (users == "")
                return;
            if (_status != "Not Active")
            {
                QSound* sound = new QSound(":/notification.wav");
                sound->play();
            }
            QStringList l =  users.split("✉");
            _info->addItem(QDateTime::currentDateTime().toString("dd.MM.yy HH|mm|ss")+ ":127.0.0.1"+":"+l[0] + ": " + l[1]);
            if(_color != nullptr)
                _info->item(_info->count() - 1)->setForeground(*_color);
            usersMessageTime.push_back(QDateTime::currentDateTime().toString("dd.MM.yy HH|mm|ss"));
        }
            break;
        case comUserStatus:
        {
            QString users;
            in >> users;
            if (users == "")
                return;
            QStringList l =  users.split("✉");
            for (int i = 0; i < usersname.size(); ++i)
            {
                if (usersname[i] == l[0])
                {
                    usersStatus[i] = l[1];
                    break;
                }
            }

        }
            break;
        case comMessagePic:
        {
            QByteArray img;
            in >> img;
            QImage pic;
            pic.loadFromData(QByteArray::fromBase64(img));
            pictures.push_back(QPixmap::fromImage(pic));
            QListWidgetItem* item = new QListWidgetItem(QIcon(QPixmap::fromImage(pic).scaled(320, 240)), "");
            _info->addItem(item);
            _nextBlockSize = 0;
            usersMessageTime.push_back("");
        }
            break;
        case comUserAvatarName:
        {
            QString users;
            in >> users;
            if (users == "" && avatarChangeName !="")
                return;
            avatarChangeName = users;
            _nextBlockSize = 0;
        }
            break;
        case comUserAvatar:
        {
            if (avatarChangeName =="")
                return;
            QByteArray img;
            in >> img;
            QImage pic;
            pic.loadFromData(QByteArray::fromBase64(img));
            for (int i = 0; i < _users->count(); ++i)
            {
                if (_users->item(i)->text() == avatarChangeName)
                {
                    _users->item(i)->setIcon(QIcon(QPixmap::fromImage(pic).scaled(320, 240)));
                    break;
                }
            }
            avatarChangeName ="";
            _nextBlockSize = 0;
        }
            break;
        case comReconnect:
        {
            onDisconnection();
            QThread::sleep(20);
            onConnection();
            _nextBlockSize = 0;
        }
            break;
        case comUserFile:
        {
            if (autoLoad)
            {
                progress = new QProgressBar;
                statusBar()->addWidget(progress);
                connect(_socket, SIGNAL(bytesWritten(qint64)), SLOT(updateFileProgress(qint64)));
                statusBar()->show();

                QString fileName;
                in >> fileName;
                QByteArray line = _socket->readAll();
                QListWidgetItem* item = new QListWidgetItem(QIcon(QPixmap(":/file.png").scaled(320, 240)), "");
                item->setWhatsThis("file");
                _info->addItem(item);
                _files.push_back(line);
                usersMessageTime.push_back("");
            }
        }
            break;
        }

    }
}

void Client::sslError(QList<QSslError> errors)
{
    _socket->ignoreSslErrors();
}

void Client::showContextMenu(const QPoint &pos)
{
    if(_users->itemAt(pos) == nullptr)
        return;
    QPoint globalPos = _users->mapToGlobal(pos);
    QMenu myMenu;
    myMenu.addAction("Info", this, [=](){
        QString name = _users->itemAt(pos)->text();
        int index;
        for (int i = 0; i < usersname.size(); ++i)
        {
            if (usersname[i] == name)
                index = i;
        }
        QMessageBox::information(this, "Info", usersIP[index] + " " + usersTime[index] + " " + usersStatus[index]);
    });
    myMenu.addAction("Send file", this, [=](){
        QDialog* dialog = new QDialog(this);
        dialog->setWindowTitle("Send file to " + _users->itemAt(pos)->text());
        QGridLayout* layout = new QGridLayout;
        QLineEdit* viewl = new QLineEdit;
        QPushButton* view = new QPushButton("View...");
        QPushButton* cancel = new QPushButton("Cancel");
        QPushButton* send = new QPushButton("Send");
        connect(view, &QPushButton::clicked, [=](){
            QFileDialog* fdialog = new QFileDialog(dialog, tr("Open image..."));
            fdialog->setAcceptMode(QFileDialog::AcceptOpen);
            fdialog->setFileMode(QFileDialog::ExistingFile);
            if(fdialog->exec() != QDialog::Accepted)
                return;
            viewl->setText(fdialog->selectedFiles().constFirst());
        } );
        connect(send, &QPushButton::clicked, [=](){
            dialog->done(1);
        } );
        connect(cancel, &QPushButton::clicked, [=](){
            dialog->done(0);
        } );
        layout->addWidget(viewl, 0, 0, 1, 3);
        layout->addWidget(view, 0, 4);
        layout->addWidget(send, 1, 0);
        layout->addWidget(cancel, 1, 1);
        dialog->setLayout(layout);
        dialog->show();
        if (dialog->result() == 1)
        {
            //filesend
        }
    });

    myMenu.exec(globalPos);
}

void Client::ContextMenuTextInfo(const QPoint &pos)
{
    if(_info->itemAt(pos) == nullptr)
        return;
    if(_info->itemAt(pos)->text() != "")
        return;

    QPoint globalPos = _info->mapToGlobal(pos);
    if (_info->itemAt(pos)->whatsThis() != "file")
    {
        int k = 0;
        QPixmap pic ;
        for (int i =0; i < _info->count(); i++)
        {
            if (_info->item(i)->text() == "" && _info->item(i)->whatsThis() != "file")
                k++;

            if (_info->itemAt(pos) == _info->item(i))
            {
                pic = pictures[k-1];
                break;
            }
        }

        QMenu myMenu;
        myMenu.addAction("Open in full size", this, [=](){
            QDialog* dialog = new QDialog;
            QLabel* label = new QLabel("");
            label->setPixmap(pic);
            QGridLayout* layout = new QGridLayout;
            layout->addWidget(label);
            dialog->setLayout(layout);
            dialog->show();
        });

        myMenu.addAction("Save", this, [=](){
            QString file = QFileDialog::getSaveFileName(0,"Save as...",QDir::currentPath(), "PNG (*.png);;JPG (*.jpeg *.jpg)");
            if (file.isNull())
                return;
            pic.save(file, "PNG",-1);
        });

        myMenu.exec(globalPos);
    }
    else
    {
        int k = 0;
        int index = 0;
        QByteArray files;
        for (int i =0; i < _info->count(); i++)
        {
            if (_info->item(i)->text() == "" && _info->item(i)->whatsThis() == "file")
                k++;

            if (_info->itemAt(pos) == _info->item(i))
            {
                files = _files[k-1];
                index = i;
                break;
            }
        }

        QMenu myMenu;
        myMenu.addAction("Open file", this, [=](){
            QString filename = _info->item(index - 1)->text().remove(0, _info->item(index - 1)->text().lastIndexOf(":") + 2);
            filename = filename.section("/", -1);
            QFile target("C:/Users/Turik/Documents/" + filename);
            if (("C:/Users/Turik/Documents/" + filename).isNull())
                return;
            qDebug() << QDir::currentPath() + "/Files/" + filename;
            if (!target.open(QIODevice::WriteOnly)) {
                qDebug() << "Can't open file for written";
                return;
            }
            target.write(files);
            target.close();
            //            QProcess *process = new QProcess(this);
            //            QProcess::startDetached(QDir::currentPath() + "/Files/" + filename);
            QDesktopServices::openUrl("C:/Users/Turik/Documents/" + filename);
        });

        myMenu.addAction("Save file", this, [=](){
            QString filename = _info->item(index - 1)->text().remove(0, _info->item(index - 1)->text().lastIndexOf(":") + 2);
            filename = filename.section("/", -1);
            QFileDialog* dialog = new QFileDialog(0,"Save as...",QDir::currentPath());
            dialog->setFileMode(QFileDialog::Directory);
            if(dialog->exec() != 1)
                return;
            QFile target(dialog->selectedFiles()[0] + "/" + filename);
            qDebug() << dialog->selectedFiles()[0] + "/" + filename;
            if ((dialog->selectedFiles()[0]  + "/" + filename).isNull())
                return;
            if (!target.open(QIODevice::WriteOnly)) {
                qDebug() << "Can't open file for written";
                return;
            }
            target.write(files);
            target.close();
        });
        myMenu.exec(globalPos);
    }
}

void Client::sendstatus()
{
    QByteArray block;
    QDataStream out(&block, QIODevice::WriteOnly);
    out << (quint64)0;
    out << (quint8)Client::comUserStatus;
    out << _status;
    out.device()->seek(0);
    out << (quint64)(block.size() - sizeof(quint64));
    _socket->write(block);
}

void Client::setAvatar()
{
    QFileDialog* fdialog = new QFileDialog(this, tr("Open image..."));
    fdialog->setAcceptMode(QFileDialog::AcceptOpen);
    fdialog->setFileMode(QFileDialog::ExistingFile);
    fdialog->setMimeTypeFilters({"image/jpeg", "image/png"});
    if(fdialog->exec() != QDialog::Accepted)
        return;
    QString fn = fdialog->selectedFiles().constFirst();
    QPixmap pic(fn);
    setWindowIcon(pic);
    if (isconnect)
    {
        QByteArray arrBlock;
        QDataStream out(&arrBlock, QIODevice::WriteOnly);
        out << (quint64)0;
        out << (quint8)Client::comUserAvatar;
        pictures.push_back(pic);
        QByteArray imageData;
        QBuffer buffer(&imageData);
        pic.toImage().save(&buffer, "PNG");
        out << imageData.toBase64();
        out.device()->seek(0);
        out << quint64(arrBlock.size() - sizeof(quint64));
        _socket->write(arrBlock);
    }
}
void Client::ChangeBgColor()
{
    QColorDialog* dialog = new QColorDialog(Qt::blue, this);
    if(dialog->exec() != 1)
        return;
    const QColor* color = new QColor(dialog->currentColor());
    _settings->setValue("BGColor", color->name());
    QPalette* pallete = new QPalette;
    pallete->setColor(backgroundRole(), *color);
    setPalette(*pallete);
}

void Client::shIP(bool isChecked)
{
    _showIP = isChecked;
    _settings->setValue("showIP", _showIP);
    if(_info->count() == 0)
        return;
    if (_showIP)
    {
        for (int i = 0; i < _info->count(); ++i)
        {
            if(_info->item(i)->text() != "")
            {
                QStringList tmp = _info->item(i)->text().split(":");
                if (_showTime)
                    _info->item(i)->setText(tmp[0] + ":127.0.0.1:" + tmp[1] + ":" + tmp[2]);
                else
                    _info->item(i)->setText("127.0.0.1:" + tmp[0] + ":" + tmp[1]);
            }
        }
    }
    else
    {
        for (int i = 0; i < _info->count(); ++i)
        {
            if(_info->item(i)->text() != "")
                _info->item(i)->setText(_info->item(i)->text().remove(_info->item(i)->text().indexOf("127.0.0.1"),10));
        }
    }
}

void Client::shTime(bool isChecked)
{
    _showTime = isChecked;
    _settings->setValue("showTime", _showTime);
    if(_info->count() == 0)
        return;
    if(_showTime)
    {
        for (int i = 0; i < _info->count(); ++i)
        {
            if(_info->item(i)->text() != "")
            {
                _info->item(i)->setText(usersMessageTime[i] + ":" + _info->item(i)->text());
            }
        }
    }
    else
    {
        for (int i = 0; i < _info->count(); ++i)
        {
            if(_info->item(i)->text() != "")
                _info->item(i)->setText(_info->item(i)->text().remove(0,18));
        }
    }

}

void Client::ChangeMessageColor()
{
    QDialog* dialog = new QDialog(this);
    QGridLayout* layout = new QGridLayout;
    QColorDialog* msgColor = new QColorDialog(Qt::blue);
    QLabel* msg = new QLabel("Message color");
    msgColor->setOptions(QColorDialog::NoButtons);
    QPushButton* save = new QPushButton("Save");
    QPushButton* cancel = new QPushButton("Cancel");
    connect(save, &QPushButton::clicked, [=](){
        dialog->done(1);
    } );
    connect(cancel, &QPushButton::clicked, [=](){
        dialog->done(0);
    } );
    layout->addWidget(msg, 0 , 0);
    layout->addWidget(msgColor, 1, 0);
    layout->addWidget(save, 2, 0);
    layout->addWidget(cancel, 2, 1);
    dialog->setLayout(layout);
    if(dialog->exec() == 1)
    {
        _color = new QColor(msgColor->currentColor());
        _settings->setValue("MsgColor", _color->name());
        for(int i = 0; i < _info->count(); ++i)
        {
            if(_info->item(i)->text() != "")
            {
                _info->item(i)->setForeground(*_color);
                _info->update();
            }
        }
    }
}


void Client::updateFileProgress(qint64 numBytes)
{
         // Data size that has been sent
    bytesWritten += (int)numBytes;

         // If the data has been sent
    if (bytestoWrite > 0)
    {
        outBlock = localFile->read(qMin(bytestoWrite, perDataSize));
                 // After the data is sent, the size of the remaining data is left
        bytestoWrite -= ((int)fileSocket->write(outBlock));
                 // Send the buffer area
        outBlock.resize(0);
    }
    else
        localFile->close();

//    qDebug() << "totalBytes==" << totalBytes << "bytesWritten== " << bytesWritten;
         // Update progress bar
    progress->setMaximum(totalBytes);
    progress->setValue(bytesWritten);

         // If the sending is completed
    if (bytesWritten >= totalBytes)
    {
        localFile->close();
        disconnect(fileSocket, SIGNAL(bytesWritten(qint64)), this, SLOT(updateFileProgress(qint64)));
        delete localFile;
        bytesWritten = 0;
        //fileSocket->close();
    }
}
