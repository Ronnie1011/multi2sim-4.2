#include "estadisticas.h"
#include <string.h>
#include <mem-system/cache.h>
#include <mem-system/mshr.h>

static long long intervalo_anterior = 0;
//static long long ipc_anterior = 0;
static long long ipc_inst = 0;
static long long ipc_last_cycle = 0;
long long ciclo = 0;
int resolucion = 0;

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


void ini_estadisticas(){
//estadis = xcalloc(10,sizeof(struct esta_t));/*  
estadisticas_ipc = (struct esta_t *) calloc(10, sizeof(struct esta_t));

gpu_inst = (struct si_gpu_unit_stats *) calloc(1, sizeof(struct si_gpu_unit_stats));
mem_stats.latencias_load = (struct latenciometro *) calloc(1, sizeof(struct latenciometro));
mem_stats.latencias_nc_write = (struct latenciometro *) calloc(1, sizeof(struct latenciometro));

//imprimir columnas
fran_debug_general("lat_loads num_loads Coalesces_gpu accesos_gpu Coalesces_L1 accesos_L1 hits_L1 invalidations_L1 Coalesces_L2 accesos_L2 hits_L2 invalidations_L2 busy_in_L1-L2 busy_out_L1-L2 busy_in_L2-MM busy_out_L2-MM lat_L1-L2 paquetes_L1-L2 lat_L2-MM paquetes_L2-MM blk_compartidos blk_replicas entradas_bloqueadas_L1 entradas_bloqueadas_L2 ciclos_intervalo ciclos_totales\n");

fran_debug_ipc("lock_mshr_load evicted_dir_load finish_load access_load lock_mshr_nc_write evicted_dir_nc_write finish_nc_write access_nc_write mshr_size_L1 mshr_L1 mshr_L2 entradas_bloqueadas_L1 entradas_bloqueadas_L2 Coalesces_gpu Coalesces_L1 Coalesces_L2 accesos_gpu accesos_L1 accesos_L2 efectivos_L1 efectivos_L2 misses_L1 misses_L2 hits_L1 hits_L2 Cmisses_L1 Cmisses_L2 Chits_L1 Chits_L2 lat_L1-L2 paquetes_L1-L2 lat_L2-MM paquetes_L2-MM lat_loads_gpu num_loads_gpu lat_loads_mem num_loads_mem i_scalar i_simd mi_simd i_s_mem i_v_mem mi_v_mem i_branch i_lds mi_lds total_intervalo total_global ciclos_intervalo ciclos_totales latencia_mshr\n");

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
	gpu_stats.unit[unit]++;
	gpu_stats.total++;
	return gpu_stats.total;
}

long long add_si_macroinst(si_units unit)
{
	gpu_stats.macroinst[unit]++;
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
	mem_stats.latencias_load->lock_mshr += latencias->lock_mshr - latencias->start;
	mem_stats.latencias_load->evicted_dir += latencias->evicted_dir - latencias->lock_mshr;
	mem_stats.latencias_load->finish += latencias->finish - latencias->evicted_dir;
	mem_stats.latencias_load->access++;
}

void add_latencias_nc_write(struct latenciometro *latencias)
{
	mem_stats.latencias_nc_write->lock_mshr += latencias->lock_mshr - latencias->start;
	mem_stats.latencias_nc_write->evicted_dir += latencias->evicted_dir - latencias->lock_mshr;
	mem_stats.latencias_nc_write->finish += latencias->finish - latencias->evicted_dir;
	mem_stats.latencias_nc_write->access++;
}

int ciclo_ultimaI;

