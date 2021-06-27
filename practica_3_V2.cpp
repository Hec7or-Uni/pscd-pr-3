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

const int MAX_SEMAPHORES = 4; // Semaforo testigo va aparte.

// constantes para el calculo del tiempo que dura un trayecto.
const int limite_inferior = 10;
const int limite_superior = 15;

//--DECLARACIÓN----------------------------------------
/* Procedimiento que desbloquea el proceso que no ha cumplido su guarda.
 * Si no consigue desbloquear ningún proceso pasa el testigo. 
 */
void AVISAR(Semaphore& testigo, Semaphore* b0, Semaphore* b1, Semaphore* b2, Semaphore* b3, int d[],
            bool& puertasEntradaAbiertas, bool& puertasSalidaAbiertas, int& w1, int& w2);

/* Proceso que simula un número de <N_VIAJES> viajes en el que se realizan varias secuencias de instrucciones
 * en exclusión mutua:
 * Primero: Espera a que lo vagones <w1> & <w2> esten llenos.  
 *          Avisa de que las puertas se cierran.
 * Segundo: Simula el viaje.
 *          Avisa de que las puertas se abren.
 * Último:  Espera a que los vagones esten vacios.
 *          Avisa de que se han bajado todos.
 */
void conductor(Semaphore& testigo, Semaphore* b0, Semaphore* b1, Semaphore* b2, Semaphore* b3, int d[],
               bool& puertasEntradaAbiertas, bool& puertasSalidaAbiertas, int& w1, int& w2);

/* Proceso que controla la subida & bajada de un usuario en uno de los vagones del tren por medio de instrucciones
 * en exclusión mutua:
 * Primero: Espera a que se cumplan las condiciones de acceso al tren: {puertas abiertas}{hueco en uno de los vagones}
 *          Avisa que se ha subido al vagón.
 * Último:  Espera a que se cumpla la condicion de salida del tren: {puertas salida abiertas}
 *          Avisa que se ha bajado del vagón.
 */
void usuario(const int id, Semaphore& testigo, Semaphore* b0, Semaphore* b1, Semaphore* b2, Semaphore* b3, int d[],
             bool& puertasEntradaAbiertas, bool& puertasSalidaAbiertas, int& w1, int& w2, int& hanBajado);

int main() {
    int w1 = 0; // personas dentro de vagon 1.
    int w2 = 0; // personas dentro de vagon 2.
    ADD_EVENT("BEGIN_MAIN,0," + to_string(w1) + "," + to_string(w2));

    int  hanBajado = 0; // +1 cada vez que un usuario se baja de su vagon.
    bool puertasEntradaAbiertas = true;  // estado de las puertas
    bool puertasSalidaAbiertas = false;

    int d[4] = {0, 0, 0, 0};
    Semaphore* testigo;
    Semaphore* b[MAX_SEMAPHORES];
    thread P[N];
    srand(time(NULL));
//--INICIO-MEM-DINÁMICA--------------------------------
    testigo = new Semaphore(1);
    b[0] = new Semaphore(0);
    b[1] = new Semaphore(0);
    b[2] = new Semaphore(0);
    b[3] = new Semaphore(0);

//--INICIO-PROCESOS------------------------------------
    thread th_1(&conductor, ref(*testigo), b[0], b[1], b[2], b[3], ref(d), ref(puertasEntradaAbiertas), ref(puertasSalidaAbiertas), ref(w1), ref(w2));  // Proceso conductor
    for (int id = 0; id < N; id++) {
        P[id] = thread(&usuario, id, ref(*testigo), b[0], b[1], b[2], b[3], ref(d), ref(puertasEntradaAbiertas), ref(puertasSalidaAbiertas), ref(w1), ref(w2), ref(hanBajado));
    }

//--FIN-PROCESOS---------------------------------------
    th_1.join();    // Esperamos a que el proceso "th_1" termine.
    for (int id = 0; id < N; id++) {
        P[id].join(); // Esperamos a que el proceso "P[id]" termine.
    }

//--BORRADO-MEM-DINÁMICA-------------------------------
    delete testigo;
    for (int i = 0; i < MAX_SEMAPHORES; i++) {
        delete b[i];
    }

//-----------------------------------------------------
    ADD_EVENT("END_MAIN,0," + to_string(w1) + "," + to_string(w2));
    return 0;
}

