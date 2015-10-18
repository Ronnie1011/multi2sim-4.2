#include "estadisticas.h"
#include <string.h>

#include <mem-system/mshr.h>
#include <arch/southern-islands/emu/wavefront.h>
#include <arch/southern-islands/timing/gpu.h>
#include <arch/common/arch.h>
#include <arch/southern-islands/timing/compute-unit.h>
#include <lib/esim/esim.h>

int statistics_event_interval;

static long long intervalo_anterior = 0;
//static long long ipc_anterior = 0;
static long long ipc_inst = 0;
static long long ipc_last_cycle = 0;
long long ciclo = 0;
int resolucion = 0;
int EV_STATISTICS_BY_CYCLES;

int resultaFilesInitialized = 0;
bool statistics_event_paused = true;

void estadisticas_por_intervalos(long long intervalo){

//double latencia;
int z, x, y, k, i;
//long long ipc;

if((intervalo_anterior + ventana_muestreo) > intervalo )
	return;


long long latency = mem_stats.load_latency - ciclos_mem_stats_anterior.load_latency;
long long contador = mem_stats.load_latency_count - ciclos_mem_stats_anterior.load_latency_count;
/*
// mshr

mem_stats.superintervalo_latencia += latency;
mem_stats.superintervalo_contador += contador;

mem_stats.superintervalo_operacion += gpu_stats.total - instrucciones_gpu_stats_anterior.total;
mem_stats.superintervalo_ciclos += cycle - ipc_last_cycle;


if(mshr_control_enabled && !(gpu_stats.total % 500000))
{
	mshr_control(mem_stats.superintervalo_contador ? mem_stats.superintervalo_latencia/mem_stats.superintervalo_contador : 0,  mem_stats.superintervalo_operacion/mem_stats.superintervalo_ciclos);
	mem_stats.superintervalo_latencia = 0;
  	mem_stats.superintervalo_contador = 0;
     	mem_stats.superintervalo_operacion = 0;
        mem_stats.superintervalo_ciclos = 0;
}
*/


fran_debug_general("%lld %lld ",latency, contador);

fran_debug_general("%lld ",mem_stats.mod_level[0].coalesce - ciclos_mem_stats_anterior.mod_level[0].coalesce);
fran_debug_general("%lld ",mem_stats.mod_level[0].accesses - ciclos_mem_stats_anterior.mod_level[0].accesses);

fran_debug_general("%lld ",mem_stats.mod_level[1].coalesce - ciclos_mem_stats_anterior.mod_level[1].coalesce);
fran_debug_general("%lld ",mem_stats.mod_level[1].accesses - ciclos_mem_stats_anterior.mod_level[1].accesses);
fran_debug_general("%lld ",mem_stats.mod_level[1].hits - ciclos_mem_stats_anterior.mod_level[1].hits);
fran_debug_general("%lld ",mem_stats.mod_level[1].invalidations - ciclos_mem_stats_anterior.mod_level[1].invalidations);
fran_debug_general("%lld ",mem_stats.mod_level[2].coalesce - ciclos_mem_stats_anterior.mod_level[2].coalesce);
fran_debug_general("%lld ",mem_stats.mod_level[2].accesses - ciclos_mem_stats_anterior.mod_level[2].accesses);
fran_debug_general("%lld ",mem_stats.mod_level[2].hits - ciclos_mem_stats_anterior.mod_level[2].hits);
fran_debug_general("%lld ",mem_stats.mod_level[2].invalidations - ciclos_mem_stats_anterior.mod_level[2].invalidations);


fran_debug_general("%lld ",mem_stats.mod_level[1].busy_cicles_in - ciclos_mem_stats_anterior.mod_level[1].busy_cicles_in);
fran_debug_general("%lld ",mem_stats.mod_level[1].busy_cicles_out - ciclos_mem_stats_anterior.mod_level[1].busy_cicles_out);
fran_debug_general("%lld ",mem_stats.mod_level[2].busy_cicles_in - ciclos_mem_stats_anterior.mod_level[2].busy_cicles_in);
fran_debug_general("%lld ",mem_stats.mod_level[2].busy_cicles_out - ciclos_mem_stats_anterior.mod_level[2].busy_cicles_out);
fran_debug_general("%lld ",mem_stats.mod_level[1].latencia_red_acc - ciclos_mem_stats_anterior.mod_level[1].latencia_red_acc);
fran_debug_general("%lld ",mem_stats.mod_level[1].latencia_red_cont - ciclos_mem_stats_anterior.mod_level[1].latencia_red_cont);
fran_debug_general("%lld ",mem_stats.mod_level[2].latencia_red_acc - ciclos_mem_stats_anterior.mod_level[2].latencia_red_acc);
fran_debug_general("%lld ",mem_stats.mod_level[2].latencia_red_cont - ciclos_mem_stats_anterior.mod_level[2].latencia_red_cont);

memcpy(&ciclos_mem_stats_anterior,&mem_stats,sizeof(struct mem_system_stats));


int tag_ptr;
int state_ptr;
contador = 0;
long long locked[5] = {0,0,0,0,0};
struct mod_t *mod;
struct cache_t *cache;
struct dir_t *dir;
long long replica = 0;
long long compartido = 0;

for (k = 0; k < list_count(mem_system->mod_list); k++)
{
	mod = list_get(mem_system->mod_list, k);

        dir = mod->dir;
        cache = mod->cache;

        for (x = 0; x < dir->xsize; x++)
        {
	        for (y = 0; y < dir->ysize; y++)
        	{
			 struct dir_lock_t *dir_lock =  &dir->dir_lock[x * dir->ysize + y];

                        if(dir_lock->lock)
                                locked[mod->level]++;

		        if(mod->level != 2)
                		continue;


                	cache_get_block(cache, x, y, &tag_ptr, &state_ptr);
                        if(state_ptr)
                        {
                        	for (z = 0; z < dir->zsize; z++)
                                {
                                	contador = 0;
                                        for (i = 0; i < dir->num_nodes; i++)
                                        {
                                        	if (dir_entry_is_sharer(dir, x, y, z, i))
                                                {
                                                	contador++;
                                                        if(contador == 1)
                                                        {
                                                        	compartido++;
                                                        }
                                                        else
                                                        {
                                                        	replica++;
                                                        }
                                                }
                                        }
                                }
                        }
                }
        }
}

fran_debug_general("%lld ",compartido);
fran_debug_general("%lld ",replica);
fran_debug_general("%lld ",locked[1]);
fran_debug_general("%lld ",locked[2]);

fran_debug_general("%lld %lld\n",intervalo - intervalo_anterior , intervalo);

intervalo_anterior = intervalo;

}

void mem_load_finish(long long lat)
{
	mem_stats.load_latency += lat;
	mem_stats.load_latency_count++;
}

void hrl2(int hit , struct mod_t *mod, int from_load){

        if((mod->level == 2) && from_load)
        {
                mod->loads++;
                if(hit)
                {
                        estadisticas(1, 5);
                }
                else
                {
                        estadisticas(0, 5);
                }
        }
}

void estadisticas(int hit, int lvl){

        estadis[lvl].accesses++;

        estadis[lvl].hits += hit;
}

void statistics_event_init(int interval)
{
	EV_STATISTICS_BY_CYCLES = esim_register_event_with_name(statistics_event,	asTiming(si_gpu)->frequency_domain, "statistics_by_cycles");

	statistics_event_interval = interval;
	esim_schedule_event(EV_STATISTICS_BY_CYCLES, NULL, statistics_event_interval);

}

