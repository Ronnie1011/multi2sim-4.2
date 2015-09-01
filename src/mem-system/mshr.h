

#ifndef MEM_SYSTEM_MSHR_H
#define MEM_SYSTEM_MSHR_H

#include "mod-stack.h"
#include <stdio.h>

extern int flag_mshr_dynamic_enabled;
extern int EV_MSHR_DYNAMIC_SIZE_EVAL;

struct mshr_t
{
	struct mod_t *mod;
	int size;
	int entradasOcupadas;
	struct list_t *waiting_list;
	int size_anterior;
	int ipc_anterior;
	int latencia_anterior;

	//test values
	int testing;
	long long cycle;
	long long oper_count;
};

void mshr_init(struct mshr_t *mshr, int size);
int mshr_lock(struct mshr_t *mshr, struct mod_stack_t *stack);
void mshr_enqueue(struct mshr_t *mshr, struct mod_stack_t *stack, int event);
void mshr_unlock(struct mod_t *mod);
//void mshr_unlock(struct mshr_t *mshr);
//void mshr_unlock2(struct mod_t *mod);
struct mshr_t *mshr_create();
void mshr_free(struct mshr_t *mshr);
void mshr_control(int latencia, int opc);
void mshr_test_sizes();
int mshr_evaluar_test();
#endif
