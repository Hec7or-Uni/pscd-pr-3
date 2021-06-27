//*****************************************************************
// File:   Semaphore_V4.cpp
// Author: PSCD-University of Zaragoza
// Date:   octubre 2016
// Coms:   Ver Semaphore_V4.hpp
//         La implementación se entenderá cuando veamos variables
//         condición y monitores
//*****************************************************************

//Infoŕmación útil para entender la implementación en
//http://en.cppreference.com/w/cpp/thread/condition_variable
//La estudiaremos con más detalle cuando tratemos en la asignatura la parte de "monitores"

#include <Semaphore_V4.hpp>

//-----------------------------------------------------
// Compilación condicional: ¿quiero hacer "logging"? 
// La variable "logger" se declarará en el "main"
#ifdef LOGGING_MODE
    #include <Logger.hpp>
    extern Logger _logger;
    #define ADD_EVENT(e) {_logger.addMessage(e);} //generar evento
#else
    #define ADD_EVENT(e)   //nada
#endif
//----------------------------------------------------------
Semaphore::Semaphore(const int n, const string info) {
    assert(n >= 0);           //aborta si se viola la Pre

    count = n;
    this->info = info;
}
//----------------------------------------------------------
Semaphore::~Semaphore() {
}
//----------------------------------------------------------
void Semaphore::wait() {
    unique_lock<mutex> lck(mtx);

    while(count == 0) {
        cv.wait(lck);
    }
    count--;

    	ADD_EVENT(info + "," + "WAIT," + to_string(count));
}
//----------------------------------------------------------
void Semaphore::signal() {
    unique_lock<mutex> lck(mtx);

    count++;
    cv.notify_all(); //podemos cambiar la semántica con cv.notify_one()

    	ADD_EVENT(info + "," + "SIGNAL," + to_string(count));
}
//----------------------------------------------------------
void Semaphore::signal(const int n) {
    unique_lock<mutex> lck(mtx);

    count = count+n;
    cv.notify_all();

    	ADD_EVENT(info + "," + "SIGNAL," + to_string(count)); 
}
//----------------------------------------------------------
void Semaphore::wait(const int n) {
    unique_lock<mutex> lck(mtx);

    assert(n>0);           //aborta si se viola la Pre

    while(count < n) {
        cv.wait(lck);
    }
    count = count-n;

    	ADD_EVENT(info + "," + "WAIT," + to_string(count));
}
