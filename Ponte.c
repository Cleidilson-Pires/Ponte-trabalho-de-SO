#include <stdio.h>
#include <windows>
#include <time.h>
DWORD carros_ponte=0;

typedef enum { NORTE = 0, SUL = 1 } Sentido;

typedef struct {
	int id;
	Sentido sentido;
	DWORD chegada; /* instante de chegada, em ms */
	DWORD entrada; /* preenchido por ponte_entrar */
	DWORD saida; /* preenchido por ponte_sair */
} Carro;

Carro criar_carro(){
	Carro carro= malloc(sizeof(Carro));

}
void ponte_entrar(Carro *c){
	carros_ponte++;
} /* bloqueia até ser seguro entrar */
void ponte_sair(Carro *c){
	
	carros_ponte--;
}

void main(args[]){
	int i;
	int N_carros= N_carrosSul + N_carrosNorte;
	Carro carros_sul[N_carrosSul];
	Carro carros_norte[N_carrosNorte];
	
	while(N_carros){
		while(carros_ponte<5){
			pode_entrar();
		}
	}
	return;
}
