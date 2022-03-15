#ifndef CPPINGER_DBINTERFACE_H
#define CPPINGER_DBINTERFACE_H
#include <pqxx/pqxx>
#include "Events.h"
class DBInterface {
public:
    explicit DBInterface(const std::string& connectionString);
    void closeConnection();
    void setupDB();
    bool insertTarget(const std::string& address);
    void insertEvent(const std::string& description, enum EventType eventType, int targetID = -1);
    void setDefaultAddress(const std::string& address);
    int getTargetIDWithAddress(const std::string& address);
private:
    void prepareStatements();
    static void addEventTypes(pqxx::transaction_base& transaction);
    pqxx::connection connection;
    int defaultAddressID{};
};


#endif //CPPINGER_DBINTERFACE_H
