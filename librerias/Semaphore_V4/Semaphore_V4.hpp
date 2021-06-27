//*****************************************************************
// File:   Semaphore_V4.hpp
// Author: PSCD-University of Zaragoza
// Date:   octubre 2016
// Coms:   Especificación de una clase de semáforos, extendida, con operaciones
//         "wait(nat)" y "signal(nat)". También se ha extendido con la posibilidad
//         de generar información de "logging"
//         El valor transmitido en la inicialización ha de ser
//         no negativo. Si fuera negativo, da error y aborta la ejecución.
//         La semántica que implementa es la que en
//         M. Ben-Ari, "Principles of Concurrent and Distributed Programming", Addison-Wesley, 2006
//         denomina "Busy-wait" (ver pág. 120)
//         Para los casos en que el uso del semáforo va a ser exclusivamente como "mutex", Cpp
//         suministra la clase "std::mutex", cuya especificación se encuentra en 
//         http://en.cppreference.com/w/cpp/thread/mutex
//*****************************************************************

#ifndef SEMAPHORE_V4_HPP
#define SEMAPHORE_V4_HPP

#include <mutex>
#include <condition_variable>
#include <cassert>
#include <fstream>

//-----------------------------------------------------
using namespace std; //mutex, condition_variable, etc.

class Semaphore {
private:
    mutex mtx;                    //los dos primeros atributos se entenderán al estudiar los monitores
    condition_variable_any cv;
    volatile int count;                    //natural asociado al semáforo  
    //si vamos a querer trabajar con información en un log
    string info;                  //información complementaria para "logging"

public:
    //------------------------- constructores
    //Pre:  n>=0
    //Post: count=n
    //Coms: constructor con información para debugging
    //      Ejemplo: "Semaphore s(2);","Semaphore s(2, "mutexVarX");"
    //      "info" se usará cuando se generen eventos en un fichero de log.
    Semaphore(const int n, const string info = "");

    //------------------------- destructor
    //Pre:  True
    //Post: El dato ha sido destruído
    //Coms: se invoca de manera automática al cerrarse el bloque donde ha sido declarado
    ~Semaphore();

    //------------------------- operaciones estándar
    //Pre:  True
    //Post: <count++>
    //Coms: en el caso de semáforos con información de log, genera un evento en el fichero
    void signal();

    //Pre:  True
    //Post: <await count>0 
    //          count-- 
    //      >
    //Coms: en el caso de semáforos con información de log, genera un evento en el fichero
    void wait();

    //------------------------- operaciones extendidas
    //Pre:  n>0
    //Post: <count=count+n>
    //Coms: en el caso de semáforos con información de log, genera un evento en el fichero
    void signal(int n);

    //Pre:  n>0 
    //Post: <await count>=n 
    //          count = count-n
    //      >
    //Coms: en el caso de semáforos con información de log, genera un evento en el fichero
    void wait(const int n);
};

#endif 