void AVISAR(Semaphore& testigo, Semaphore* b0, Semaphore* b1, Semaphore* b2, Semaphore* b3, int d[],
            bool& puertasEntradaAbiertas, bool& puertasSalidaAbiertas, int& w1, int& w2) {

    if ((puertasEntradaAbiertas && (w1 < N_W1 || w2 < N_W2)) && (d[0] > 0)) {
        d[0]--;
        b0->signal();
    } else if (puertasSalidaAbiertas && (d[1] > 0)) {
        d[1]--;
        b1->signal();
    } else if (((w1 == N_W1) && (w2 == N_W2)) && (d[2] > 0)) {
        d[2]--;
        b2->signal();
    } else if (((w1 == 0) && (w2 == 0)) && (d[3] > 0)) {
        d[3]--;
        b3->signal();
    } else {
        testigo.signal();
    }
}

void conductor(Semaphore& testigo, Semaphore* b0, Semaphore* b1, Semaphore* b2, Semaphore* b3, int d[],
               bool& puertasEntradaAbiertas, bool& puertasSalidaAbiertas, int& w1, int& w2) {

    for (int viaje = 1; viaje <= N_VIAJES; viaje++) {
        testigo.wait();
        if (!(w1 == N_W1 && w2 == N_W2)) {  // Si los vagones no estan llenos no salimos de la estación.
            d[2]++;
            testigo.signal();
            b2->wait();
        }
        puertasEntradaAbiertas = false; // Vagón lleno -> Cerrar puertas.
        ADD_EVENT("INICIO_VIAJE,1000," + to_string(w1) + "," + to_string(w2));
        AVISAR(testigo, b0, b1, b2, b3, d, puertasEntradaAbiertas, puertasSalidaAbiertas, w1, w2);

        // generar un numero aleatorio entre 10-15 para simular los viajes
        int tiempo_viaje = limite_inferior + rand() % (limite_superior + 1 - limite_inferior);
        this_thread::sleep_for(chrono::milliseconds(tiempo_viaje));

        testigo.wait();
        puertasSalidaAbiertas = true;   // Fin de trayecto -> Abrimos puertas de salida.
        ADD_EVENT("FIN_VIAJE,1000," + to_string(w1) + "," + to_string(w2));
        AVISAR(testigo, b0, b1, b2, b3, d, puertasEntradaAbiertas, puertasSalidaAbiertas, w1, w2);

        testigo.wait();
        if (!(w1 == 0 && w2 == 0)) {    // Si los vagones no estan vacios esperamos a que se vacien.
            d[3]++;
            testigo.signal();
            b3->signal();
        }
        puertasSalidaAbiertas = false;
        puertasEntradaAbiertas = true;
        ADD_EVENT("TODOS_HAN_BAJADO,1000," + to_string(w1) + "," + to_string(w2));
        AVISAR(testigo, b0, b1, b2, b3, d, puertasEntradaAbiertas, puertasSalidaAbiertas, w1, w2);
    }
}

void usuario(const int id, Semaphore& testigo, Semaphore* b0, Semaphore* b1, Semaphore* b2, Semaphore* b3, int d[],
             bool& puertasEntradaAbiertas, bool& puertasSalidaAbiertas, int& w1, int& w2, int& hanBajado) {

    bool wag_1;
    testigo.wait();
    if (!(puertasEntradaAbiertas && (w1 < N_W1 || w2 < N_W2))) {    // Si puertas cerradas o vagones llenos esperan al siguiente tren.
        d[0]++;
        testigo.signal();
        b0->wait();
    }
    if (w1 < N_W1) {    // Hay hueco en el vagón 1.
        wag_1 = true;
        w1++;
    } else {            // Si el vagón 1 estaba lleno me monto en el vagón 2.
        wag_1 = false;
        w2++;
    }
    ADD_EVENT("MONTA," + to_string(id) + "," + to_string(w1) + "," + to_string(w2));
    AVISAR(testigo, b0, b1, b2, b3, d, puertasEntradaAbiertas, puertasSalidaAbiertas, w1, w2);

    testigo.wait();
    if (!puertasSalidaAbiertas) {   // Si las puertas estan cerradas
        d[1]++;
        testigo.signal();
        b1->wait();
    }
    if (wag_1) {    // El usuario[i] estaba subido en el vagón 1.
        w1--;
        hanBajado++;
    } else {        // El usuario[i] estaba subido en el vagón 2.
        w2--;
        hanBajado++;
    }
    ADD_EVENT("DESMONTA," + to_string(id) + "," + to_string(w1) + "," + to_string(w2));
    AVISAR(testigo, b0, b1, b2, b3, d, puertasEntradaAbiertas, puertasSalidaAbiertas, w1, w2);
}