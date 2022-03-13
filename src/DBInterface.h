#ifndef CPPINGER_DBINTERFACE_H
#define CPPINGER_DBINTERFACE_H
#include <string>
#include "pqxx"
class DBInterface {
public:
    DBInterface();
    void connect(std::string address);
private:
    pqxx::connection connection;
};


#endif //CPPINGER_DBINTERFACE_H
