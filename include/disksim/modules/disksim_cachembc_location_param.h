
#ifndef _DISKSIM_CACHEMBC_LOCATION_PARAM_H
#define _DISKSIM_CACHEMBC_LOCATION_PARAM_H  

#include <libparam/libparam.h>
#ifdef __cplusplus
extern"C"{
#endif
struct dm_disk_if;

/* prototype for disksim_cachembc_location param loader function */
int disksim_cachembc_location_loadparams(struct lp_block *b);

typedef enum {
   DISKSIM_CACHEMBC_LOCATION_START_LBA,
   DISKSIM_CACHEMBC_LOCATION_SIZE
} disksim_cachembc_location_param_t;

#define DISKSIM_CACHEMBC_LOCATION_MAX_PARAM		DISKSIM_CACHEMBC_LOCATION_SIZE
extern void * DISKSIM_CACHEMBC_LOCATION_loaders[];
extern lp_paramdep_t DISKSIM_CACHEMBC_LOCATION_deps[];


static struct lp_varspec disksim_cachembc_location_params [] = {
   {"Start LBA", I, 1 },
   {"Size", I, 1 },
   {0,0,0}
};
#define DISKSIM_CACHEMBC_LOCATION_MAX 2
static struct lp_mod disksim_cachembc_location_mod = { "disksim_cachembc_location", disksim_cachembc_location_params, DISKSIM_CACHEMBC_LOCATION_MAX, (lp_modloader_t)disksim_cachembc_location_loadparams,  0, 0, DISKSIM_CACHEMBC_LOCATION_loaders, DISKSIM_CACHEMBC_LOCATION_deps };


#ifdef __cplusplus
}
#endif
#endif // _DISKSIM_CACHEMBC_LOCATION_PARAM_H