void ipc_instructions(long long cycle, si_units unit)
{
	
	long long intervalo_instrucciones = 10000;

	if(!(add_si_inst(unit) % intervalo_instrucciones))
	{

long long locked[5] = {0,0,0,0,0}, mshr[3] = {0,0,0}, mshr_size[3]={0,0,0};

for (int k = 0; k < list_count(mem_system->mod_list); k++)
{
        struct mod_t *mod = list_get(mem_system->mod_list, k);

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


	long long efectivosL1 = (mem_stats.mod_level[1].accesses - instrucciones_mem_stats_anterior.mod_level[1].accesses) - (mem_stats.mod_level[1].coalesce - instrucciones_mem_stats_anterior.mod_level[1].coalesce);
        long long efectivosL2 = (mem_stats.mod_level[2].accesses - instrucciones_mem_stats_anterior.mod_level[2].accesses) - (mem_stats.mod_level[2].coalesce - instrucciones_mem_stats_anterior.mod_level[2].coalesce);

	fran_debug_ipc("%lld ",mem_stats.latencias_load->lock_mshr);
	fran_debug_ipc("%lld ",mem_stats.latencias_load->evicted_dir);
	fran_debug_ipc("%lld ",mem_stats.latencias_load->finish);
	fran_debug_ipc("%lld ",mem_stats.latencias_load->access);

	fran_debug_ipc("%lld ",mem_stats.latencias_nc_write->lock_mshr);
	fran_debug_ipc("%lld ",mem_stats.latencias_nc_write->evicted_dir);
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
	fran_debug_ipc("%lld ",gpu_stats.unit[scalar_u] - instrucciones_gpu_stats_anterior.unit[scalar_u]);
        fran_debug_ipc("%lld ",gpu_stats.unit[simd_u] - instrucciones_gpu_stats_anterior.unit[simd_u]);
        fran_debug_ipc("%lld ",gpu_stats.macroinst[simd_u] - instrucciones_gpu_stats_anterior.macroinst[simd_u]);
        fran_debug_ipc("%lld ",gpu_stats.unit[s_mem_u] - instrucciones_gpu_stats_anterior.unit[s_mem_u]);
        fran_debug_ipc("%lld ",gpu_stats.unit[v_mem_u] - instrucciones_gpu_stats_anterior.unit[v_mem_u]);
        fran_debug_ipc("%lld ",gpu_stats.macroinst[v_mem_u] - instrucciones_gpu_stats_anterior.macroinst[v_mem_u]);
	fran_debug_ipc("%lld ",gpu_stats.unit[branch_u] - instrucciones_gpu_stats_anterior.unit[branch_u]);
        fran_debug_ipc("%lld ",gpu_stats.unit[lds_u] - instrucciones_gpu_stats_anterior.unit[lds_u]);
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
        if(mhsr_control_enabled && !(gpu_stats.total % 500000) /*&& (ciclo_ultimaI + lat_umbral) < cycle */)
        {
		ciclo_ultimaI = cycle;
                mshr_control(mem_stats.superintervalo_contador ? mem_stats.superintervalo_latencia/mem_stats.superintervalo_contador : 0,  mem_stats.superintervalo_operacion/mem_stats.superintervalo_ciclos);
                mem_stats.superintervalo_latencia = 0;
                mem_stats.superintervalo_contador = 0;
	        mem_stats.superintervalo_operacion = 0;
		mem_stats.superintervalo_ciclos = 0;
		fran_debug_ipc("%lld",lat_umbral);
        }else{
		fran_debug_ipc("0");
	}
	fran_debug_ipc("\n");
		
	//	free(estadisticas_ipc);
	//	free(gpu_inst);
	memcpy(&instrucciones_mem_stats_anterior,&mem_stats,sizeof(struct mem_system_stats));
	free(mem_stats.latencias_load);
	free(mem_stats.latencias_nc_write);

	mem_stats.latencias_load = (struct latenciometro *) calloc(1, sizeof(struct latenciometro));
	mem_stats.latencias_nc_write = (struct latenciometro *) calloc(1, sizeof(struct latenciometro));

	memcpy(&instrucciones_gpu_stats_anterior,&gpu_stats,sizeof(struct si_gpu_unit_stats));

	//	estadisticas_ipc = (struct esta_t *) calloc(10, sizeof(struct esta_t));
	//	gpu_inst = (struct si_gpu_unit_stats *) calloc(1, sizeof(struct si_gpu_unit_stats));

	}
}