void ini_estadisticas(){
//estadis = xcalloc(10,sizeof(struct esta_t));/*

statistics_event_init(10000);


estadisticas_ipc = (struct esta_t *) calloc(10, sizeof(struct esta_t));

gpu_inst = (struct si_gpu_unit_stats *) calloc(1, sizeof(struct si_gpu_unit_stats));
mem_stats.latencias_load = (struct latenciometro *) calloc(1, sizeof(struct latenciometro));
mem_stats.latencias_nc_write = (struct latenciometro *) calloc(1, sizeof(struct latenciometro));
gpu_stats.latencias_load = (struct latenciometro *) calloc(1, sizeof(struct latenciometro));
gpu_stats.latencias_nc_write = (struct latenciometro *) calloc(1, sizeof(struct latenciometro));
//imprimir columnas
print_cache_states((long long *) NULL);

fran_debug_general("lat_loads num_loads Coalesces_gpu accesos_gpu Coalesces_L1 accesos_L1 hits_L1 invalidations_L1 Coalesces_L2 accesos_L2 hits_L2 invalidations_L2 busy_in_L1-L2 busy_out_L1-L2 busy_in_L2-MM busy_out_L2-MM lat_L1-L2 paquetes_L1-L2 lat_L2-MM paquetes_L2-MM blk_compartidos blk_replicas entradas_bloqueadas_L1 entradas_bloqueadas_L2 ciclos_intervalo ciclos_totales\n");

fran_debug_ipc("vmb_inst_counter ");

for(int i = 0; i < 10; i++)
{
	fran_debug_ipc("vmb_blocked_load_CU%d ",i);
}

for(int i = 0; i < 10; i++)
{
	fran_debug_ipc("vmb_blocked_store_CU%d ",i);
}

fran_debug_ipc("gpu_coalesce_load gpu_coalesce_store mem_coalesce_load mem_coalesce_store ");

fran_debug_ipc("evictions_L2 counter_load_action_retry cycles_load_action_retry counter_load_miss_retry cycles_load_miss_retry counter_nc_store_writeback_retry cycles_nc_store_writeback_retry counter_nc_store_action_retry cycles_nc_store_action_retry counter_nc_store_miss_retry cycles_nc_store_miss_retry accesses_with_retries invalidations ");

fran_debug_ipc("gpu_utilization dispatch_branch_instruction_infly dispatch_scalar_instruction_infly dispatch_mem_scalar_instruction_infly dispatch_simd_instruction_infly dispatch_v_mem_instruction_infly dispatch_lds_instruction_infly ");

fran_debug_ipc("cycles_simd_running dispatch_no_stall dispatch_stall_instruction_infly dispatch_stall_barrier dispatch_stall_mem_access dispatch_stall_no_wavefront dispatch_stall_others ");

fran_debug_ipc("no_stall stall_mem_access stall_barrier stall_instruction_infly stall_fetch_buffer_full stall_no_wavefront stall_others ");
fran_debug_ipc("start2fetch fetch2complete v_mem_full simd_idle1 simd_idle2 simd_idle3 simd_idle4 ");
fran_debug_ipc("queue_load lock_mshr_load lock_dir_load eviction_load retry_load miss_load finish_load access_load gpu_queue_load gpu_lock_mshr_load gpu_lock_dir_load gpu_eviction_load gpu_retry_load gpu_miss_load gpu_finish_load gpu_latencia_total_load gpu_access_load queue_nc_write lock_mshr_nc_write lock_dir_nc_write eviction_nc_write retry_nc_write miss_nc_write finish_nc_write access_nc_write mshr_size_L1 mshr_L1 mshr_L2 entradas_bloqueadas_L1 entradas_bloqueadas_L2 Coalesces_gpu Coalesces_L1 Coalesces_L2 accesos_gpu accesos_L1 accesos_L2 efectivos_L1 efectivos_L2 misses_L1 misses_L2 hits_L1 hits_L2 Cmisses_L1 Cmisses_L2 Chits_L1 Chits_L2 lat_L1-L2 paquetes_L1-L2 lat_L2-MM paquetes_L2-MM lat_loads_gpu num_loads_gpu lat_loads_mem num_loads_mem i_scalar i_simd mi_simd i_s_mem i_v_mem mi_v_mem i_branch i_lds mi_lds total_intervalo total_global ciclos_intervalo ciclos_totales esim_time\n");

        for(int i = 0; i < 10; i++){
                estadis[i].coalesce = 0;
                estadis[i].accesses = 0;
                estadis[i].hits= 0;
                estadis[i].misses = 0;
		estadis[i].busy_cicles_in = 0;
                estadis[i].busy_cicles_in = 0;
                estadis[i].invalidations = 0;
                estadis[i].delayed_read_hit= 0;
                estadis[i].esim_cycle_anterior= 0;
                estadis[i].media_latencia = 0;
                estadis[i].tiempo_acceso_latencia = 0;
                estadis[i].latencia_red_acc = 0;
                estadis[i].latencia_red_cont = 0;
                estadis[i].blk_compartidos = 0;
                estadis[i].replicas_en_l1 = 0;
		estadis[i].coalesceHits = 0;
        }
}

void add_coalesce(int level)
{
	//(estadisticas_ipc + level)->coalesce++;
        mem_stats.mod_level[level].coalesce++;
}

void add_coalesce_load(int level)
{
	//(estadisticas_ipc + level)->coalesce++;
        mem_stats.mod_level[level].coalesce_load++;
}

void add_coalesce_store(int level)
{
	//(estadisticas_ipc + level)->coalesce++;
        mem_stats.mod_level[level].coalesce_store++;
}

void add_access(int level)
{
	//(estadisticas_ipc + level)->accesses++;
        mem_stats.mod_level[level].accesses++;
}

void add_hit(int level)
{
	//(estadisticas_ipc + level)->hits++;
        mem_stats.mod_level[level].hits++;
}

void add_miss(int level)
{
        //(estadisticas_ipc + level)->misses++;
        mem_stats.mod_level[level].misses++;
}

long long add_si_inst(si_units unit)
{
	//gpu_inst->unit[unit]++;
	//gpu_inst->total++;
	gpu_stats.op_counter[unit]++;
	gpu_stats.total++;
	return gpu_stats.total;
}

long long add_si_macroinst(si_units unit, struct si_uop_t *uop)
{
	gpu_stats.macroinst[unit]++;

	if(unit == simd_u)
		add_uop_latencies(uop);

    return gpu_stats.total;
}

void add_CoalesceHit(int level)
{
	mem_stats.mod_level[level].coalesceHits++;
}

void add_CoalesceMiss(int level)
{
        mem_stats.mod_level[level].coalesceMisses++;
}


void gpu_load_finish(long long latencia, long long cantidad)
{
	gpu_stats.loads_latency += latencia * cantidad;
	gpu_stats.loads_count += cantidad;
}

void add_latencias_load(struct latenciometro *latencias)
{
	mem_stats.latencias_load->queue += latencias->queue;
	mem_stats.latencias_load->lock_mshr += latencias->lock_mshr;
	mem_stats.latencias_load->lock_dir += latencias->lock_dir;
	mem_stats.latencias_load->eviction += latencias->eviction;
	mem_stats.latencias_load->retry += latencias->retry;
	mem_stats.latencias_load->miss += latencias->miss;
	mem_stats.latencias_load->finish += latencias->finish;
	mem_stats.latencias_load->access++;
}

void copy_latencies_to_wavefront(struct latenciometro *latencias, struct si_wavefront_t *wf)
{
	long long stack_latency = latencias->queue + latencias->lock_mshr + latencias->lock_dir + latencias->eviction + latencias->retry + latencias->miss + latencias->finish;
	assert(wf != NULL);
	if(wf->latencies->total < stack_latency)
	{
		memcpy(wf->latencies,latencias,sizeof(struct latenciometro));
		wf->latencies->total = stack_latency;
	}
}

