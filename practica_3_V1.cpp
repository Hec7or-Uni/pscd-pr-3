# include <iostream>
# include <thread>
# include <string>
# include <cstdlib>
# include <chrono>
# include <time.h>
# include <Semaphore_V4.hpp>

using namespace std;

//-----------------------------------------------------
#ifdef LOGGING_MODE
    #include <Logger.hpp>
    Logger _logger("_log_.log", 1024);
    // Logger _logger("_log_2.log", cout, 1); // si queremos echo de eventos en stdout
    #define ADD_EVENT(e) {_logger.addMessage(e);} //generar evento
#else
    #define ADD_EVENT(e)   // nada
#endif
//-----------------------------------------------------

const int N = 48;       // num de usuarios.
const int N_VIAJES = 8; // num total de viajes.
const int N_W1 = 4;     // capacidad vagon 1.
const int N_W2 = 2;     // capacidad vagon 2.
// constantes para el calculo del tiempo que dura un trayecto.
const int limite_inferior = 10;
const int limite_superior = 15;

int w1 = 0;             // personas dentro de vagon 1.
int w2 = 0;             // personas dentro de vagon 2.
int hanBajado = 0;      // +1 cada vez que un usuario se baja de su vagon.
bool puertasEntradaAbiertas = true;  // estado de las puertas
bool puertasSalidaAbiertas  = false;

Semaphore testigo(1);
Semaphore b1(0),
          b2(0),
          b3(0),
          b4(0);
int d1 = 0,
    d2 = 0,
    d3 = 0,
    d4 = 0;

//--DECLARACIÓN----------------------------------------
void AVISAR();
void conductor();
void usuario(const int id);

int main() {
    ADD_EVENT("BEGIN_MAIN,0," + to_string(w1) + "," + to_string(w2));
    thread P[N];
    srand(time(NULL));

//--INICIO-PROCESOS------------------------------------
    thread th_1(&conductor);  // Proceso conductor
    for (int id = 0; id < N; id++) {
        P[id] = thread(&usuario, id);
    }
//--FIN-PROCESOS---------------------------------------
    th_1.join();    // Esperamos a que el proceso "th_1" termine.
    for (int id = 0; id < N; id++) {
        P[id].join(); // Esperamos a que el proceso "P[id]" termine
    }
//-----------------------------------------------------
    ADD_EVENT("END_MAIN,0," + to_string(w1) + "," + to_string(w2));
    return 0;
}

//--IMPLEMENTACIÓN-------------------------------------
void AVISAR() {
    if ((puertasEntradaAbiertas && (w1 < N_W1 || w2 < N_W2)) && (d1 > 0)) {
        d1--;
        b1.signal();
    } else if (puertasSalidaAbiertas && (d2 > 0)) {
        d2--;
        b2.signal();
    } else if (((w1 == N_W1) && (w2 == N_W2)) && (d3 > 0)) {
        d3--;
        b3.signal();
    } else if (((w1 == 0) && (w2 == 0)) && (d4 > 0)) {
        d4--;
        b4.signal();
    } else {
        testigo.signal();
    }
}

void conductor() {
    for (int viaje = 1; viaje <= N_VIAJES; viaje++) {
        
        testigo.wait();
        if (!(w1 == N_W1 && w2 == N_W2)) {  // Si los vagones no estan llenos no salimos de la estación.
            d3++;
            testigo.signal();
            b3.wait();
        }
        puertasEntradaAbiertas = false; // Vagón lleno -> Cerrar puertas.
        ADD_EVENT("INICIO_VIAJE,1000," + to_string(w1) + "," + to_string(w2));
        AVISAR();

        // generar un numero aleatorio entre 10-15 para simular los viajes
        int tiempo_viaje = limite_inferior + rand() % (limite_superior + 1 - limite_inferior);
        this_thread::sleep_for(chrono::milliseconds(tiempo_viaje));

        testigo.wait();
        puertasSalidaAbiertas = true;   // Fin de trayecto -> Abrimos puertas de salida.
        ADD_EVENT("FIN_VIAJE,1000," + to_string(w1) + "," + to_string(w2));
        AVISAR();

        testigo.wait();
        if (!(w1 == 0 && w2 == 0)) {    // Si los vagones no estan vacios esperamos a que se vacien.
            d4++;
            testigo.signal();
            b4.wait();
        }
        puertasSalidaAbiertas = false;
        puertasEntradaAbiertas = true;
        ADD_EVENT("TODOS_HAN_BAJADO,1000," + to_string(w1) + "," + to_string(w2));
        AVISAR();
    }
}

void usuario(const int id) {
    bool wag_1;
    testigo.wait();
    if (!(puertasEntradaAbiertas && (w1 < N_W1 || w2 < N_W2))) {    // Si puertas cerradas o vagones llenos esperan al siguiente tren.
        d1++;
        testigo.signal();
        b1.wait();
    }
    if (w1 < N_W1) {    // Hay hueco en el vagón 1.
        wag_1 = true;
        w1++;
    } else {            // Si el vagón 1 estaba lleno me monto en el vagón 2.
        wag_1 = false;
        w2++;
    }
    ADD_EVENT("MONTA," + to_string(id) + "," + to_string(w1) + "," + to_string(w2));
    AVISAR();

    testigo.wait();
    if (!puertasSalidaAbiertas) {   // Si las puertas estan cerradas
        d2++;
        testigo.signal();
        b2.wait();
    }
    if (wag_1) {    // El usuario[i] estaba subido en el vagón 1.
        w1--;
        hanBajado++;
    } else {        // El usuario[i] estaba subido en el vagón 2.
        w2--;
        hanBajado++;
    }
    ADD_EVENT("DESMONTA," + to_string(id) + "," + to_string(w1) + "," + to_string(w2));
    AVISAR();
}