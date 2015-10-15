/*
 *  Multi2Sim
 *  Copyright (C) 2012  Rafael Ubal (ubal@ece.neu.edu)
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */


#include <arch/southern-islands/emu/wavefront.h>
#include <arch/southern-islands/timing/wavefront-pool.h>
#include <lib/esim/trace.h>
#include <lib/util/list.h>

#include "compute-unit.h"
#include "gpu.h"
#include "simd-unit.h"
#include "uop.h"
#include "cycle-interval-report.h"

#include <arch/southern-islands/emu/emu.h>

#include <lib/util/estadisticas.h>

void si_simd_complete(struct si_simd_t *simd)
{
	struct si_uop_t *uop;
	int list_entries;
	int list_index = 0;
	int i;

    struct si_work_item_t *work_item;
    int work_item_id;


	list_entries = list_count(simd->exec_buffer);

	if(list_entries == 0)
		add_simd_idle_cycle(simd->id_in_compute_unit);


	assert(list_entries <= si_gpu_simd_exec_buffer_size);

	for (i = 0; i < list_entries; i++)
	{
		uop = list_get(simd->exec_buffer, list_index);
		assert(uop);

		if (asTiming(si_gpu)->cycle < uop->execute_ready)
		{
			list_index++;
			continue;
		}

		/* Access complete, remove the uop from the queue */
		list_remove(simd->exec_buffer, uop);

		si_trace("si.end_inst id=%lld cu=%d\n", uop->id_in_compute_unit,
			uop->compute_unit->id);

		/* Statistics */
		simd->inst_count++;

        si_gpu->last_complete_cycle = asTiming(si_gpu)->cycle;

		/*add_si_macroinst(simd_u, uop);

		SI_FOREACH_WORK_ITEM_IN_WAVEFRONT(uop->wavefront, work_item_id)
    {
			work_item = uop->wavefront->work_items[work_item_id];
      if (si_wavefront_work_item_active(uop->wavefront, work_item->id_in_wavefront))
      {
				si_units unit = simd_u;
				ipc_instructions(asTiming(si_gpu)->cycle, unit);
			}
		}*/

		/* Free uop */
		si_uop_free(uop);
	}
}

void si_simd_execute(struct si_simd_t *simd)
{
	struct si_uop_t *uop;
	int list_entries;
	int list_index = 0;
	int instructions_processed = 0;
	int i;

	list_entries = list_count(simd->decode_buffer);

	/* Sanity check the decode buffer */
	assert(list_entries <= si_gpu_simd_decode_buffer_size);

	for (i = 0; i < list_entries; i++)
	{
		uop = list_get(simd->decode_buffer, list_index);
		assert(uop);

		instructions_processed++;

		/* Uop is not ready yet */
		if (asTiming(si_gpu)->cycle < uop->decode_ready)
		{
			list_index++;
			continue;
		}

		/* Stall if the width has been reached */
		if (instructions_processed > si_gpu_simd_width)
		{
			si_trace("si.inst id=%lld cu=%d wf=%d uop_id=%lld "
				"stg=\"s\"\n", uop->id_in_compute_unit,
				simd->compute_unit->id, uop->wavefront->id,
				uop->id_in_wavefront);
			list_index++;
			continue;
		}

		/* Sanity check exec buffer */
		assert(list_count(simd->exec_buffer) <=
			si_gpu_simd_exec_buffer_size);

		/* Stall if SIMD unit is full */
		if (list_count(simd->exec_buffer) ==
			si_gpu_simd_exec_buffer_size)
		{
			si_trace("si.inst id=%lld cu=%d wf=%d uop_id=%lld "
				"stg=\"s\"\n", uop->id_in_compute_unit,
				simd->compute_unit->id, uop->wavefront->id,
				uop->id_in_wavefront);
			list_index++;
			continue;
		}

		/* Includes time for pipelined read-exec-write of
		 * all subwavefronts */
		uop->execute_ready = asTiming(si_gpu)->cycle +
			si_gpu_simd_exec_latency;

		/* Transfer the uop to the outstanding execution buffer */
		list_remove(simd->decode_buffer, uop);
		list_enqueue(simd->exec_buffer, uop);

		uop->wavefront_pool_entry->ready_next_cycle = 1;

		si_trace("si.inst id=%lld cu=%d wf=%d uop_id=%lld "
			"stg=\"simd-e\"\n", uop->id_in_compute_unit,
			simd->compute_unit->id, uop->wavefront->id,
			uop->id_in_wavefront);
	}
}

void si_simd_decode(struct si_simd_t *simd)
{
	struct si_uop_t *uop;
	int instructions_processed = 0;
	int list_entries;
	int list_index = 0;
	int i;

	list_entries = list_count(simd->issue_buffer);

	/* Sanity check the issue buffer */
	assert(list_entries <= si_gpu_simd_issue_buffer_size);

	for (i = 0; i < list_entries; i++)
	{
		uop = list_get(simd->issue_buffer, list_index);
		assert(uop);

		instructions_processed++;

		/* Uop not ready yet */
		if (asTiming(si_gpu)->cycle < uop->issue_ready)
		{
			list_index++;
			continue;
		}

		/* Stall if the issue width has been reached. */
		if (instructions_processed > si_gpu_simd_width)
		{
			si_trace("si.inst id=%lld cu=%d wf=%d uop_id=%lld "
				"stg=\"s\"\n", uop->id_in_compute_unit,
				simd->compute_unit->id, uop->wavefront->id,
				uop->id_in_wavefront);
			list_index++;
			continue;
		}

		/* Sanity check the decode buffer */
		assert(list_count(simd->decode_buffer) <=
				si_gpu_simd_decode_buffer_size);

		/* Stall if the decode buffer is full. */
		if (list_count(simd->decode_buffer) ==
			si_gpu_simd_decode_buffer_size)
		{
			si_trace("si.inst id=%lld cu=%d wf=%d uop_id=%lld "
				"stg=\"s\"\n", uop->id_in_compute_unit,
				simd->compute_unit->id, uop->wavefront->id,
				uop->id_in_wavefront);
			list_index++;
			continue;
		}

		uop->decode_ready = asTiming(si_gpu)->cycle + si_gpu_simd_decode_latency;
		list_remove(simd->issue_buffer, uop);
		list_enqueue(simd->decode_buffer, uop);

		if (si_spatial_report_active)
			si_simd_alu_report_new_inst(simd->compute_unit, uop);

		si_trace("si.inst id=%lld cu=%d wf=%d uop_id=%lld "
			"stg=\"simd-d\"\n", uop->id_in_compute_unit,
			simd->compute_unit->id, uop->wavefront->id,
			uop->id_in_wavefront);
	}
}

void si_simd_run(struct si_simd_t *simd)
{
	/* SIMD stages */
	si_simd_complete(simd);
	si_simd_execute(simd);
	si_simd_decode(simd);
}