void add_wavefront_latencias_load(struct si_wavefront_t *wf)
{
	if(wf->mem_blocking)
	{
		struct latenciometro *latencias = wf->latencies;
		gpu_stats.latencias_load->queue += latencias->queue;
		gpu_stats.latencias_load->lock_mshr += latencias->lock_mshr;
		gpu_stats.latencias_load->lock_dir += latencias->lock_dir;
		gpu_stats.latencias_load->eviction += latencias->eviction;
		gpu_stats.latencias_load->retry += latencias->retry;
		gpu_stats.latencias_load->miss += latencias->miss;
		gpu_stats.latencias_load->finish += latencias->finish;
		gpu_stats.latencias_load->total += latencias->total;
		gpu_stats.latencias_load->access++;
		wf->mem_blocking = 0;
		//wf->latencies = calloc(1, sizeof(struct latenciometro));
		//free(latencias);

	}
	memset(wf->latencies,0,sizeof(struct latenciometro));
}

void add_wavefront_scalar_latencias_load(struct si_wavefront_t *wf)
{
	if(wf->scalar_mem_blocking)
	{
		struct latenciometro *latencias = wf->latencies;
		gpu_stats.latencias_load->queue += latencias->queue;
		gpu_stats.latencias_load->lock_mshr += latencias->lock_mshr;
		gpu_stats.latencias_load->lock_dir += latencias->lock_dir;
		gpu_stats.latencias_load->eviction += latencias->eviction;
		gpu_stats.latencias_load->retry += latencias->retry;
		gpu_stats.latencias_load->miss += latencias->miss;
		gpu_stats.latencias_load->finish += latencias->finish;
		gpu_stats.latencias_load->total += latencias->total;
		gpu_stats.latencias_load->access++;
		wf->scalar_mem_blocking = 0;
		//wf->latencies = calloc(1, sizeof(struct latenciometro));
		//free(latencias);

	}
	memset(wf->latencies,0,sizeof(struct latenciometro));
}


void add_latencias_nc_write(struct latenciometro *latencias)
{
	mem_stats.latencias_nc_write->queue += latencias->queue;
	mem_stats.latencias_nc_write->lock_mshr += latencias->lock_mshr;
	mem_stats.latencias_nc_write->lock_dir += latencias->lock_dir;
	mem_stats.latencias_nc_write->eviction += latencias->eviction;
	mem_stats.latencias_nc_write->retry += latencias->retry;
	mem_stats.latencias_nc_write->miss += latencias->miss;
	mem_stats.latencias_nc_write->finish += latencias->finish;
	mem_stats.latencias_nc_write->access++;
}

int ciclo_ultimaI;

void add_simd_idle_cycle(int simd_id)
{
	gpu_stats.simd_idle[simd_id]++;
}

void add_cu_mem_full()
{
	gpu_stats.v_mem_full++;
}

void add_uop_latencies(struct si_uop_t *uop)
{
	gpu_stats.start2fetch += uop->fetch_ready - uop->instruction_ready;
	gpu_stats.fetch2complete += uop->execute_ready - uop->fetch_ready;
}

void analizeTypeInstructionInFly(struct si_wavefront_t *wf)
{
			struct si_inst_t inst = wf->inst;

			/* Only evaluate branch instructions */
			if (inst.info->fmt == SI_FMT_SOPP &&
			(inst.micro_inst.sopp.op > 1 && inst.micro_inst.sopp.op < 10))
			{
				gpu_stats.dispatch_branch_instruction_infly++;
				return;
			}

			/* Only evaluate scalar instructions */
			if ((inst.info->fmt == SI_FMT_SOPP &&
			    (inst.micro_inst.sopp.op <= 1 ||
				inst.micro_inst.sopp.op >= 10)) ||
				inst.info->fmt == SI_FMT_SOP1 ||
				inst.info->fmt == SI_FMT_SOP2 ||
				inst.info->fmt == SI_FMT_SOPC ||
				inst.info->fmt == SI_FMT_SOPK)
			{

				struct si_scalar_unit_t scalar_unit = wf->wavefront_pool_entry->wavefront_pool->compute_unit->scalar_unit;
				struct si_uop_t *uop = list_get(scalar_unit.read_buffer,0);
				if(list_count(scalar_unit.inflight_mem_buffer) == si_gpu_scalar_unit_max_inflight_mem_accesses && uop && uop->scalar_mem_read )
				{
					gpu_stats.dispatch_mem_scalar_instruction_infly++;
				}else{
					gpu_stats.dispatch_scalar_instruction_infly++;
				}
				return;
			}

			if(inst.info->fmt == SI_FMT_SMRD)
			{
				gpu_stats.dispatch_mem_scalar_instruction_infly++;
				return;
			}
			/* Only evaluate SIMD instructions */
			if (inst.info->fmt == SI_FMT_VOP2 ||
				inst.info->fmt == SI_FMT_VOP1 ||
				inst.info->fmt == SI_FMT_VOPC ||
				inst.info->fmt == SI_FMT_VOP3a ||
				inst.info->fmt == SI_FMT_VOP3b)
			{
				gpu_stats.dispatch_simd_instruction_infly++;
				return;
			}

			/* Only evaluate memory instructions */
			if (inst.info->fmt == SI_FMT_MTBUF)
			{
				gpu_stats.dispatch_v_mem_instruction_infly++;
				return;
			}

			/* Only evaluate LDS instructions */
			if (inst.info->fmt == SI_FMT_DS)
			{
				gpu_stats.dispatch_lds_instruction_infly++;
				return;
			}
}

void add_eviction(int level)
{
	mem_stats.mod_level[level].evictions++;
}

void add_cache_states(int state, int level)
{
	mem_stats.mod_level[level].cache_state[state]++;
}

void add_inst_to_vmb()
{
	gpu_stats.vmb_inst_counter++;
}

void print_cache_states(long long *results)
{
	if(results == NULL && !resultaFilesInitialized)
	{
		report_cache_states("invalid noncoherent modified owned exclusive shared valid \n");
		resultaFilesInitialized = 1;
		return;
	}

	for(int i = 0; i < cache_block_state_size;i++)
	{
		report_cache_states("%lld ",*(results+i));
	}
	report_cache_states("\n");

}

void add_invalidation(int level)
{
	mem_stats.mod_level[level].invalidations++;
}

void add_retry(struct mod_stack_t *stack, retries_kinds_t retry_type)
{
	long long lat_retries_anteriores = 0;
	for(int i =0; i < num_retries_kinds; i++)
	{
		lat_retries_anteriores += stack->retries_counter[i].cycles;
	}

	stack->retries_counter[retry_type].counter++;
	stack->retries_counter[retry_type].cycles += asTiming(si_gpu)->cycle - (stack->tiempo_acceso + lat_retries_anteriores);
}


void accu_retry_time_lost(struct mod_stack_t *stack)
{
	if(stack->retry)
	{
		for(int i =0; i < num_retries_kinds; i++)
		{
			mem_stats.retries[i].counter += stack->retries_counter[i].counter;
			mem_stats.retries[i].cycles += stack->retries_counter[i].cycles;
		}
		mem_stats.stacks_with_retries++;
	}
}

void analizarCausaBloqueo(struct si_wavefront_pool_t *wavefront_pool, int active_fb)
{
	struct si_wavefront_pool_entry_t *wp_entry;
	struct si_wavefront_t *wavefront;

	for (int i = 0; i < si_gpu_max_wavefronts_per_wavefront_pool; i++)
	{
		wp_entry = wavefront_pool->entries[i];
		wavefront = wp_entry->wavefront;

		if(!wp_entry->ready && wavefront){

			if(wavefront->inst.info->fmt)
				analizeTypeInstructionInFly(wavefront);

			gpu_stats.dispatch_stall_instruction_infly++;
		}else if(wp_entry->wait_for_mem){
			gpu_stats.dispatch_stall_mem_access++;
		}else if(wp_entry->wait_for_barrier){
			gpu_stats.dispatch_stall_barrier++;
		}else if(!wp_entry->wavefront || wp_entry->wavefront_finished || wp_entry->wavefront->finished){
			gpu_stats.dispatch_stall_no_wavefront++;
		}else{
			gpu_stats.dispatch_stall_others++;
		}
	}
}

