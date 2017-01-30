
#ifndef _DISKSIM_LOGORG_PARAM_H
#define _DISKSIM_LOGORG_PARAM_H  

#include <libparam/libparam.h>
#ifdef __cplusplus
extern"C"{
#endif
struct dm_disk_if;

/* prototype for disksim_logorg param loader function */
struct logorg *disksim_logorg_loadparams(struct lp_block *b);

typedef enum {
   DISKSIM_LOGORG_ADDRESSING_MODE,
   DISKSIM_LOGORG_DISTRIBUTION_SCHEME,
   DISKSIM_LOGORG_REDUNDANCY_SCHEME,
   DISKSIM_LOGORG_ERASURE_CODE_SCHEME,
   DISKSIM_LOGORG_COMPONENTS,
   DISKSIM_LOGORG_DEVICES,
   DISKSIM_LOGORG_STRIPE_UNIT,
   DISKSIM_LOGORG_SYNCH_WRITES_FOR_SAFETY,
   DISKSIM_LOGORG_NUMBER_OF_COPIES,
   DISKSIM_LOGORG_COPY_CHOICE_ON_READ,
   DISKSIM_LOGORG_RMW_VS_RECONSTRUCT,
   DISKSIM_LOGORG_PARITY_STRIPE_UNIT,
   DISKSIM_LOGORG_PARITY_ROTATION_TYPE,
   DISKSIM_LOGORG_NUMBER_DRIVES_PER_CKG,
   DISKSIM_LOGORG_TOTAL_NUMBER_PARITY_PATTERNS,
   DISKSIM_LOGORG_TOTAL_NUMBER_CKGS,
   DISKSIM_LOGORG_CHUNK_GROUP_MAP,
   DISKSIM_LOGORG_HOT_SPARE_RESERVE,
   DISKSIM_LOGORG_IDLE_DELAY_MS,
   DISKSIM_LOGORG_NODE_ID_TO_FORCE_REBUILD,
   DISKSIM_LOGORG_TOTAL_DRIVES_TO_REBUILD,
   DISKSIM_LOGORG_NODE_ID_LIST_TO_FORCE_REBUILD,
   DISKSIM_LOGORG_TOTAL_STRIPES_TO_REBUILD,
   DISKSIM_LOGORG_TOTAL_STRIPES__TO_REBUILD,
   DISKSIM_LOGORG_STARTING_STRIPE_REBUILD_PERCENTAGE,
   DISKSIM_LOGORG_CHECKING_INTERVAL_MS,
   DISKSIM_LOGORG_IDLE_DELAY_IN_MS,
   DISKSIM_LOGORG_PRIORITY_REBUILD_NODE,
   DISKSIM_LOGORG_FLAG_REPAIR_NODE_STAGE,
   DISKSIM_LOGORG_TIME_STAMP_INTERVAL,
   DISKSIM_LOGORG_TIME_STAMP_START_TIME,
   DISKSIM_LOGORG_TIME_STAMP_STOP_TIME,
   DISKSIM_LOGORG_TIME_STAMP_FILE_NAME
} disksim_logorg_param_t;

#define DISKSIM_LOGORG_MAX_PARAM		DISKSIM_LOGORG_TIME_STAMP_FILE_NAME
extern void * DISKSIM_LOGORG_loaders[];
extern lp_paramdep_t DISKSIM_LOGORG_deps[];


static struct lp_varspec disksim_logorg_params [] = {
   {"Addressing mode", S, 1 },
   {"Distribution scheme", S, 1 },
   {"Redundancy scheme", S, 1 },
   {"Erasure code scheme", S, 0 },
   {"Components", S, 0 },
   {"devices", LIST, 1 },
   {"Stripe unit", I, 1 },
   {"Synch writes for safety", I, 1 },
   {"Number of copies", I, 1 },
   {"Copy choice on read", I, 1 },
   {"RMW vs. reconstruct", D, 1 },
   {"Parity stripe unit", I, 1 },
   {"Parity rotation type", I, 0 },
   {"Number drives per ckg", I, 0 },
   {"Total number parity patterns", I, 0 },
   {"Total number ckgs", I, 0 },
   {"Chunk group map", S, 0 },
   {"Hot spare reserve", S, 0 },
   {"Idle delay ms", D, 0 },
   {"Node id to force rebuild", I, 0 },
   {"Total drives to rebuild", I, 0 },
   {"Node id list to force rebuild", S, 0 },
   {"Total stripes to rebuild", I, 0 },
   {"Total stripes (zones) to rebuild", I, 0 },
   {"Starting stripe rebuild percentage", D, 0 },
   {"Checking interval ms", D, 0 },
   {"Idle delay in ms", D, 0 },
   {"Priority rebuild node", I, 0 },
   {"Flag repair node stage", I, 0 },
   {"Time stamp interval", D, 0 },
   {"Time stamp start time", D, 0 },
   {"Time stamp stop time", D, 0 },
   {"Time stamp file name", S, 0 },
   {0,0,0}
};
#define DISKSIM_LOGORG_MAX 33
static struct lp_mod disksim_logorg_mod = { "disksim_logorg", disksim_logorg_params, DISKSIM_LOGORG_MAX, (lp_modloader_t)disksim_logorg_loadparams,  0, 0, DISKSIM_LOGORG_loaders, DISKSIM_LOGORG_deps };


#ifdef __cplusplus
}
#endif
#endif // _DISKSIM_LOGORG_PARAM_H
