#include <stdio.h>
#include <windows.h>
#include <time.h>
#include <stdlib.h>
#include <pthread.h>
#define Max_Ponte 3
// Contadores globais
DWORD carros_norte_ponte = 0;
DWORD carros_sul_ponte = 0;
DWORD total_violacoes_c1 = 0;
DWORD total_violacoes_c2 = 0;
DWORD travessia_Global;
//Mutex
HANDLE hmutex;
HANDLE Semaforo_Ponte;

// Estrutura dos carros
typedef enum { NORTE = 0, SUL = 1 } Sentido;

typedef struct {
    int id;//identificador do carro
    Sentido sentido;
    DWORD chegada; 
    DWORD entrada; 
    DWORD saida; 
} Carro;

void ponte_entrar(Carro *c) {
    // 1. Primeiro ganha a vaga física na ponte (limite de 3 carros)
    WaitForSingleObject(Semaforo_Ponte, INFINITE);

    if (c->sentido == NORTE) {
        while (1) {
            WaitForSingleObject(hmutex, INFINITE);
            if (carros_sul_ponte == 0) {
                carros_norte_ponte++;
                ReleaseMutex(hmutex);
                break;
            }
            ReleaseMutex(hmutex);
            Sleep(5);
        }
    } 
    else { // SUL
        while (1) {
            WaitForSingleObject(hmutex, INFINITE);
            if (carros_norte_ponte == 0) {
                carros_sul_ponte++;
                ReleaseMutex(hmutex);
                break;
            }
            ReleaseMutex(hmutex);
            Sleep(5);
        }
    }
    
    // Verificações de segurança (agora com contadores fiéis à realidade)
    WaitForSingleObject(hmutex, INFINITE);
    
    if (carros_norte_ponte > 0 && carros_sul_ponte > 0) {
        total_violacoes_c1++;
        printf("[VIOLACAO C1] Sentidos opostos na ponte\n");
    }
    DWORD total_na_ponte = carros_norte_ponte + carros_sul_ponte;
    if (total_na_ponte > Max_Ponte) {
        total_violacoes_c2++;
        printf("[VIOLACAO C2] Capacidade maxima excedida: %ld carros\n", total_na_ponte);
    }
    
    ReleaseMutex(hmutex);
}

void ponte_sair(Carro *c) {
	ReleaseSemaphore(Semaforo_Ponte, 1, NULL);
    WaitForSingleObject(hmutex, INFINITE);
    
    if (c->sentido == NORTE) {
        carros_norte_ponte--;
    } else {
        carros_sul_ponte--;
    }
    ReleaseMutex(hmutex);
}

//Gera um carro e o manda para a ponte
DWORD WINAPI gerar_carro(LPVOID carro){
	Carro *c= (Carro*) carro;
	Sleep(c->chegada);
    ponte_entrar(c);
    Sleep(travessia_Global);
    ponte_sair(c);
    return 0;
}

int main(int argc, char *argv[]) {
	int i;
    int n_norte = atoi(argv[1]);
    int n_sul = atoi(argv[2]);
    int chegada_ms = atoi(argv[3]);
    int travessia_ms = atoi(argv[4]);
    int total_carros= n_norte+n_sul;
    travessia_Global= (DWORD) travessia_ms;
    
    hmutex = CreateMutex(NULL, FALSE, NULL);
    //Inicia o semaforo com numero maximo= Max_ponte e inicia em MAX_PONTE
    Semaforo_Ponte= CreateSemaphore(NULL, Max_Ponte, Max_Ponte, NULL);
    
    //Array das Threads dos carros e dos dados dos carros
    HANDLE *carros_threadsH = malloc(total_carros * sizeof(HANDLE));
    Carro *carros = malloc(total_carros * sizeof(Carro));
    srand(time(NULL));
    
    //inicia todos os arrays de acordo com os argumentos passados
    for(i=0;i<n_norte;i++){
    	carros[i].id=i;
    	carros[i].sentido= 0;
    	carros[i].chegada = rand() % (chegada_ms + 1);
    	
    	carros_threadsH[i]= CreateThread(NULL, 0, gerar_carro, &carros[i], 0, NULL);
    	
	}
	for(i=n_norte;i<n_norte+n_sul;i++){
    	carros[i].id=i;
    	carros[i].sentido= 1;
    	carros[i].chegada = rand() % (chegada_ms + 1);
    	
    	carros_threadsH[i]= CreateThread(NULL, 0, gerar_carro, &carros[i], 0, NULL);
	}
	
	
    WaitForMultipleObjects(total_carros, carros_threadsH, TRUE, INFINITE);

	//fecha os threads
    for (i = 0; i < total_carros; i++) {
        CloseHandle(carros_threadsH[i]);
    }

	//Liberta os espaços alocados na memória
    free(carros_threadsH);
    free(carros);
    printf("Simulacao terminada. Violacoes C1: %ld, Violacoes C2: %ld\n", total_violacoes_c1, total_violacoes_c2);
    return 0;
}
