
#ifndef _DISKSIM_CACHEMBC_PARAM_H
#define _DISKSIM_CACHEMBC_PARAM_H  

#include <libparam/libparam.h>
#ifdef __cplusplus
extern"C"{
#endif
struct dm_disk_if;

/* prototype for disksim_cachembc param loader function */
struct cache_if *disksim_cachembc_loadparams(struct lp_block *b);

typedef enum {
   DISKSIM_CACHEMBC_CACHE_DEVICE,
   DISKSIM_CACHEMBC_MAX_REQUEST_SIZE,
   DISKSIM_CACHEMBC_WRITE_SCHEME,
   DISKSIM_CACHEMBC_FLUSH_IDLE_DELAY,
   DISKSIM_CACHEMBC_MBC_LOCATIONS
} disksim_cachembc_param_t;

#define DISKSIM_CACHEMBC_MAX_PARAM		DISKSIM_CACHEMBC_MBC_LOCATIONS
extern void * DISKSIM_CACHEMBC_loaders[];
extern lp_paramdep_t DISKSIM_CACHEMBC_deps[];


static struct lp_varspec disksim_cachembc_params [] = {
   {"Cache device", S, 1 },
   {"Max request size", I, 1 },
   {"Write scheme", I, 1 },
   {"Flush idle delay", D, 1 },
   {"MBC Locations", LIST, 1 },
   {0,0,0}
};
#define DISKSIM_CACHEMBC_MAX 5
static struct lp_mod disksim_cachembc_mod = { "disksim_cachembc", disksim_cachembc_params, DISKSIM_CACHEMBC_MAX, (lp_modloader_t)disksim_cachembc_loadparams,  0, 0, DISKSIM_CACHEMBC_loaders, DISKSIM_CACHEMBC_deps };


#ifdef __cplusplus
}
#endif
#endif // _DISKSIM_CACHEMBC_PARAM_H
