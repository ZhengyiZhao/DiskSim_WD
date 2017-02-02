
#ifndef _DM_DISK_PARAM_H
#define _DM_DISK_PARAM_H  

#include <libparam/libparam.h>
#ifdef __cplusplus
extern"C"{
#endif
struct dm_disk_if;

/* prototype for dm_disk param loader function */
struct dm_disk_if * dm_disk_loadparams(struct lp_block *b, int *num);

typedef enum {
   DM_DISK_BLOCK_COUNT,
   DM_DISK_NUMBER_OF_DATA_SURFACES,
   DM_DISK_NUMBER_OF_CYLINDERS,
   DM_DISK_FLAG_OF_MEADIA_HEAD,
   DM_DISK_NUMBER_OF_WEDGES,
   DM_DISK_MECHANICAL_MODEL,
   DM_DISK_LAYOUT_MODEL,
   DM_DISK_DISK_POWER
} dm_disk_param_t;

#define DM_DISK_MAX_PARAM		DM_DISK_DISK_POWER
extern void * DM_DISK_loaders[];
extern lp_paramdep_t DM_DISK_deps[];


static struct lp_varspec dm_disk_params [] = {
   {"Block count", I, 1 },
   {"Number of data surfaces", I, 1 },
   {"Number of cylinders", I, 1 },
   {"Flag of meadia head(smr or cmr)", I, 0 },
   {"Number of wedges", I, 0 },
   {"Mechanical Model", BLOCK, 0 },
   {"Layout Model", BLOCK, 1 },
   {"Disk Power", BLOCK, 0 },
   {0,0,0}
};
#define DM_DISK_MAX 8
static struct lp_mod dm_disk_mod = { "dm_disk", dm_disk_params, DM_DISK_MAX, (lp_modloader_t)dm_disk_loadparams,  0, 0, DM_DISK_loaders, DM_DISK_deps };


#ifdef __cplusplus
}
#endif
#endif // _DM_DISK_PARAM_H