void add_simd_running_cycle()
{
	gpu_stats.cycles_simd_running++;
}

void add_cycle_vmb_blocked(int compute_unit_id, int blocked_by_store)
{
	if(blocked_by_store)
	{
	gpu_stats.cycles_vmb_blocked_store[compute_unit_id]++;
	}else{
	gpu_stats.cycles_vmb_blocked_load[compute_unit_id]++;
	}
}

/*void ipc_instructions(long long cycle, si_units unit)
{

	long long intervalo_instrucciones = 10000;

	if(!(add_si_inst(unit) % intervalo_instrucciones))
	{

long long locked[5] = {0,0,0,0,0}, mshr[3] = {0,0,0}, mshr_size[3]={0,0,0};

for (int k = 0; k < list_count(mem_system->mod_list); k++)
{
        struct mod_t *mod = list_get(mem_system->mod_list, k);

        struct dir_t *dir = mod->dir;

	if(mod->level == 1)
	{
		mshr[1] += mod->mshr->entradasOcupadas;
		mshr_size[1] = mod->mshr->size;
	}
	if(mod->level == 2)
		mshr[2] += mod->mshr->entradasOcupadas;

        for (int x = 0; x < dir->xsize; x++)
        {
                for (int y = 0; y < dir->ysize; y++)
                {
                        struct dir_lock_t *dir_lock =  &dir->dir_lock[x * dir->ysize + y];

                        if(dir_lock->lock)
                                locked[mod->level]++;
                }
        }
}

print_cache_states(mem_stats.mod_level[1].cache_state);

fran_debug_ipc("%lld ",gpu_stats.vmb_inst_counter);
gpu_stats.vmb_inst_counter = 0;

//vmb_blocks
for(int i = 0; i < 10; i++)
{
	fran_debug_ipc("%lld ",gpu_stats.cycles_vmb_blocked_load[i]);
	gpu_stats.cycles_vmb_blocked_load[i] = 0;
}

for(int i = 0; i < 10; i++)
{
	fran_debug_ipc("%lld ",gpu_stats.cycles_vmb_blocked_store[i]);
	gpu_stats.cycles_vmb_blocked_store[i] = 0;
}

fran_debug_ipc("%lld %lld %lld %lld ",mem_stats.mod_level[0].coalesce_load, mem_stats.mod_level[0].coalesce_store, mem_stats.mod_level[1].coalesce_load, mem_stats.mod_level[1].coalesce_store);
mem_stats.mod_level[0].coalesce_load = 0;
mem_stats.mod_level[0].coalesce_store = 0;
mem_stats.mod_level[1].coalesce_load = 0;
mem_stats.mod_level[1].coalesce_store = 0;
//evitions

fran_debug_ipc("%lld ",mem_stats.mod_level[2].evictions - instrucciones_mem_stats_anterior.mod_level[2].evictions);


//retries
fran_debug_ipc("%lld ",mem_stats.retries[load_action_retry].counter - instrucciones_mem_stats_anterior.retries[load_action_retry].counter);
fran_debug_ipc("%lld ",mem_stats.retries[load_action_retry].cycles - instrucciones_mem_stats_anterior.retries[load_action_retry].cycles);
fran_debug_ipc("%lld ",mem_stats.retries[load_miss_retry].counter - instrucciones_mem_stats_anterior.retries[load_miss_retry].counter);
fran_debug_ipc("%lld ",mem_stats.retries[load_miss_retry].cycles -instrucciones_mem_stats_anterior.retries[load_miss_retry].cycles );
fran_debug_ipc("%lld ",mem_stats.retries[nc_store_writeback_retry].counter - instrucciones_mem_stats_anterior.retries[nc_store_writeback_retry].counter);
fran_debug_ipc("%lld ",mem_stats.retries[nc_store_writeback_retry].cycles - instrucciones_mem_stats_anterior.retries[nc_store_writeback_retry].cycles);
fran_debug_ipc("%lld ",mem_stats.retries[nc_store_action_retry].counter - instrucciones_mem_stats_anterior.retries[nc_store_action_retry].counter);
fran_debug_ipc("%lld ",mem_stats.retries[nc_store_action_retry].cycles - instrucciones_mem_stats_anterior.retries[nc_store_action_retry].cycles);
fran_debug_ipc("%lld ",mem_stats.retries[nc_store_miss_retry].counter - instrucciones_mem_stats_anterior.retries[nc_store_miss_retry].counter);
fran_debug_ipc("%lld ",mem_stats.retries[nc_store_miss_retry].cycles - instrucciones_mem_stats_anterior.retries[nc_store_miss_retry].cycles);
fran_debug_ipc("%lld ",mem_stats.stacks_with_retries - instrucciones_mem_stats_anterior.stacks_with_retries);

fran_debug_ipc("%lld ",mem_stats.mod_level[1].invalidations - instrucciones_mem_stats_anterior.mod_level[1].invalidations);


	long long efectivosL1 = (mem_stats.mod_level[1].accesses - instrucciones_mem_stats_anterior.mod_level[1].accesses) - (mem_stats.mod_level[1].coalesce - instrucciones_mem_stats_anterior.mod_level[1].coalesce);
    long long efectivosL2 = (mem_stats.mod_level[2].accesses - instrucciones_mem_stats_anterior.mod_level[2].accesses) - (mem_stats.mod_level[2].coalesce - instrucciones_mem_stats_anterior.mod_level[2].coalesce);

	SIGpu *gpu = asSIGpu(arch_southern_islands->timing);

	int compute_unit_id = 0;
	int work_groups_running = 0;
	SI_GPU_FOREACH_COMPUTE_UNIT(compute_unit_id)
	{
		work_groups_running += gpu->compute_units[compute_unit_id]->work_group_count;
	}

    	float gpu_utilization = work_groups_running / (float)(si_gpu_num_compute_units *  gpu->compute_units[0]->num_wavefront_pools * si_gpu_max_work_groups_per_wavefront_pool);

	fran_debug_ipc("%f ",gpu_utilization);
	fran_debug_ipc("%lld ",gpu_stats.dispatch_branch_instruction_infly);
	fran_debug_ipc("%lld ",gpu_stats.dispatch_scalar_instruction_infly);
	fran_debug_ipc("%lld ",gpu_stats.dispatch_mem_scalar_instruction_infly);
	fran_debug_ipc("%lld ",gpu_stats.dispatch_simd_instruction_infly);
	fran_debug_ipc("%lld ",gpu_stats.dispatch_v_mem_instruction_infly);
	fran_debug_ipc("%lld ",gpu_stats.dispatch_lds_instruction_infly);

    	fran_debug_ipc("%lld ",gpu_stats.cycles_simd_running);
	fran_debug_ipc("%lld ",gpu_stats.dispatch_no_stall);
	fran_debug_ipc("%lld ",gpu_stats.dispatch_stall_instruction_infly);
	fran_debug_ipc("%lld ",gpu_stats.dispatch_stall_barrier);
	fran_debug_ipc("%lld ",gpu_stats.dispatch_stall_mem_access);
	fran_debug_ipc("%lld ",gpu_stats.dispatch_stall_no_wavefront);
	fran_debug_ipc("%lld ",gpu_stats.dispatch_stall_others);

	fran_debug_ipc("%lld ",instrucciones_gpu_stats_anterior.no_stall);
	fran_debug_ipc("%lld ",instrucciones_gpu_stats_anterior.stall_mem_access);
	fran_debug_ipc("%lld ",instrucciones_gpu_stats_anterior.stall_barrier);
	fran_debug_ipc("%lld ",instrucciones_gpu_stats_anterior.stall_instruction_infly);
	fran_debug_ipc("%lld ",instrucciones_gpu_stats_anterior.stall_fetch_buffer_full);
	fran_debug_ipc("%lld ",instrucciones_gpu_stats_anterior.stall_no_wavefront);
	fran_debug_ipc("%lld ",instrucciones_gpu_stats_anterior.stall_others);


	fran_debug_ipc("%lld ",gpu_stats.start2fetch - instrucciones_gpu_stats_anterior.start2fetch);
	fran_debug_ipc("%lld ",gpu_stats.fetch2complete - instrucciones_gpu_stats_anterior.fetch2complete);

	fran_debug_ipc("%lld ",gpu_stats.v_mem_full - instrucciones_gpu_stats_anterior.v_mem_full);

	fran_debug_ipc("%lld ",gpu_stats.simd_idle[0] - instrucciones_gpu_stats_anterior.simd_idle[0]);
	fran_debug_ipc("%lld ",gpu_stats.simd_idle[1] - instrucciones_gpu_stats_anterior.simd_idle[1]);
	fran_debug_ipc("%lld ",gpu_stats.simd_idle[2] - instrucciones_gpu_stats_anterior.simd_idle[2]);
	fran_debug_ipc("%lld ",gpu_stats.simd_idle[3] - instrucciones_gpu_stats_anterior.simd_idle[3]);

	fran_debug_ipc("%lld ",mem_stats.latencias_load->queue);
	fran_debug_ipc("%lld ",mem_stats.latencias_load->lock_mshr);
	fran_debug_ipc("%lld ",mem_stats.latencias_load->lock_dir);
	fran_debug_ipc("%lld ",mem_stats.latencias_load->eviction);
	fran_debug_ipc("%lld ",mem_stats.latencias_load->retry);
	fran_debug_ipc("%lld ",mem_stats.latencias_load->miss);
	fran_debug_ipc("%lld ",mem_stats.latencias_load->finish);
	fran_debug_ipc("%lld ",mem_stats.latencias_load->access);

	fran_debug_ipc("%lld ",gpu_stats.latencias_load->queue);
	fran_debug_ipc("%lld ",gpu_stats.latencias_load->lock_mshr);
	fran_debug_ipc("%lld ",gpu_stats.latencias_load->lock_dir);
	fran_debug_ipc("%lld ",gpu_stats.latencias_load->eviction);
	fran_debug_ipc("%lld ",gpu_stats.latencias_load->retry);
	fran_debug_ipc("%lld ",gpu_stats.latencias_load->miss);
	fran_debug_ipc("%lld ",gpu_stats.latencias_load->finish);
	fran_debug_ipc("%lld ",gpu_stats.latencias_load->total);
	fran_debug_ipc("%lld ",gpu_stats.latencias_load->access);

	fran_debug_ipc("%lld ",mem_stats.latencias_nc_write->queue);
	fran_debug_ipc("%lld ",mem_stats.latencias_nc_write->lock_mshr);
	fran_debug_ipc("%lld ",mem_stats.latencias_nc_write->lock_dir);
	fran_debug_ipc("%lld ",mem_stats.latencias_nc_write->eviction);
	fran_debug_ipc("%lld ",mem_stats.latencias_nc_write->retry);
	fran_debug_ipc("%lld ",mem_stats.latencias_nc_write->miss);
	fran_debug_ipc("%lld ",mem_stats.latencias_nc_write->finish);
	fran_debug_ipc("%lld ",mem_stats.latencias_nc_write->access);


	fran_debug_ipc("%lld ",mshr_size[1]);
        fran_debug_ipc("%lld %lld ",mshr[1],mshr[2]);
	fran_debug_ipc("%lld %lld ",locked[1],locked[2]);
	fran_debug_ipc("%lld ",mem_stats.mod_level[0].coalesce - instrucciones_mem_stats_anterior.mod_level[0].coalesce);
	fran_debug_ipc("%lld ",mem_stats.mod_level[1].coalesce - instrucciones_mem_stats_anterior.mod_level[1].coalesce);
	fran_debug_ipc("%lld ", mem_stats.mod_level[2].coalesce - instrucciones_mem_stats_anterior.mod_level[2].coalesce);
	fran_debug_ipc("%lld ", mem_stats.mod_level[0].accesses - instrucciones_mem_stats_anterior.mod_level[0].accesses);
	fran_debug_ipc("%lld ", mem_stats.mod_level[1].accesses - instrucciones_mem_stats_anterior.mod_level[1].accesses);
	fran_debug_ipc("%lld ", mem_stats.mod_level[2].accesses - instrucciones_mem_stats_anterior.mod_level[2].accesses);
	fran_debug_ipc("%lld %lld ",efectivosL1, efectivosL2);

		// MPKI

	fran_debug_ipc("%lld ", mem_stats.mod_level[1].misses - instrucciones_mem_stats_anterior.mod_level[1].misses);
	fran_debug_ipc("%lld ", mem_stats.mod_level[2].misses - instrucciones_mem_stats_anterior.mod_level[2].misses);


	fran_debug_ipc("%lld ", mem_stats.mod_level[1].hits - instrucciones_mem_stats_anterior.mod_level[1].hits);
        fran_debug_ipc("%lld ", mem_stats.mod_level[2].hits - instrucciones_mem_stats_anterior.mod_level[2].hits);

	fran_debug_ipc("%lld ", mem_stats.mod_level[1].coalesceMisses - instrucciones_mem_stats_anterior.mod_level[1].coalesceMisses);
	fran_debug_ipc("%lld ", mem_stats.mod_level[2].coalesceMisses - instrucciones_mem_stats_anterior.mod_level[2].coalesceMisses);
	fran_debug_ipc("%lld ", mem_stats.mod_level[1].coalesceHits - instrucciones_mem_stats_anterior.mod_level[1].coalesceHits);
	fran_debug_ipc("%lld ", mem_stats.mod_level[2].coalesceHits - instrucciones_mem_stats_anterior.mod_level[2].coalesceHits);

		fran_debug_ipc("%lld %lld ",(estadisticas_ipc + 1)->latencia_red_acc,(estadisticas_ipc + 1)->latencia_red_cont);
                fran_debug_ipc("%lld %lld ",(estadisticas_ipc + 2)->latencia_red_acc,(estadisticas_ipc + 2)->latencia_red_cont);

	gpu_inst->macroinst[v_mem_u] = gpu_stats.macroinst[v_mem_u] - instrucciones_gpu_stats_anterior.macroinst[v_mem_u];
	gpu_inst->macroinst[simd_u] = gpu_stats.macroinst[simd_u] - instrucciones_gpu_stats_anterior.macroinst[simd_u];
	gpu_inst->macroinst[lds_u] = gpu_stats.macroinst[lds_u] - instrucciones_gpu_stats_anterior.macroinst[lds_u];
		// latencia en gpu
	long long latency = gpu_stats.loads_latency - instrucciones_gpu_stats_anterior.loads_latency;
	long long contador = gpu_stats.loads_count - instrucciones_gpu_stats_anterior.loads_count;

	fran_debug_ipc("%lld %lld ",latency, contador);

	mem_stats.superintervalo_latencia += latency;
	mem_stats.superintervalo_contador += contador;

        //Latencia mem system
        latency = mem_stats.load_latency - instrucciones_mem_stats_anterior.load_latency;
        contador = mem_stats.load_latency_count - instrucciones_mem_stats_anterior.load_latency_count;
	fran_debug_ipc("%lld %lld ",latency, contador);


	fran_debug_ipc("%lld ",gpu_stats.op_counter[scalar_u] - instrucciones_gpu_stats_anterior.op_counter[scalar_u]);
        fran_debug_ipc("%lld ",gpu_stats.op_counter[simd_u] - instrucciones_gpu_stats_anterior.op_counter[simd_u]);
        fran_debug_ipc("%lld ",gpu_stats.macroinst[simd_u] - instrucciones_gpu_stats_anterior.macroinst[simd_u]);
        fran_debug_ipc("%lld ",gpu_stats.op_counter[s_mem_u] - instrucciones_gpu_stats_anterior.op_counter[s_mem_u]);
        fran_debug_ipc("%lld ",gpu_stats.op_counter[v_mem_u] - instrucciones_gpu_stats_anterior.op_counter[v_mem_u]);
        fran_debug_ipc("%lld ",gpu_stats.macroinst[v_mem_u] - instrucciones_gpu_stats_anterior.macroinst[v_mem_u]);
	fran_debug_ipc("%lld ",gpu_stats.op_counter[branch_u] - instrucciones_gpu_stats_anterior.op_counter[branch_u]);
        fran_debug_ipc("%lld ",gpu_stats.op_counter[lds_u] - instrucciones_gpu_stats_anterior.op_counter[lds_u]);
        fran_debug_ipc("%lld ",gpu_stats.macroinst[lds_u] - instrucciones_gpu_stats_anterior.macroinst[lds_u]);
	fran_debug_ipc("%lld ",gpu_stats.total - instrucciones_gpu_stats_anterior.total);
	fran_debug_ipc("%lld ",gpu_stats.total);


	fran_debug_ipc("%lld %lld ",cycle - ipc_last_cycle, cycle);

// mshr
	mem_stats.superintervalo_operacion += gpu_stats.total - instrucciones_gpu_stats_anterior.total;
	mem_stats.superintervalo_ciclos += cycle - ipc_last_cycle;

	ipc_inst = 0;
	ipc_last_cycle = cycle;

	int lat_umbral = mem_stats.superintervalo_contador ? mem_stats.superintervalo_latencia/mem_stats.superintervalo_contador : 0;
	lat_umbral *= 1.5;


		fran_debug_ipc("0");
	//}
	fran_debug_ipc("%lld",esim_time);
	fran_debug_ipc("\n");

	memcpy(&instrucciones_mem_stats_anterior,&mem_stats,sizeof(struct mem_system_stats));
	free(mem_stats.latencias_load);
	free(mem_stats.latencias_nc_write);

	mem_stats.latencias_load = (struct latenciometro *) calloc(1, sizeof(struct latenciometro));
	mem_stats.latencias_nc_write = (struct latenciometro *) calloc(1, sizeof(struct latenciometro));
	gpu_stats.latencias_load = (struct latenciometro *) calloc(1, sizeof(struct latenciometro));

	memcpy(&instrucciones_gpu_stats_anterior,&gpu_stats,sizeof(struct si_gpu_unit_stats));

	//	estadisticas_ipc = (struct esta_t *) calloc(10, sizeof(struct esta_t));
	//	gpu_inst = (struct si_gpu_unit_stats *) calloc(1, sizeof(struct si_gpu_unit_stats));

	}
}
*/

