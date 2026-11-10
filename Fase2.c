#include <stdio.h>
#include <windows.h>
#include <time.h>
#include <stdlib.h>
#include <pthread.h>
// Contadores globais
DWORD carros_norte_ponte = 0;
DWORD carros_sul_ponte = 0;
DWORD total_violacoes_c1 = 0;
DWORD total_violacoes_c2 = 0;
DWORD travessia_Global;
//Mutex
HANDLE hMutex;


// Estrutura dos carros
typedef enum { NORTE = 0, SUL = 1 } Sentido;

typedef struct {
    int id;//identificador do carro
    Sentido sentido;
    DWORD chegada; 
    DWORD entrada; 
    DWORD saida; 
} Carro;

//Funções da ponte
void ponte_entrar(Carro *c);
void ponte_sair(Carro *c);


//Gera um carro e o manda para a ponte
DWORD WINAPI gerar_carro(LPVOID carro){
	Carro *c= (Carro*) carro;
	Sleep(c->chegada);
	WaitForSingleObject(hMutex, INFINITE);
	//região critica
    ponte_entrar(c);
    Sleep(travessia_Global);
    ponte_sair(c);
    ReleaseMutex(hMutex);
    return 0;
}


void ponte_entrar(Carro *c) {
    if (c->sentido == NORTE) {
        carros_norte_ponte++;
    } else {
        carros_sul_ponte++;
    }
    if (carros_norte_ponte > 0 && carros_sul_ponte > 0) {
        total_violacoes_c1++;
        printf("[VIOLACAO C1] Carros do Norte e Sul na ponte em simultaneo!\n");
    }
    DWORD total_na_ponte = carros_norte_ponte + carros_sul_ponte;
    if (total_na_ponte > 3) {
        total_violacoes_c2++;
        printf("[VIOLACAO C2] Capacidade maxima excedida: %ld carros na ponte!\n", total_na_ponte);
    }
}

void ponte_sair(Carro *c) {
    if (c->sentido == NORTE) {
        carros_norte_ponte--;
    } else {
        carros_sul_ponte--;
    }
}

int main(int argc, char *argv[]) {
	int i;
    int n_norte = atoi(argv[1]);
    int n_sul = atoi(argv[2]);
    int chegada_ms = atoi(argv[3]);
    int travessia_ms = atoi(argv[4]);
    int total_carros= n_norte+n_sul;
    travessia_Global= (DWORD) travessia_ms;
    hMutex = CreateMutex(NULL, FALSE, NULL);
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
    printf("Simulacao terminada. Violações C1: %ld, Violações C2: %ld\n", total_violacoes_c1, total_violacoes_c2);
    return 0;
}
