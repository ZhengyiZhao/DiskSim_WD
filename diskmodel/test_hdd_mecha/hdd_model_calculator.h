
#ifndef HDD_MODEL_CALCULATOR_H
#define HDD_MODEL_CALCULATOR_H

#include <diskmodel/dm.h>


typedef struct
{
	double tSeek_ms;
	double tLatency_ms;
	double tTransfer_ms;
	double tAccessTime_ms;
	double dAdditionalLatency_ms;
	int iFromCylinder;
	int iFromHead;
	int iFromSector;
	double dFromSkew;
	LBA_TYPE nFirstLBAonTrackFrom;
	LBA_TYPE nLastLBAonTrackFrom;
	int iTargetCylinder;
	int iTargetHead;
	int iTargetSector;
	double dTargetSkew;
	LBA_TYPE nFirstLBAonTrackTarget;
	LBA_TYPE nLastLBAonTrackTarget;
	double dSkewAfterSeek;

//  Power calculation output
	double dEnergySeekVCM_watt_sec;
	double dEnergyBiasVCM_watt_sec;
	double dEnergySpindle_watt_sec;
	double dEnergyHeadServo_watt_sec;
	double dEnergyHeadReadWrite_watt_sec;
	double dEnergyIdle_watt_sec;  // @012.2
	int nFlagIdleMode;
		// 0: always active,
	    // 1: upto idle-1
	    // 2: upto idle-2
	    // 3: upto idle-3
	    // 4: upto stand-by
	int nTargetZone;
	double tRegEndSeekTime;
	double tRegEndRW_Time;

	int nFlagCalculationError;
	int nErrorLine;

	char strFilename[1024];
}OUTPUT_HDD_CALCULATE_TIME_AND_ENERGY;

extern int hdd_model_calculate_access_time_and_power(struct dm_disk_if *d,
		INPUT_HDD_CALCULATE_ACCESS_TIME *stpInputCalcHDD,
		OUTPUT_HDD_CALCULATE_TIME_AND_ENERGY *stpOutputCalcHDD);

#endif // HDD_MODEL_CALCULATOR_H
