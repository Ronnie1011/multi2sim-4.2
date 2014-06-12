#include "./estadisticas.h"


void estadisticas_por_intervalos(long long intervalo){

double latencia;
int z, x, y, k, i;
//long long ipc;

if((intervalo_anterior + ventana_muestreo) > intervalo )
	return;

for (k = 0; k < list_count(mem_system->mod_list); k++)
{
       struct mod_t *mod = list_get(mem_system->mod_list, k);

  fran_debug_t1000k("%d %d ",mod->access_list_count, mod->access_list_coalesced_count);
}
fran_debug_t1000k("\n");

long long latency = mem_stats.load_latency - ciclos_mem_stats_anterior.load_latency;
long long contador = mem_stats.load_latency_count - ciclos_mem_stats_anterior.load_latency_count;

//fran_debug_general("%lld %lld ",mem_stats.mod_level[1].entradas_bloqueadas,mem_stats.mod_level[2].entradas_bloqueadas);
fran_debug_general("%lld %lld ",latency, contador);

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
			 struct dir_lock_t *dir_lock = dir_lock_get(dir, x, y);

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

/*
long long *compartidos = calloc(5, sizeof(long long)), *replicas = calloc(5, sizeof(long long)), *locked = calloc(5, sizeof(long long));
obtener_stats_cache(compartidos, replicas, locked);

fran_debug_general("%lld ",*(compartidos+2));
fran_debug_general("%lld ",*(replicas+2));
fran_debug_general("%lld ",*(locked+1));
fran_debug_general("%lld ",*(locked+2));
*/
fran_debug_general("%lld %lld\n",intervalo - intervalo_anterior , intervalo);

intervalo_anterior = intervalo;

}


void obtener_stats_cache(long long *ret_locked, long long *ret_compartidos, long long  *ret_replicas){
int z, x, y, k, i;
int tag_ptr;
int state_ptr;
int comp = 0;
struct mod_t *mod;
struct cache_t *cache;
struct dir_t *dir;
long long replicas[5] = {0,0,0,0,0};
long long compartidos[5] = {0,0,0,0,0};
long long locked[5] = {0,0,0,0,0};
for (k = 0; k < list_count(mem_system->mod_list); k++)
{
        mod = list_get(mem_system->mod_list, k);

        dir = mod->dir;
        cache = mod->cache;

        for (x = 0; x < dir->xsize; x++)
        {
                for (y = 0; y < dir->ysize; y++)
                {
			struct dir_lock_t *dir_lock = dir_lock_get(dir, x, y);
			
			if(dir_lock->lock)
				ret_locked[mod->level]++;

                        cache_get_block(cache, x, y, &tag_ptr, &state_ptr);
                        if(state_ptr)
                        {
                                for (z = 0; z < dir->zsize; z++)
                                {
                                        comp = 0;
                                        for (i = 0; i < dir->num_nodes; i++)
                                        {
                                                if (dir_entry_is_sharer(dir, x, y, z, i))
                                                {
                                                        comp++;
                                                        if(comp == 1)
                                                        {
                                                                ret_compartidos[mod->level]++;
                                                        }
                                                        else
                                                        {
                                                                ret_replicas[mod->level]++;
                                                        }
                                                }
                                        }
                                }
                        }
                }
        }
}

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

//imprimir columnas
fran_debug_general("IPC Coa_L2 Lat HR_L2 Hits_L2 accesos_L2 HR_L1 Hits_L1 Accesos_L1 X X X X L1->L2_busy_in L1<-L2_busy_out invalidaciones X X X L2<-MM_busy_in L2->MM_busy_out Lat_L1-L2 Lat_L2-MM blk_comp_L2 Replicas_L1 \n");

fran_debug_ipc("Coalesce_L1 Coalesce_L2 accesos_L1 accesos_L2 efectivos_L1 efectivos_L2 MPKI_L1 MPKI_L2 HR_L1 HR_L2 Lat_L1-L2 Lat_L2-MM campo1 campo2 campo ....3 \n");

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
	(estadisticas_ipc + level)->coalesce++;
        mem_stats.mod_level[level].coalesce++;
}

void add_access(int level)
{
	(estadisticas_ipc + level)->accesses++;
        mem_stats.mod_level[level].accesses++;
}

void add_hit(int level)
{
	(estadisticas_ipc + level)->hits++;
        mem_stats.mod_level[level].hits++;
}

void add_miss(int level)
{
        (estadisticas_ipc + level)->misses++;
        mem_stats.mod_level[level].misses++;
}

