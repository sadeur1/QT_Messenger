#include "mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow{parent}
{
    QMenu* settings = menuBar()->addMenu("Settings");
    QAction* network = new QAction("Network", this);
    connect(network, SIGNAL(triggered()), SLOT(setServer()));
    settings->addAction(network);
    QAction* changeKey = new QAction("Change Key", this);
    connect(changeKey, &QAction::triggered, [=](){
        emit reconnect();
    });
    settings->addAction(changeKey);
    QMenu* file = menuBar()->addMenu("File");
    QAction* on = new QAction("Start", this);
    connect(on, &QAction::triggered, [=](){
        emit start();
    });
    file->addAction(on);
    QAction* off = new QAction("Stop", this);
    connect(off, &QAction::triggered, [=](){
        emit stop();
    });
    file->addAction(off);
    QAction* saveXML = new QAction("Save in XML", this);
    connect(saveXML, SIGNAL(triggered()), SLOT(saveXML()));
    file->addAction(saveXML);
    QAction* exit = new QAction("Exit", this);
    connect(exit, SIGNAL(triggered()), SLOT(close()));
    file->addAction(exit);

    QGridLayout* layout = new QGridLayout;
    text = new QPlainTextEdit();
    text->setReadOnly(true);
    lwUsers = new QListWidget();
    layout->addWidget(text, 0,1);
    layout->addWidget(lwUsers,0,2);
    QWidget *widhet = new QWidget();
    widhet->setLayout(layout);
    setCentralWidget(widhet);


}

void MainWindow::setServer()
{
    QDialog* dialog = new QDialog(this);
    QGridLayout* l = new QGridLayout;
    QInputDialog* ip = new QInputDialog;
    QInputDialog* port = new QInputDialog;
    ip->setLabelText("IP");
    ip->setInputMode(QInputDialog::TextInput);
    ip->setOptions(QInputDialog::NoButtons);
    ip->setTextValue("127.0.0.1");
    port->setLabelText("Port");
    port->setInputMode(QInputDialog::IntInput);
    port->setOptions(QInputDialog::NoButtons);
    port->setIntRange(1025, 65535);
    port->setIntValue(45678);
    QPushButton* save = new QPushButton("Save");
    connect(save, &QPushButton::clicked, [=](){
        emit setServerProp(ip->textValue(), port->intValue());
        dialog->close();
    });
    QPushButton* cancel = new QPushButton("Cancel");
    connect(cancel, &QPushButton::clicked, [=](){
        dialog->close();

    });
    l->addWidget(ip, 0, 0 , 1, 2);
    l->addWidget(port, 1, 0, 1, 2);
    l->addWidget(save, 2, 0);
    l->addWidget(cancel,2, 1);
    dialog->setLayout(l);
    dialog->exec();


}

void MainWindow::saveXML()
{
     if (text->toPlainText().isEmpty())
     {
         QMessageBox::warning(this, "Warning","No messages to save!");
         return;
     }

     QFile file("./Server_messages.xml");
     file.open(QIODevice::WriteOnly);
     QXmlStreamWriter xmlWriter(&file);
     xmlWriter.setAutoFormatting(true);
     xmlWriter.writeStartDocument();
     xmlWriter.writeStartElement("messages");
     int k=1;
     for (auto i : text->toPlainText().split("\n"))
     {
         xmlWriter.writeStartElement("message");
         xmlWriter.writeAttribute("number",QString().setNum(k));
         xmlWriter.writeCharacters(i);
         xmlWriter.writeEndElement();
         k++;
     }
      xmlWriter.writeEndElement();
      xmlWriter.writeEndDocument();
      file.close();
}
