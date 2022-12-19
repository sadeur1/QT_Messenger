#include <QApplication>
#include "sslserver.h"
using namespace server;
int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    SslServer s("127.0.0.1",45678);
    s.window->show();
    return a.exec();
}
