//// disk power model (version 1.0)
//// Authors: Wan Jie, Zhengyi, XuJun
//// Contributors: Zhengyi
////
//// Copyright (c) of Werstern Digital, 2014-2019.
////
//// This software is being provided by the copyright holders under the
//// following license. By obtaining, using and/or copying this
//// software, you agree that you have read, understood, and will comply
//// with the following terms and conditions:
////
//// Permission to reproduce, use, and prepare derivative works of this
//// software is granted provided the copyright and "No Warranty"
//// statements are included with all reproductions and derivative works
//// and associated documentation. This software may also be
//// redistributed without charge provided that the copyright and "No
//// Warranty" statements are included in all redistributions.
////
//// NO WARRANTY. THIS SOFTWARE IS FURNISHED ON AN "AS IS" BASIS.
//// WERSTERN DIGITAL MAKES NO WARRANTIES OF ANY KIND, EITHER
//// EXPRESSED OR IMPLIED AS TO THE MATTER INCLUDING, BUT NOT LIMITED
//// TO: WARRANTY OF FITNESS FOR PURPOSE OR MERCHANTABILITY, EXCLUSIVITY
//// OF RESULTS OR RESULTS OBTAINED FROM USE OF THIS SOFTWARE. WERSTERN DIGITAL
//// DOES NOT MAKE ANY WARRANTY OF ANY KIND WITH
//// RESPECT TO FREEDOM FROM PATENT, TRADEMARK, OR COPYRIGHT
//// INFRINGEMENT.  COPYRIGHT HOLDERS WILL BEAR NO LIABILITY FOR ANY USE
//// OF THIS SOFTWARE OR DOCUMENTATION.

#ifndef __DM_POWER_H_
#define __DM_POWER_H_
#include "dm.h"

typedef struct
{
    double fPowerRAM_Idle;
   double fPowerRAM_Active;
}RAM_POWER;

typedef struct
{
    double fPowerSSD_Idle;
   double fPowerSSD_Active;
}SSD_POWER;

typedef struct
{
	double fHeadPowerRead;
	double fHeadPowerWrite;
	double fHeadServoPower;
}HEAD_RW_POWER;

typedef struct
{
	double fHDD_idle_1_inactive_wait_ms;  //  = 30
	double fHDD_idle_1_recover_time_to_active; // = 18
	double fHDD_idle_2_inactive_wait_ms;  //  = 300
	double fHDD_idle_2_recover_time_to_active; // = 30
	double fHDD_idle_3_inactive_wait_ms;  //  = 8000
	double fHDD_idle_3_recover_time_to_active; // = 320
	int iFlagHDD_idle_4_standby_trigger_mode;   // = "Timer", "OnDemand" string to flag
	double fHDD_idle_4_standby_timer_ms;		// = 60000
	double fHDD_standby_recover_time_to_active; // = 3700
	double fHDD_idle_1_power;
	double fHDD_idle_2_power;
	double fHDD_idle_3_power;
	double fHDD_idle_4_power;
        double fSpindleIdlePower_w;
}HDD_POWER_IDLE_SYSTEM_CFG;

typedef struct dm_mech_power
{
	struct dm_power_if hdr; // Function pointers
    struct dm_disk_if *disk; // back pointer to disk this goes with

     double *afPowerSpindle;        // Array of spindle power, length = number of zones, (we DONOT use the power per cylinder, since it will be very large in size.
     int nTotalCntSpindlePowerAtZone;
     int *aidxZoneSpindlePower;

     double *afPowerVCM_RdSeekEnergy;  // array of VCM Power for seeking, length = length of seek-time table
     double *afPowerVCM_WrSeekEnergy;  // array of VCM Power for seeking, length = length of seek-time table
     int nTotalCntVoiceCoilSeekTable;
     int *aidxSeekCylinderDist;
     int iFlagDualSeek;

     double *afPowerVCM_Bias;  // array of VCM power for bias current, length = number of zones; 
     int nTotalCntVoiceCoilBiasPowerAtZone;
     int *aidxZoneVoiceCoilBiasPower;

     // RAM power
     RAM_POWER stPowerRAM_w;
     // SSD power
     SSD_POWER stPowerSSD_w;

     // RW head Energy per bit
     // 1 Joule = 1 W x 1 sec = 1 Volt x 1 Ampere X 1 sec
     HEAD_RW_POWER stPowerHeadRW_w;

     // Byte per block
     int nByteSize_per_blk;

     // HDD power idle mode, system configuration
     HDD_POWER_IDLE_SYSTEM_CFG stPowerIdleModeSystemCfg;
} DISK_POWER_PARAMETER;

typedef struct calc_power_output
{
	double fHeadEnergy_RW;
	double tHeadTransferTime_ms;
	int iFlagRW;
	double fSeekEnergyVCM;
	double tSeekTime_ms;
	double tLatencyTime_ms;
	double tTransferTime_ms;
	double tAdditionalLatencyTime_ms;
	double fBiasEnergyVCM; //
	double fSpindleEnergy;
} DISK_POWER_BLK_TRACE;

//double calc_power_spindle_by_zone();
//double calc_power_vcm_bias_by_zone();
//double calc_power_seek_energy();

//extern struct dm_power_if *
//dm_mech_power_loadparams(struct lp_block *b, struct dm_disk_if *parent);

extern struct dm_power_if mech_power;

#endif
