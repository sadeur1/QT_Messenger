#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QPlainTextEdit>
#include <QListWidget>
#include <QGridLayout>
#include <QMenuBar>
#include <QDialog>
#include <QInputDialog>
#include <QPushButton>
#include <QMessageBox>
#include <QtXml>

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = nullptr);
    QPlainTextEdit*  text;
    QListWidget* lwUsers;
    int clientsConnected = 0;
signals:
    QString setServerProp(QString, int);
    void reconnect();
    void start();
    void stop();
private slots:
    void setServer();
    void saveXML();
};

#endif // MAINWINDOW_H
