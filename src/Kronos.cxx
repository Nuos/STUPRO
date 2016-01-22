#include <Kronos.h>

#include <Utils/Misc/Exceptions.hpp>

#include <exception>
#include <iostream>
#include <QString>

Kronos* Kronos::instance = 0;

Kronos* Kronos::getInstance() {
    if (Kronos::instance == 0) {
        throw new InvalidStateException("No Kronos instance exists at the moment");
    }
    return Kronos::instance;
}



Kronos::Kronos(QObject* parent) : QObject(parent) {
    std::cout << "### KRONOS MAIN INIT ###" << std::endl;

    if (Kronos::instance != NULL) {
        throw new InvalidStateException("Only exactly one instance of Kronos may be initialized");
    }
    Kronos::instance = this;

    this->citiesDatabase = 0;
    this->initialized = false;
}

Kronos::~Kronos() {
    delete this->citiesDatabase;
    this->citiesDatabase = 0;
}



void Kronos::onStartup() {
    std::cout << "### ON STARTUP ###" << std::endl;

    CitiesDatabase* db = new CitiesDatabase("kronos","stuproUser","weloveparaview","127.0.0.1","5432");
    this->citiesDatabase = db;
    db->printAllCities();
    //db->openDatabase();
    //db->getAllCities();


    this->initialized = true;
}

void Kronos::onShutdown() {
    delete this->citiesDatabase;
    this->citiesDatabase = 0;
    std::cout << "### ON SHUTDOWN ###" << std::endl;
}

CitiesDatabase* Kronos::getCitiesDatabase() {
    this->ensureReady();

    return this->citiesDatabase;
}

void Kronos::ensureReady() {
    if (!this->initialized) {
        throw new InvalidStateException("Kronos is not yet fully initialized");
    }
}
