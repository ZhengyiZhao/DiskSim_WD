
#ifndef _DM_DISK_POWER_PARAM_H
#define _DM_DISK_POWER_PARAM_H  

#include <libparam/libparam.h>
#ifdef __cplusplus
extern"C"{
#endif
struct dm_disk_if;

/* prototype for dm_disk_power param loader function */
struct dm_power_if *dm_mech_power_loadparams(struct lp_block *b, int *num);

typedef enum {
   DM_DISK_POWER_SPINDLE_POWER_CURVE_VS_ZONE,
   DM_DISK_POWER_VCM_SEEK_ENERGY_VS_CYLINDER_DISTANCE,
   DM_DISK_POWER_VCM_BIAS_POWER_VS_ZONE
} dm_disk_power_param_t;

#define DM_DISK_POWER_MAX_PARAM		DM_DISK_POWER_VCM_BIAS_POWER_VS_ZONE
extern void * DM_DISK_POWER_loaders[];
extern lp_paramdep_t DM_DISK_POWER_deps[];


static struct lp_varspec dm_disk_power_params [] = {
   {"Spindle power curve vs zone", S, 0 },
   {"VCM Seek energy vs cylinder distance", S, 0 },
   {"VCM Bias power vs zone", S, 0 },
   {0,0,0}
};
#define DM_DISK_POWER_MAX 3
static struct lp_mod dm_disk_power_mod = { "dm_disk_power", dm_disk_power_params, DM_DISK_POWER_MAX, (lp_modloader_t)dm_disk_power_loadparams,  0, 0, DM_DISK_POWER_loaders, DM_DISK_POWER_deps };


#ifdef __cplusplus
}
#endif
#endif // _DM_DISK_POWER_PARAM_H
