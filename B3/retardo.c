#include "retardo.h"

void retardo_1us (void){
	
	uint32_t contador;
	
	for (contador =0; contador <20; contador++);

}
void retardo_1Ms (void){
	
	uint32_t contador;
	
	for (contador =0; contador <20000; contador++);

}
void retardo_1s (void){
	
	uint32_t contador;
	
	for (contador =0; contador <20000000; contador++);

}
void retardo_us (int us){
	
	uint32_t contador;
	
	for (contador =0; contador <(20*us); contador++);

}
void retardo_ms (int ms){
	
	uint32_t contador;
	
	for (contador =0; contador <(20000*ms); contador++);

}
void retardo_s (int s){
	
	uint32_t contador;
	
	for (contador =0; contador <(20000000*s); contador++);

}
