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


HANDLE hmutex;//Mutex
HANDLE Semaforo_Ponte;//Semaforo

// Estrutura dos carros
typedef enum { NORTE = 0, SUL = 1 } Sentido;
typedef struct {
    int id;//identificador do carro
    Sentido sentido;
    DWORD chegada; 
    DWORD entrada; 
    DWORD saida; 
} Carro;
//Verificador de violações das regras
void verificador(int total_na_ponte){
	if (carros_norte_ponte > 0 && carros_sul_ponte > 0) {
        total_violacoes_c1++;
        printf("[VIOLACAO C1] Sentidos opostos na ponte\n");
    }
    if (total_na_ponte > Max_Ponte) {
        total_violacoes_c2++;
        printf("[VIOLACAO C2] Capacidade maxima excedida: %ld carros\n", total_na_ponte);
    }
}

void carro_vrumvrum(Carro* carro){
	printf("O carro %d passou no sentido %d \n",carro->id,carro->sentido);
}

void ponte_entrar(Carro *c) {
   	Sleep(c->chegada);
    WaitForSingleObject(Semaforo_Ponte, INFINITE);//espera o semaforo estar disponivel e decrementa 1 caso esteja
    carro_vrumvrum(c);
    if (c->sentido == NORTE) {
        while (1) {
            WaitForSingleObject(hmutex, INFINITE);
            if (carros_sul_ponte == 0) {//verifica se ainda há carros do sentido sul circulando na ponte
                carros_norte_ponte++;
                ReleaseMutex(hmutex);
                break;
            }
            ReleaseMutex(hmutex);
        }
    }
    else { // SUL
        while (1) {
            WaitForSingleObject(hmutex, INFINITE);
            if (carros_norte_ponte == 0) {//verifica se ainda há carros do sentido norte circulando na ponte
                carros_sul_ponte++;
                ReleaseMutex(hmutex);
                break;
            }
            ReleaseMutex(hmutex);
        }
    }
    
    // Verificações de segurança (agora com contadores fieis à realidade)
    WaitForSingleObject(hmutex, INFINITE);

    DWORD total_na_ponte = carros_norte_ponte + carros_sul_ponte;
    verificador(total_na_ponte);
    ReleaseMutex(hmutex);
}



void ponte_sair(Carro *c) {
	Sleep(travessia_Global);
	
	
    WaitForSingleObject(hmutex, INFINITE);
    if (c->sentido == NORTE) {
        carros_norte_ponte--;
    } else {
        carros_sul_ponte--;
    }
    
    ReleaseMutex(hmutex);
    
    ReleaseSemaphore(Semaforo_Ponte, 1, NULL);
}


//Gera um carro e o manda para a ponte
DWORD WINAPI gerar_carro(LPVOID carro){
	Carro *c= (Carro*) carro;
    ponte_entrar(c);
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
    int index = 0;
    
    for(i = 0; i < n_norte; i++){
        carros[index].id = index;
        carros[index].sentido = NORTE;
        carros[index].chegada = rand() % (chegada_ms + 1);
        carros_threadsH[index] = CreateThread(NULL, 0, gerar_carro, &carros[index], 0, NULL);
        index++;
    }

    // Criar carros do sentido SUL (1)
    for(i = 0; i < n_sul; i++){
        carros[index].id = index;
        carros[index].sentido = SUL;
        carros[index].chegada = rand() % (chegada_ms + 1);
        carros_threadsH[index] = CreateThread(NULL, 0, gerar_carro, &carros[index], 0, NULL);
        index++;
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
