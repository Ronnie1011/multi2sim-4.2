/*
 * coherence_controler.c
 *
 *  Created on: 18/2/2015
 *      Author: sisco
 */
#include <mem-system/module.h>
#include <mem-system/mod_stack.h>
#include <lib/mhandle/mhandle.h>

struct coherence_controller_t *cc_create()
{
	struct coherence_controller_t *cc;
	cc = xcalloc(1,sizeof(struct coherence_controller_t));
	cc->transaction_queue = list_create_with_size(32);
	return cc;
}

void cc_free(struct coherence_controller_t * cc)
{
	list_free(cc->transaction_queue);
	free(cc);
}

// desde el find and lock se tiene que pasar stack->ret
int cc_add_transaction(struct coherence_controller_t *cc, struct mod_stack_t *stack)
{
	struct mod_t *mod = stack->mod;

	list_add(cc->transaction_queue,(void *)stack);
	struct dir_lock_t *dir_lock = dir_lock_get(mod->dir, stack->fal_stack->set, stack->fal->way);

	if(dir_lock->lock)
	{
		return 0;
	}

	dir_entry_lock(mod->dir, stack->set, stack->way, EV_MOD_NMOESI_FIND_AND_LOCK, stack);
	return 1;
}

int cc_finish_transaction(struct coherence_controller_t *cc, struct mod_stack_t *stack)
{

	int index = list_index_of(cc->transaction_queue,(void *)stack);
	struct mod_stack_t *removed_stack= (struct mod_stack_t *) list_remove_at(cc->transaction_queue, index);

	cc_launch_next_transaction(cc,removed_stack);
	return 0;
}

void cc_launch_next_transaction(struct coherence_controller_t *cc, struct mod_stack_t *stack)
{
	int index = cc_search_next_transaction(cc, stack->mod, stack->set, stack->way);

	if(index >= 0)
	{

	}



}

int cc_search_next_transaction(struct coherence_controller_t *cc, struct mod_t *mod, int set, int way)
{
	struct mod_stack_t *stack_in_list;
	for(int i = 0;i < list_count(cc->transaction_queue); i++)
	{
		stack_in_list = (struct mod_stack_t *) list_get(cc->transaction_queue, i);
		if( stack_in_list->mod == mod && stack_in_list->set == set, stack_in_list->way == way)
		{
			return i;
		}
	}

	return -1;

}

