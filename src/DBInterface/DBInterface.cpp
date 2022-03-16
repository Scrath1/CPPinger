#include <iostream>
#include "DBInterface.h"

DBInterface::DBInterface(const std::string& connectionString)
:connection(connectionString), defaultAddressID(-1)
{}

void DBInterface::closeConnection() {
    connection.close();
}

/// Creates the necessary tables if they don't exist already, compiles the prepared Statements for later queries
/// and inserts the event types into the eventtypes table (if they do not already exist)
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

/// Inserts and event into the events table. You have to either provide a targetID
/// (which can be acquired using getTargetIDWithAddress) or set a default target before
/// calling this function using setDefaultAddress
void DBInterface::insertEvent(const std::string& description, enum EventType eventType, int targetID) {
    int tID = (targetID==-1) ? defaultAddressID : targetID;
    assert(tID!=-1);
    pqxx::work transaction(connection);
    transaction.exec_prepared("insertEvent", description, eventToString(eventType), tID);
    transaction.commit();
}

/// Sets the defaultAddressID used by insertEvent if no targetID is provided.
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
                       "INSERT INTO eventtypes VALUES($1) ON CONFLICT DO NOTHING");
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

/// retrieves the id of the provided address from the database.
/// If the address is not in the database -1 is returned instead.
int DBInterface::getTargetIDWithAddress(const std::string& address) {
    pqxx::work transaction(connection);
    try{
        pqxx::row row = transaction.exec_prepared1("getTargetID", address);
        transaction.commit();
        return row[0].as<int>();
    }
    catch(pqxx::unexpected_rows& e){
        transaction.commit();
        return -1;
    }
}

/// inserts the event strings specified in Events.h into the database
void DBInterface::addEventTypes(pqxx::transaction_base& transaction) {
    for(const std::string& s:EventStrings){
        try{
            transaction.exec_prepared("registerEvent",s);
        }
        catch(pqxx::unique_violation& e){}
    }
}


