#include <iostream>
#include "DBInterface.h"

DBInterface::DBInterface(const std::string& connectionString)
:connection(connectionString), defaultAddressID(-1)
{}

void DBInterface::closeConnection() {
    connection.close();
}

void DBInterface::setupDB() {
    pqxx::work transaction(connection);
    const std::string eventTypesTable = "CREATE TABLE IF NOT EXISTS eventtypes ("
                                        " name  VARCHAR(32) PRIMARY KEY"
                                        ")";
    const std::string pingTargetTable = "CREATE TABLE IF NOT EXISTS pingtargets ("
                                        "   id  SERIAL PRIMARY KEY,"
                                        "   address VARCHAR(255) NOT NULL,"
                                        "   UNIQUE(address)"
                                        ")";
    const std::string eventsTable = "CREATE TABLE IF NOT EXISTS events ("
                                    "   timestamp     TIMESTAMP   DEFAULT now(),"
                                    "   description   TEXT,"
                                    "   name       VARCHAR(32)    NOT NULL REFERENCES eventtypes(name),"
                                    "   targetID      INT       NOT NULL REFERENCES pingtargets(ID),"
                                    "   PRIMARY KEY (Timestamp, TargetID)"
                                    ")";
    transaction.exec(eventTypesTable);
    transaction.exec(pingTargetTable);
    transaction.exec(eventsTable);
    prepareStatements();
    addEventTypes(transaction);
    transaction.commit();
}

void DBInterface::insertEvent(const std::string& description, enum EventType eventType, int targetID) {
    int tID = (targetID==-1) ? defaultAddressID : targetID;
    assert(tID!=-1);
    pqxx::work transaction(connection);
    transaction.exec_prepared("insertEvent", description, eventToString(eventType), tID);
    transaction.commit();
}

void DBInterface::setDefaultAddress(const std::string& address) {
    defaultAddressID=getTargetIDWithAddress(address);
}

void DBInterface::prepareStatements() {
    connection.prepare("insertEvent",
                       "INSERT INTO events VALUES(DEFAULT,$1,$2,$3)");
    connection.prepare("insertTarget",
                       "INSERT INTO pingtargets VALUES(DEFAULT,$1)");
    connection.prepare("getTargetID",
                       "SELECT id FROM pingtargets where address=$1");
    connection.prepare("registerEvent",
                       "INSERT INTO eventtypes VALUES($1)");
}

/// attempts to insert address into pingtargets.
/// Returns true if the insert was successfull.
/// Returns false if the address already exists.
bool DBInterface::insertTarget(const std::string& address) {
    try{
        pqxx::work transaction(connection);
        transaction.exec_prepared("insertTarget", address);
        transaction.commit();
        return true;
    }
    catch(pqxx::unique_violation& e){
        return false;
    }
}

int DBInterface::getTargetIDWithAddress(const std::string& address) {
    pqxx::work transaction(connection);
    pqxx::row row = transaction.exec_prepared1("getTargetID", address);
    transaction.commit();
    return row[0].as<int>();
//    TODO: Test for nonexistant address
}

/// inserts the event strings specified in Events.h into the database
void DBInterface::addEventTypes(pqxx::transaction_base& transaction) {
    for(const std::string& s:EventStrings){
        transaction.exec_prepared("registerEvent",s);
    }
}


