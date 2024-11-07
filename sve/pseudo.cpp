#include <iostream>
using namespace std;

// Definišemo stanja kao enum
enum State {
    IDLE, StartLoop, InnerLoop, ComputeRPos1, ComputeRPos2, ComputeRPos3,
    ComputeRPos4, ComputeRPos5, SetRXandCX, BoundaryCheck, ComputePosition,
    ProcessSample, ComputeDerivatives, WaitForData1, WaitForData2, FetchDXX1_1,
    FetchDXX1_2, ComputeDXX1, Finish, NextSample, IncrementI
};

// Globalne promenljive
int counter = 0;         // Brojač za upravljanje mikro-operacijama
int i_reg = 0, j_reg = 0; // Registarske promenljive za iteraciju
int iradius = 5;         // Primer vrednosti za radijus
bool start_i = false;    // Signal za pokretanje FSM-a
State state = IDLE;      // Početno stanje je IDLE
bool done = false;       // Signal za završetak FSM-a

// Funkcija za upravljanje FSM logikom
void FSM() {
    switch (state) {
        case IDLE:
            if (start_i) {
                // Reset brojača i prelazak u sledeće stanje
                counter = 0;
                state = StartLoop;
                cout << "State: IDLE -> StartLoop" << endl;
            }
            break;
            
        case StartLoop:
            if (counter == 3) {
                // Prelaz u sledeće stanje
                counter = 0;
                j_reg = 0; // Resetujemo j_reg
                state = InnerLoop;
                cout << "State: StartLoop -> InnerLoop" << endl;
            } else {
                // Inkrementiraj brojač
                counter++;
                cout << "StartLoop counter: " << counter << endl;
            }
            break;

        case InnerLoop:
            if (counter == 3) {
                // Prelaz u sledeće stanje
                counter = 0;
                state = ComputeRPos1;
                cout << "State: InnerLoop -> ComputeRPos1" << endl;
            } else {
                // Inkrementiraj brojač
                counter++;
                cout << "InnerLoop counter: " << counter << endl;
            }
            break;
            
        case ComputeRPos1:
            if (counter == 3) {
                counter = 0;
                state = ComputeRPos2;
                cout << "State: ComputeRPos1 -> ComputeRPos2" << endl;
            } else {
                counter++;
                cout << "ComputeRPos1 counter: " << counter << endl;
            }
            break;

        case ComputeRPos2:
            if (counter == 3) {
                counter = 0;
                state = ComputeRPos3;
                cout << "State: ComputeRPos2 -> ComputeRPos3" << endl;
            } else {
                counter++;
                cout << "ComputeRPos2 counter: " << counter << endl;
            }
            break;

        case ComputeRPos3:
            if (counter == 3) {
                counter = 0;
                state = ComputeRPos4;
                cout << "State: ComputeRPos3 -> ComputeRPos4" << endl;
            } else {
                counter++;
                cout << "ComputeRPos3 counter: " << counter << endl;
            }
            break;

        case ComputeRPos4:
            if (counter == 3) {
                counter = 0;
                state = ComputeRPos5;
                cout << "State: ComputeRPos4 -> ComputeRPos5" << endl;
            } else {
                counter++;
                cout << "ComputeRPos4 counter: " << counter << endl;
            }
            break;

        case ComputeRPos5:
            if (counter == 3) {
                counter = 0;
                state = SetRXandCX;
                cout << "State: ComputeRPos5 -> SetRXandCX" << endl;
            } else {
                counter++;
                cout << "ComputeRPos5 counter: " << counter << endl;
            }
            break;

        case SetRXandCX:
            if (counter == 3) {
                counter = 0;
                state = BoundaryCheck;
                cout << "State: SetRXandCX -> BoundaryCheck" << endl;
            } else {
                counter++;
                cout << "SetRXandCX counter: " << counter << endl;
            }
            break;

        case BoundaryCheck:
            // Proveravamo granice (primer logike za granice)
            if (/*rx > -1.0 && rx < IndexSize && cx > -1.0 && cx < IndexSize*/) {
                state = ComputePosition;
                cout << "State: BoundaryCheck -> ComputePosition" << endl;
            } else {
                state = NextSample;
                cout << "State: BoundaryCheck -> NextSample" << endl;
            }
            break;

        case ComputePosition:
            // Proveravamo da li smo u granicama slike
            if (/*provera uslova za r i c*/) {
                state = ProcessSample;
                cout << "State: ComputePosition -> ProcessSample" << endl;
            } else {
                state = NextSample;
                cout << "State: ComputePosition -> NextSample" << endl;
            }
            break;

        case ProcessSample:
            // Procesiraj uzorak i prebaci u sledeće stanje
            state = ComputeDerivatives;
            cout << "State: ProcessSample -> ComputeDerivatives" << endl;
            break;

        case ComputeDerivatives:
            // Računanje derivata i čekanje podataka
            state = WaitForData1;
            cout << "State: ComputeDerivatives -> WaitForData1" << endl;
            break;

        case WaitForData1:
            state = FetchDXX1_1;
            cout << "State: WaitForData1 -> FetchDXX1_1" << endl;
            break;

        // Slična logika za ostala stanja kao što su WaitForData2, ComputeDXX1 itd.

        case NextSample:
            // Povećavamo `j_reg` i proveravamo da li prelazimo u sledeću iteraciju `i_reg`
            j_reg++;
            if (j_reg >= 2 * iradius) {
                state = IncrementI;
                cout << "State: NextSample -> IncrementI" << endl;
            } else {
                state = InnerLoop;
                cout << "State: NextSample -> InnerLoop" << endl;
            }
            break;

        case IncrementI:
            // Povećavamo `i_reg` i proveravamo da li smo završili
            i_reg++;
            if (i_reg >= 2 * iradius) {
                state = Finish;
                cout << "State: IncrementI -> Finish" << endl;
            } else {
                state = StartLoop;
                cout << "State: IncrementI -> StartLoop" << endl;
            }
            break;

        case Finish:
            // Završavamo proces
            done = true;
            cout << "State: Finish -> IDLE (done)" << endl;
            state = IDLE;
            break;

        default:
            state = IDLE;
            break;
    }
}

int main() {
    // Postavi signal za start
    start_i = true;

    // Simuliraj FSM petlju
    while (!done) {
        FSM();  // Pozivaj FSM funkciju u petlji dok se ne završi
    }

    return 0;
}
