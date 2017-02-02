
#ifndef _DM_MECH_POWER_PARAM_H
#define _DM_MECH_POWER_PARAM_H  

#include <libparam/libparam.h>
#ifdef __cplusplus
extern"C"{
#endif
struct dm_disk_if;

/* prototype for dm_mech_power param loader function */
struct dm_power_if *dm_mech_power_loadparams(struct lp_block *b, struct dm_disk_if *parent);

typedef enum {
   DM_MECH_POWER_SPINDLE_POWER_CURVE_VS_ZONE,
   DM_MECH_POWER_VCM_SEEK_ENERGY_VS_CYLINDER_DISTANCE,
   DM_MECH_POWER_VCM_BIAS_POWER_VS_ZONE,
   DM_MECH_POWER_HEAD_READ_POWER,
   DM_MECH_POWER_HEAD_WRITE_POWER,
   DM_MECH_POWER_SERVO_CHANNEL_POWER,
   DM_MECH_POWER_BYTE_SIZE_PER_BLOCK,
   DM_MECH_POWER_RAM_ACTIVE_POWER,
   DM_MECH_POWER_RAM_IDLE_POWER,
   DM_MECH_POWER_SSD_ACTIVE_POWER,
   DM_MECH_POWER_SSD_IDLE_POWER,
   DM_MECH_POWER_HDD_IDLE_ONE_INACTIVE_WAIT_MS,
   DM_MECH_POWER_HDD_IDLE_ONE_RECOVER_TIME_TO_ACTIVE,
   DM_MECH_POWER_HDD_POWER_IDLE_ONE_IN_WATT,
   DM_MECH_POWER_SPINDLE_IDLE_POWER,
   DM_MECH_POWER_HDD_IDLE_TWO_INACTIVE_WAIT_MS,
   DM_MECH_POWER_HDD_IDLE_TWO_RECOVER_TIME_TO_ACTIVE,
   DM_MECH_POWER_HDD_POWER_IDLE_TWO_IN_WATT,
   DM_MECH_POWER_HDD_IDLE_THREE_INACTIVE_WAIT_MS,
   DM_MECH_POWER_HDD_IDLE_THREE_RECOVER_TIME_TO_ACTIVE,
   DM_MECH_POWER_HDD_POWER_IDLE_THREE_IN_WATT,
   DM_MECH_POWER_HDD_IDLE_FOUR_STANDBY_TRIGGER_MODE,
   DM_MECH_POWER_HDD_IDLE_FOUR_STANDBY_TIMER_MS,
   DM_MECH_POWER_HDD_STANDBY_RECOVER_TIME_TO_ACTIVE,
   DM_MECH_POWER_HDD_POWER_IDLE_FOUR_IN_WATT
} dm_mech_power_param_t;

#define DM_MECH_POWER_MAX_PARAM		DM_MECH_POWER_HDD_POWER_IDLE_FOUR_IN_WATT
extern void * DM_MECH_POWER_loaders[];
extern lp_paramdep_t DM_MECH_POWER_deps[];


static struct lp_varspec dm_mech_power_params [] = {
   {"Spindle power curve vs zone", S, 0 },
   {"VCM Seek energy vs cylinder distance", S, 0 },
   {"VCM Bias power vs zone", S, 0 },
   {"Head read power", D, 1 },
   {"Head write power", D, 1 },
   {"Servo channel power", D, 1 },
   {"Byte size per block", I, 1 },
   {"RAM active power", D, 1 },
   {"RAM idle power", D, 1 },
   {"SSD active power", D, 1 },
   {"SSD idle power", D, 1 },
   {"HDD idle one inactive wait ms", D, 1 },
   {"HDD idle one recover time to active", D, 1 },
   {"HDD Power idle one in watt", D, 1 },
   {"Spindle idle power", D, 1 },
   {"HDD idle two inactive wait ms", D, 1 },
   {"HDD idle two recover time to active", D, 1 },
   {"HDD Power idle two in watt", D, 1 },
   {"HDD idle three inactive wait ms", D, 1 },
   {"HDD idle three recover time to active", D, 1 },
   {"HDD Power idle three in watt", D, 1 },
   {"HDD idle four standby trigger mode", S, 0 },
   {"HDD idle four standby timer ms", D, 1 },
   {"HDD standby recover time to active", D, 1 },
   {"HDD Power idle four in watt", D, 1 },
   {0,0,0}
};
#define DM_MECH_POWER_MAX 25
static struct lp_mod dm_mech_power_mod = { "dm_mech_power", dm_mech_power_params, DM_MECH_POWER_MAX, (lp_modloader_t)dm_mech_power_loadparams,  0, 0, DM_MECH_POWER_loaders, DM_MECH_POWER_deps };


#ifdef __cplusplus
}
#endif
#endif // _DM_MECH_POWER_PARAM_H