void statistics_event()
{
struct mod_t *mod;
long long cycle = asTiming(si_gpu)->cycle;

long long locked[5] = {0,0,0,0,0}, mshr[3] = {0,0,0}, mshr_size[3]={0,0,0};

for (int k = 0; k < list_count(mem_system->mod_list); k++)
{
        mod = list_get(mem_system->mod_list, k);

				if(mod->level == 1)
				{
					if(mod->access_list_count != 0 || mem_stats.latencias_load->access != 0 || mem_stats.latencias_nc_write->access != 0)
						break;
				}

				if(k == list_count(mem_system->mod_list)-1)
				{
					esim_schedule_event(EV_STATISTICS_BY_CYCLES, NULL, statistics_event_interval);
					return;
				}

			}

for (int k = 0; k < list_count(mem_system->mod_list); k++)
{
        mod = list_get(mem_system->mod_list, k);

        struct dir_t *dir = mod->dir;
//        struct cache_t *cache = mod->cache;

	if(mod->level == 1)
	{
		mshr[1] += mod->mshr->entradasOcupadas;
		mshr_size[1] = mod->mshr->size;
	}

	if(mod->level == 2)
		mshr[2] += mod->mshr->entradasOcupadas;

    for (int x = 0; x < dir->xsize; x++)
    {
      for (int y = 0; y < dir->ysize; y++)
      {
        struct dir_lock_t *dir_lock =  &dir->dir_lock[x * dir->ysize + y];
        if(dir_lock->lock)
          locked[mod->level]++;
      }
    }
}

print_cache_states(mem_stats.mod_level[1].cache_state);

fran_debug_ipc("%lld ",gpu_stats.vmb_inst_counter);
gpu_stats.vmb_inst_counter = 0;

//vmb_blocks
for(int i = 0; i < 10; i++)
{
	fran_debug_ipc("%lld ",gpu_stats.cycles_vmb_blocked_load[i]);
	gpu_stats.cycles_vmb_blocked_load[i] = 0;
}

for(int i = 0; i < 10; i++)
{
	fran_debug_ipc("%lld ",gpu_stats.cycles_vmb_blocked_store[i]);
	gpu_stats.cycles_vmb_blocked_store[i] = 0;
}

fran_debug_ipc("%lld %lld %lld %lld ",mem_stats.mod_level[0].coalesce_load, mem_stats.mod_level[0].coalesce_store, mem_stats.mod_level[1].coalesce_load, mem_stats.mod_level[1].coalesce_store);
mem_stats.mod_level[0].coalesce_load = 0;
mem_stats.mod_level[0].coalesce_store = 0;
mem_stats.mod_level[1].coalesce_load = 0;
mem_stats.mod_level[1].coalesce_store = 0;
//evitions

fran_debug_ipc("%lld ",mem_stats.mod_level[2].evictions - instrucciones_mem_stats_anterior.mod_level[2].evictions);


//retries
fran_debug_ipc("%lld ",mem_stats.retries[load_action_retry].counter - instrucciones_mem_stats_anterior.retries[load_action_retry].counter);
fran_debug_ipc("%lld ",mem_stats.retries[load_action_retry].cycles - instrucciones_mem_stats_anterior.retries[load_action_retry].cycles);
fran_debug_ipc("%lld ",mem_stats.retries[load_miss_retry].counter - instrucciones_mem_stats_anterior.retries[load_miss_retry].counter);
fran_debug_ipc("%lld ",mem_stats.retries[load_miss_retry].cycles -instrucciones_mem_stats_anterior.retries[load_miss_retry].cycles );
fran_debug_ipc("%lld ",mem_stats.retries[nc_store_writeback_retry].counter - instrucciones_mem_stats_anterior.retries[nc_store_writeback_retry].counter);
fran_debug_ipc("%lld ",mem_stats.retries[nc_store_writeback_retry].cycles - instrucciones_mem_stats_anterior.retries[nc_store_writeback_retry].cycles);
fran_debug_ipc("%lld ",mem_stats.retries[nc_store_action_retry].counter - instrucciones_mem_stats_anterior.retries[nc_store_action_retry].counter);
fran_debug_ipc("%lld ",mem_stats.retries[nc_store_action_retry].cycles - instrucciones_mem_stats_anterior.retries[nc_store_action_retry].cycles);
fran_debug_ipc("%lld ",mem_stats.retries[nc_store_miss_retry].counter - instrucciones_mem_stats_anterior.retries[nc_store_miss_retry].counter);
fran_debug_ipc("%lld ",mem_stats.retries[nc_store_miss_retry].cycles - instrucciones_mem_stats_anterior.retries[nc_store_miss_retry].cycles);
fran_debug_ipc("%lld ",mem_stats.stacks_with_retries - instrucciones_mem_stats_anterior.stacks_with_retries);

fran_debug_ipc("%lld ",mem_stats.mod_level[1].invalidations - instrucciones_mem_stats_anterior.mod_level[1].invalidations);


	long long efectivosL1 = (mem_stats.mod_level[1].accesses - instrucciones_mem_stats_anterior.mod_level[1].accesses) - (mem_stats.mod_level[1].coalesce - instrucciones_mem_stats_anterior.mod_level[1].coalesce);
    long long efectivosL2 = (mem_stats.mod_level[2].accesses - instrucciones_mem_stats_anterior.mod_level[2].accesses) - (mem_stats.mod_level[2].coalesce - instrucciones_mem_stats_anterior.mod_level[2].coalesce);

	SIGpu *gpu = asSIGpu(arch_southern_islands->timing);

	int compute_unit_id = 0;
	int work_groups_running = 0;
	SI_GPU_FOREACH_COMPUTE_UNIT(compute_unit_id)
	{
		work_groups_running += gpu->compute_units[compute_unit_id]->work_group_count;
	}

    	float gpu_utilization = work_groups_running / (float)(si_gpu_num_compute_units *  gpu->compute_units[0]->num_wavefront_pools * si_gpu_max_work_groups_per_wavefront_pool);

	fran_debug_ipc("%f ",gpu_utilization);
	fran_debug_ipc("%lld ",gpu_stats.dispatch_branch_instruction_infly);
	fran_debug_ipc("%lld ",gpu_stats.dispatch_scalar_instruction_infly);
	fran_debug_ipc("%lld ",gpu_stats.dispatch_mem_scalar_instruction_infly);
	fran_debug_ipc("%lld ",gpu_stats.dispatch_simd_instruction_infly);
	fran_debug_ipc("%lld ",gpu_stats.dispatch_v_mem_instruction_infly);
	fran_debug_ipc("%lld ",gpu_stats.dispatch_lds_instruction_infly);

    	fran_debug_ipc("%lld ",gpu_stats.cycles_simd_running);
	fran_debug_ipc("%lld ",gpu_stats.dispatch_no_stall);
	fran_debug_ipc("%lld ",gpu_stats.dispatch_stall_instruction_infly);
	fran_debug_ipc("%lld ",gpu_stats.dispatch_stall_barrier);
	fran_debug_ipc("%lld ",gpu_stats.dispatch_stall_mem_access);
	fran_debug_ipc("%lld ",gpu_stats.dispatch_stall_no_wavefront);
	fran_debug_ipc("%lld ",gpu_stats.dispatch_stall_others);

	fran_debug_ipc("%lld ",instrucciones_gpu_stats_anterior.no_stall);
	fran_debug_ipc("%lld ",instrucciones_gpu_stats_anterior.stall_mem_access);
	fran_debug_ipc("%lld ",instrucciones_gpu_stats_anterior.stall_barrier);
	fran_debug_ipc("%lld ",instrucciones_gpu_stats_anterior.stall_instruction_infly);
	fran_debug_ipc("%lld ",instrucciones_gpu_stats_anterior.stall_fetch_buffer_full);
	fran_debug_ipc("%lld ",instrucciones_gpu_stats_anterior.stall_no_wavefront);
	fran_debug_ipc("%lld ",instrucciones_gpu_stats_anterior.stall_others);


	fran_debug_ipc("%lld ",gpu_stats.start2fetch - instrucciones_gpu_stats_anterior.start2fetch);
	fran_debug_ipc("%lld ",gpu_stats.fetch2complete - instrucciones_gpu_stats_anterior.fetch2complete);

	fran_debug_ipc("%lld ",gpu_stats.v_mem_full - instrucciones_gpu_stats_anterior.v_mem_full);

	fran_debug_ipc("%lld ",gpu_stats.simd_idle[0] - instrucciones_gpu_stats_anterior.simd_idle[0]);
	fran_debug_ipc("%lld ",gpu_stats.simd_idle[1] - instrucciones_gpu_stats_anterior.simd_idle[1]);
	fran_debug_ipc("%lld ",gpu_stats.simd_idle[2] - instrucciones_gpu_stats_anterior.simd_idle[2]);
	fran_debug_ipc("%lld ",gpu_stats.simd_idle[3] - instrucciones_gpu_stats_anterior.simd_idle[3]);

	fran_debug_ipc("%lld ",mem_stats.latencias_load->queue);
	fran_debug_ipc("%lld ",mem_stats.latencias_load->lock_mshr);
	fran_debug_ipc("%lld ",mem_stats.latencias_load->lock_dir);
	fran_debug_ipc("%lld ",mem_stats.latencias_load->eviction);
	fran_debug_ipc("%lld ",mem_stats.latencias_load->retry);
	fran_debug_ipc("%lld ",mem_stats.latencias_load->miss);
	fran_debug_ipc("%lld ",mem_stats.latencias_load->finish);
	fran_debug_ipc("%lld ",mem_stats.latencias_load->access);

	fran_debug_ipc("%lld ",gpu_stats.latencias_load->queue);
	fran_debug_ipc("%lld ",gpu_stats.latencias_load->lock_mshr);
	fran_debug_ipc("%lld ",gpu_stats.latencias_load->lock_dir);
	fran_debug_ipc("%lld ",gpu_stats.latencias_load->eviction);
	fran_debug_ipc("%lld ",gpu_stats.latencias_load->retry);
	fran_debug_ipc("%lld ",gpu_stats.latencias_load->miss);
	fran_debug_ipc("%lld ",gpu_stats.latencias_load->finish);
	fran_debug_ipc("%lld ",gpu_stats.latencias_load->total);
	fran_debug_ipc("%lld ",gpu_stats.latencias_load->access);

	fran_debug_ipc("%lld ",mem_stats.latencias_nc_write->queue);
	fran_debug_ipc("%lld ",mem_stats.latencias_nc_write->lock_mshr);
	fran_debug_ipc("%lld ",mem_stats.latencias_nc_write->lock_dir);
	fran_debug_ipc("%lld ",mem_stats.latencias_nc_write->eviction);
	fran_debug_ipc("%lld ",mem_stats.latencias_nc_write->retry);
	fran_debug_ipc("%lld ",mem_stats.latencias_nc_write->miss);
	fran_debug_ipc("%lld ",mem_stats.latencias_nc_write->finish);
	fran_debug_ipc("%lld ",mem_stats.latencias_nc_write->access);


	fran_debug_ipc("%lld ",mshr_size[1]);
  fran_debug_ipc("%lld %lld ",mshr[1],mshr[2]);
	fran_debug_ipc("%lld %lld ",locked[1],locked[2]);
	fran_debug_ipc("%lld ",mem_stats.mod_level[0].coalesce - instrucciones_mem_stats_anterior.mod_level[0].coalesce);
	fran_debug_ipc("%lld ",mem_stats.mod_level[1].coalesce - instrucciones_mem_stats_anterior.mod_level[1].coalesce);
	fran_debug_ipc("%lld ", mem_stats.mod_level[2].coalesce - instrucciones_mem_stats_anterior.mod_level[2].coalesce);
	fran_debug_ipc("%lld ", mem_stats.mod_level[0].accesses - instrucciones_mem_stats_anterior.mod_level[0].accesses);
	fran_debug_ipc("%lld ", mem_stats.mod_level[1].accesses - instrucciones_mem_stats_anterior.mod_level[1].accesses);
	fran_debug_ipc("%lld ", mem_stats.mod_level[2].accesses - instrucciones_mem_stats_anterior.mod_level[2].accesses);
	fran_debug_ipc("%lld %lld ",efectivosL1, efectivosL2);

		// MPKI
		//fran_debug_ipc("%lld %lld ",(estadisticas_ipc + 1)->misses, (estadisticas_ipc + 2)->misses);
	fran_debug_ipc("%lld ", mem_stats.mod_level[1].misses - instrucciones_mem_stats_anterior.mod_level[1].misses);
	fran_debug_ipc("%lld ", mem_stats.mod_level[2].misses - instrucciones_mem_stats_anterior.mod_level[2].misses);


	fran_debug_ipc("%lld ", mem_stats.mod_level[1].hits - instrucciones_mem_stats_anterior.mod_level[1].hits);
        fran_debug_ipc("%lld ", mem_stats.mod_level[2].hits - instrucciones_mem_stats_anterior.mod_level[2].hits);

	fran_debug_ipc("%lld ", mem_stats.mod_level[1].coalesceMisses - instrucciones_mem_stats_anterior.mod_level[1].coalesceMisses);
	fran_debug_ipc("%lld ", mem_stats.mod_level[2].coalesceMisses - instrucciones_mem_stats_anterior.mod_level[2].coalesceMisses);
	fran_debug_ipc("%lld ", mem_stats.mod_level[1].coalesceHits - instrucciones_mem_stats_anterior.mod_level[1].coalesceHits);
	fran_debug_ipc("%lld ", mem_stats.mod_level[2].coalesceHits - instrucciones_mem_stats_anterior.mod_level[2].coalesceHits);

		fran_debug_ipc("%lld %lld ",(estadisticas_ipc + 1)->latencia_red_acc,(estadisticas_ipc + 1)->latencia_red_cont);
                fran_debug_ipc("%lld %lld ",(estadisticas_ipc + 2)->latencia_red_acc,(estadisticas_ipc + 2)->latencia_red_cont);

	gpu_inst->macroinst[v_mem_u] = gpu_stats.macroinst[v_mem_u] - instrucciones_gpu_stats_anterior.macroinst[v_mem_u];
	gpu_inst->macroinst[simd_u] = gpu_stats.macroinst[simd_u] - instrucciones_gpu_stats_anterior.macroinst[simd_u];
	gpu_inst->macroinst[lds_u] = gpu_stats.macroinst[lds_u] - instrucciones_gpu_stats_anterior.macroinst[lds_u];
		// latencia en gpu
	long long latency = gpu_stats.loads_latency - instrucciones_gpu_stats_anterior.loads_latency;
	long long contador = gpu_stats.loads_count - instrucciones_gpu_stats_anterior.loads_count;

	fran_debug_ipc("%lld %lld ",latency, contador);

	mem_stats.superintervalo_latencia += latency;
	mem_stats.superintervalo_contador += contador;

        //Latencia mem system
        latency = mem_stats.load_latency - instrucciones_mem_stats_anterior.load_latency;
        contador = mem_stats.load_latency_count - instrucciones_mem_stats_anterior.load_latency_count;
	fran_debug_ipc("%lld %lld ",latency, contador);
/*
		fran_debug_ipc("%lld %lld %lld %lld %lld %lld %lld %lld %lld %lld %lld ", gpu_inst->unit[scalar_u] , gpu_inst->unit[simd_u] , gpu_inst->macroinst[simd_u], gpu_inst->unit[s_mem_u] , gpu_inst->unit[v_mem_u] , gpu_inst->macroinst[v_mem_u], gpu_inst->unit[branch_u] , gpu_inst->unit[lds_u] , gpu_inst->macroinst[lds_u], gpu_inst->total , gpu_stats.total );
*/

/*
// workgroups counters
fran_debug_ipc("%lld ",gpu_stats.total); //finished
fran_debug_ipc("%lld ",gpu_stats.total); //started
fran_debug_ipc("%lld ",gpu_stats.total); //active
*/

	fran_debug_ipc("%lld ",gpu_stats.op_counter[scalar_u] - instrucciones_gpu_stats_anterior.op_counter[scalar_u]);
        fran_debug_ipc("%lld ",gpu_stats.op_counter[simd_u] - instrucciones_gpu_stats_anterior.op_counter[simd_u]);
        fran_debug_ipc("%lld ",gpu_stats.macroinst[simd_u] - instrucciones_gpu_stats_anterior.macroinst[simd_u]);
        fran_debug_ipc("%lld ",gpu_stats.op_counter[s_mem_u] - instrucciones_gpu_stats_anterior.op_counter[s_mem_u]);
        fran_debug_ipc("%lld ",gpu_stats.op_counter[v_mem_u] - instrucciones_gpu_stats_anterior.op_counter[v_mem_u]);
        fran_debug_ipc("%lld ",gpu_stats.macroinst[v_mem_u] - instrucciones_gpu_stats_anterior.macroinst[v_mem_u]);
	fran_debug_ipc("%lld ",gpu_stats.op_counter[branch_u] - instrucciones_gpu_stats_anterior.op_counter[branch_u]);
        fran_debug_ipc("%lld ",gpu_stats.op_counter[lds_u] - instrucciones_gpu_stats_anterior.op_counter[lds_u]);
        fran_debug_ipc("%lld ",gpu_stats.macroinst[lds_u] - instrucciones_gpu_stats_anterior.macroinst[lds_u]);
	fran_debug_ipc("%lld ",gpu_stats.total - instrucciones_gpu_stats_anterior.total);
	fran_debug_ipc("%lld ",gpu_stats.total);


	fran_debug_ipc("%lld %lld ",cycle - ipc_last_cycle, cycle);

// mshr
	mem_stats.superintervalo_operacion += gpu_stats.total - instrucciones_gpu_stats_anterior.total;
	mem_stats.superintervalo_ciclos += cycle - ipc_last_cycle;

	ipc_inst = 0;
	ipc_last_cycle = cycle;

	int lat_umbral = mem_stats.superintervalo_contador ? mem_stats.superintervalo_latencia/mem_stats.superintervalo_contador : 0;
	lat_umbral *= 1.5;

	/* FIXME DEPRECATED FOR MNSHR_EVENT */
	/*if(flag_mshr_dynamic_enabled)
  {
		ciclo_ultimaI = cycle;
    mshr_control(mem_stats.superintervalo_contador ? mem_stats.superintervalo_latencia/mem_stats.superintervalo_contador : 0,  mem_stats.superintervalo_operacion/mem_stats.superintervalo_ciclos);
    mem_stats.superintervalo_latencia = 0;
    mem_stats.superintervalo_contador = 0;
	  mem_stats.superintervalo_operacion = 0;
		mem_stats.superintervalo_ciclos = 0;
	}*/

	fran_debug_ipc("%lld",esim_time);
	fran_debug_ipc("\n");

	memcpy(&instrucciones_mem_stats_anterior,&mem_stats,sizeof(struct mem_system_stats));
	free(mem_stats.latencias_load);
	free(mem_stats.latencias_nc_write);

	mem_stats.latencias_load = (struct latenciometro *) calloc(1, sizeof(struct latenciometro));
	mem_stats.latencias_nc_write = (struct latenciometro *) calloc(1, sizeof(struct latenciometro));
	gpu_stats.latencias_load = (struct latenciometro *) calloc(1, sizeof(struct latenciometro));

	memcpy(&instrucciones_gpu_stats_anterior,&gpu_stats,sizeof(struct si_gpu_unit_stats));

	//	estadisticas_ipc = (struct esta_t *) calloc(10, sizeof(struct esta_t));
	//	gpu_inst = (struct si_gpu_unit_stats *) calloc(1, sizeof(struct si_gpu_unit_stats));

	esim_schedule_event(EV_STATISTICS_BY_CYCLES, NULL, statistics_event_interval);


}
