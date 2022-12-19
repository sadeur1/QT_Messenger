#ifndef TYPE_H
#define TYPE_H
namespace server {

    enum class Error:int {
        SERVER_START_ERROR = 1,
        SOCKET_SETUP_ERROR = 2};
}
#endif // TYPE_H
