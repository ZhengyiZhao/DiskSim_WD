
#ifndef _DISKSIM_CACHESSD_PARAM_H
#define _DISKSIM_CACHESSD_PARAM_H  

#include <libparam/libparam.h>
#ifdef __cplusplus
extern"C"{
#endif
struct dm_disk_if;

/* prototype for disksim_cachessd param loader function */
struct cache_if *disksim_cachessd_loadparams(struct lp_block *b);

typedef enum {
   DISKSIM_CACHESSD_CACHE_SIZE,
   DISKSIM_CACHESSD_CACHE_MAP_SIZE,
   DISKSIM_CACHESSD_CACHE_BLK_SIZE,
   DISKSIM_CACHESSD_MAX_REQUEST_SIZE,
   DISKSIM_CACHESSD_FLUSH_PERIOD,
   DISKSIM_CACHESSD_FLUSH_IDLE_DELAY
} disksim_cachessd_param_t;

#define DISKSIM_CACHESSD_MAX_PARAM		DISKSIM_CACHESSD_FLUSH_IDLE_DELAY
extern void * DISKSIM_CACHESSD_loaders[];
extern lp_paramdep_t DISKSIM_CACHESSD_deps[];


static struct lp_varspec disksim_cachessd_params [] = {
   {"Cache size", I, 1 },
   {"Cache map size", I, 1 },
   {"Cache blk size", I, 1 },
   {"Max request size", I, 1 },
   {"Flush period", D, 1 },
   {"Flush idle delay", D, 1 },
   {0,0,0}
};
#define DISKSIM_CACHESSD_MAX 6
static struct lp_mod disksim_cachessd_mod = { "disksim_cachessd", disksim_cachessd_params, DISKSIM_CACHESSD_MAX, (lp_modloader_t)disksim_cachessd_loadparams,  0, 0, DISKSIM_CACHESSD_loaders, DISKSIM_CACHESSD_deps };


#ifdef __cplusplus
}
#endif
#endif // _DISKSIM_CACHESSD_PARAM_H