long long add_si_inst(si_units unit)
{
	gpu_inst->unit[unit]++;
	gpu_inst->total++;
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

void load_finish(long long latencia, long long cantidad)
{
	gpu_stats.loads_latency += latencia * cantidad;
	gpu_stats.loads_count += cantidad;
}

void ipc_instructions(long long cycle, si_units unit)
{
	long long efectivosL1, efectivosL2;
	
	long long intervalo_instrucciones = 100000;

	if(!(add_si_inst(unit) % intervalo_instrucciones))
	{

long long locked[5] = {0,0,0,0,0}, mshr[3] = {0,0,0};

for (int k = 0; k < list_count(mem_system->mod_list); k++)
{
        struct mod_t *mod = list_get(mem_system->mod_list, k);

        struct dir_t *dir = mod->dir;
        struct cache_t *cache = mod->cache;
	
	if(mod->level == 1)
		mshr[1] += mod->mshr_count;
	if(mod->level == 2)
		mshr[2] += mod->mshr_count;

	fran_debug_ipc("%d %d ",mod->access_list_count, mod->access_list_coalesced_count);
        for (int x = 0; x < dir->xsize; x++)
        {
                for (int y = 0; y < dir->ysize; y++)
                {
                        struct dir_lock_t *dir_lock = dir_lock_get(dir, x, y);

                        if(dir_lock->lock)
                                locked[mod->level]++;
                }
        }
}


		efectivosL1 = (estadisticas_ipc + 1)->accesses - (estadisticas_ipc + 1)->coalesce;
                efectivosL2 = (estadisticas_ipc + 2)->accesses - (estadisticas_ipc + 2)->coalesce;
                fran_debug_ipc("%lld %lld ",mshr[1],mshr[2]);
		fran_debug_ipc("%lld %lld ",locked[1],locked[2]);		
		fran_debug_ipc("%lld %lld ",(estadisticas_ipc + 1)->coalesce, (estadisticas_ipc + 2)->coalesce);
		fran_debug_ipc("%lld %lld ",(estadisticas_ipc + 1)->accesses, (estadisticas_ipc + 2)->accesses);
		fran_debug_ipc("%lld %lld ",efectivosL1, efectivosL2);

		// MPKI
		fran_debug_ipc("%lld %lld ",(estadisticas_ipc + 1)->misses, (estadisticas_ipc + 2)->misses);
		
		fran_debug_ipc("%lld ", (estadisticas_ipc + 1)->hits);
                fran_debug_ipc("%lld ", (estadisticas_ipc + 2)->hits);
                fran_debug_ipc("%lld ", mem_stats.mod_level[1].coalesceHits - instrucciones_mem_stats_anterior.mod_level[1].coalesceHits);
                fran_debug_ipc("%lld ", mem_stats.mod_level[2].coalesceHits - instrucciones_mem_stats_anterior.mod_level[2].coalesceHits);

		fran_debug_ipc("%lld %lld ",(estadisticas_ipc + 1)->latencia_red_acc,(estadisticas_ipc + 1)->latencia_red_cont);
                fran_debug_ipc("%lld %lld ",(estadisticas_ipc + 2)->latencia_red_acc,(estadisticas_ipc + 2)->latencia_red_cont);

		double tiempo = (double) cycle - ipc_last_cycle;
		//fran_debug_ipc("%.2f ",   ipc_inst / (double)cycle);


           	gpu_inst->macroinst[v_mem_u] = gpu_stats.macroinst[v_mem_u] - instrucciones_gpu_stats_anterior.macroinst[v_mem_u];
                gpu_inst->macroinst[simd_u] = gpu_stats.macroinst[simd_u] - instrucciones_gpu_stats_anterior.macroinst[simd_u];
                gpu_inst->macroinst[lds_u] = gpu_stats.macroinst[lds_u] - instrucciones_gpu_stats_anterior.macroinst[lds_u];
		// latencia en gpu
                long long latency = gpu_stats.loads_latency - instrucciones_gpu_stats_anterior.loads_latency;
                long long contador = gpu_stats.loads_count - instrucciones_gpu_stats_anterior.loads_count;
		fran_debug_ipc("%lld %lld ",latency, contador);
		
		// Latencia mem system
		latency = mem_stats.load_latency - instrucciones_mem_stats_anterior.load_latency;
		contador = mem_stats.load_latency_count - instrucciones_mem_stats_anterior.load_latency_count;
                fran_debug_ipc("%lld %lld ",latency, contador);

memcpy(&instrucciones_mem_stats_anterior,&mem_stats,sizeof(struct mem_system_stats));
memcpy(&instrucciones_gpu_stats_anterior,&gpu_stats,sizeof(struct si_gpu_unit_stats));
/*
		instruciones_mem_stats_anterior.load_latency = mem_stats.load_latency;
		instruciones_mem_stats_anterior.load_latency_count = mem_stats.load_latency_count;	
		instruciones_gpu_stats_anterior.loads_latency = gpu_stats.loads_latency;
		instruciones_gpu_stats_anterior.loads_count = gpu_stats.loads_count;
		instruciones_gpu_stats_anterior.macroinst[v_mem_u] = gpu_stats.macroinst[v_mem_u];
                instruciones_gpu_stats_anterior.macroinst[simd_u] = gpu_stats.macroinst[simd_u];
                instruciones_gpu_stats_anterior.macroinst[lds_u] = gpu_stats.macroinst[lds_u];
*/
		fran_debug_ipc("%lld %lld %lld %lld %lld %lld %lld %lld %lld %lld %lld ", gpu_inst->unit[scalar_u] , gpu_inst->unit[simd_u] , gpu_inst->macroinst[simd_u], gpu_inst->unit[s_mem_u] , gpu_inst->unit[v_mem_u] , gpu_inst->macroinst[v_mem_u], gpu_inst->unit[branch_u] , gpu_inst->unit[lds_u] , gpu_inst->macroinst[lds_u], gpu_inst->total , gpu_stats.total );

		fran_debug_ipc("%lld %lld\n",cycle - ipc_last_cycle, cycle);

		ipc_inst = 0;
		ipc_last_cycle = cycle;

		
		free(estadisticas_ipc);
		free(gpu_inst);

		estadisticas_ipc = (struct esta_t *) calloc(10, sizeof(struct esta_t));
		gpu_inst = (struct si_gpu_unit_stats *) calloc(1, sizeof(struct si_gpu_unit_stats));

	}
}

