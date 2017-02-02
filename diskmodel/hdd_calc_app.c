
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <dm.h>

// #include "test_hdd_mecha/hdd_model_calculator.h"
// for a simplified conversion, only cylinder and head are used,
  //           i.e. only for functions like, seek, and vcm motion distance, and etc.
//                   purpose is for faster calculation in real-time handlings.
  // NOT for the other functions like, skew time and etc.
void hdd_convert_pbn_ch_to_mech_state(struct dm_pbn *stpInputPBN,
		struct dm_mech_state *stpOutputMechState
		)
{
	stpOutputMechState->cyl = stpInputPBN->cyl;
	stpOutputMechState->head = stpInputPBN->head;
	stpOutputMechState->theta = 0;
}
// for an exact conversion, cylinder and head, sectors are used,
// For all functions, but it is slower for referring the disk layout model.
void hdd_convert_pbn_chs_to_mech_state(struct dm_disk_if *d,
		struct dm_pbn *stpInputPBN,
		struct dm_mech_state *stpOutputMechState
		)
{
	stpOutputMechState->cyl = stpInputPBN->cyl;
	stpOutputMechState->head = stpInputPBN->head;

	stpOutputMechState->theta = d->layout->dm_pbn_skew(d, stpInputPBN);
}

// Convertion from mech state to pbn, @018
void hdd_convert_mech_state_to_pbn_ch(struct dm_mech_state *stpInputMechState,
		struct dm_pbn *stpOutputPBN)
{
	stpOutputPBN->cyl = stpInputMechState->cyl;
	stpOutputPBN->head = stpInputMechState->head;
	stpOutputPBN->sector = 0;
}

#define DEBUG_MATCH_MAXTOR_TRACE
#ifdef DEBUG_MATCH_MAXTOR_TRACE
#define OFFSET_SIM_START_TIME  (32.66107)
#else
#define OFFSET_SIM_START_TIME  (0)
#endif


int hdd_model_calculate_access_time(struct dm_disk_if *d,
		INPUT_HDD_CALCULATE_ACCESS_TIME *stpInputCalcHDD,
		OUTPUT_HDD_CALCULATE_ACCESS_TIME *stpOutputCalcHDD)
{
uint64_t nSeekTime, nAccessTime, nXfterTime, nLatency;
struct dm_pbn stTargetStartPBA, stFromPBA, stEndPBA;
struct dm_pbn stPBA_FirstSectorFollowingTargetTrack, stLastPBA_TargetLBA;
LBA_TYPE iToLBA, iFromLBA;
LBA_TYPE firstLBA_trackFrom, lastLBA_trackFrom;
LBA_TYPE firstLBA_trackTarget, lastLBA_trackTarget;
unsigned int nBlkCount = 0, nFlagRW = 1;
double tStartSeek_ms;
dm_ptol_result_t rc;
dm_angle_t skewTargetStartLBA, skewFrom;
dm_angle_t skewNextTrackFollowTargetStartLBA;
dm_angle_t skewLastSectorTargetStartLBA;
int iRet = CALC_OK;

	iFromLBA = stpInputCalcHDD->nStartLBA;
	iToLBA = stpInputCalcHDD->nTargetLBA;
	nBlkCount = stpInputCalcHDD->nLen;
	nFlagRW = stpInputCalcHDD->nFlag;
	tStartSeek_ms = stpInputCalcHDD->tStartSeek_ms;


	//// Calculation for the fromLBA
	rc = d->layout->dm_translate_ltop(d, iFromLBA, MAP_FULL, &stFromPBA, 0);
	if(rc != DM_OK)
	{
		iRet = CALC_ERR; stpOutputCalcHDD->nFlagCalculationError = rc;
		stpOutputCalcHDD->nErrorLine = __LINE__;
		sprintf(stpOutputCalcHDD->strFilename, "%s", __FILE__);
		goto __label_hdd_model_calculate_access_time;
	}
	skewFrom = d->layout->dm_pbn_skew(d, &stFromPBA);
    d->layout->dm_get_track_boundaries(d, &stFromPBA, &firstLBA_trackFrom, &lastLBA_trackFrom, 0);
	stpOutputCalcHDD->dFromSkew = dm_angle_itod(skewFrom);
	stpOutputCalcHDD->iFromCylinder = stFromPBA.cyl;
	stpOutputCalcHDD->iFromHead = stFromPBA.head;
	stpOutputCalcHDD->iFromSector = stFromPBA.sector ;
	stpOutputCalcHDD->nFirstLBAonTrackFrom = firstLBA_trackFrom;
	stpOutputCalcHDD->nLastLBAonTrackFrom = lastLBA_trackFrom;

	//// Calculation for the TargetLBA
	rc = d->layout->dm_translate_ltop(d, iToLBA, MAP_FULL, &stTargetStartPBA, 0);
	if(rc != DM_OK)
	{
		iRet = CALC_ERR; stpOutputCalcHDD->nFlagCalculationError = rc;
		stpOutputCalcHDD->nErrorLine = __LINE__;
		sprintf(stpOutputCalcHDD->strFilename, "%s", __FILE__);
		goto __label_hdd_model_calculate_access_time;
	}
	skewTargetStartLBA = d->layout->dm_pbn_skew(d, &stTargetStartPBA);
    d->layout->dm_get_track_boundaries(d, &stTargetStartPBA,
    		&firstLBA_trackTarget, &lastLBA_trackTarget, 0);
	stpOutputCalcHDD->dTargetSkew = dm_angle_itod(skewTargetStartLBA);
	stpOutputCalcHDD->iTargetCylinder = stTargetStartPBA.cyl;
	stpOutputCalcHDD->iTargetHead = stTargetStartPBA.head;
	stpOutputCalcHDD->iTargetSector = stTargetStartPBA.sector ;
	stpOutputCalcHDD->nFirstLBAonTrackTarget = firstLBA_trackTarget;
	stpOutputCalcHDD->nLastLBAonTrackTarget = lastLBA_trackTarget;

	/// Calculation of the
	nSeekTime = d->mech->dm_seek_time(d,
			 (struct dm_mech_state *)&stFromPBA,
            (struct dm_mech_state *)&stTargetStartPBA,
            (nFlagRW & DISKSIM_FLAG_BIT_RW));

	 // Below is the calculation of the media rotation during the seek-time
	dm_time_t residtime =
			 (nSeekTime + dm_time_dtoi(tStartSeek_ms))
			 % d->mech->dm_period(d);
	 struct dm_mech_state startstate;
	 startstate.cyl = stTargetStartPBA.cyl;    // Now the R/W head is already on the target track
	 startstate.head = stTargetStartPBA.head;
#ifdef DEBUG_MATCH_MAXTOR_TRACE
	 startstate.theta = 0; // assume 0 start for matching report of w2-Dec/2013
#else
	 startstate.theta = skewFrom;
#endif
	 startstate.theta += d->mech->dm_rotate(d,
	 							&residtime);

	 stpOutputCalcHDD->dSkewAfterSeek = dm_angle_itod(startstate.theta);

	 nLatency = d->mech->dm_latency(d,
	    		    (struct dm_mech_state *)&startstate,
	    		    stTargetStartPBA.sector,
	    		    nBlkCount,
					0,  // immediate R/W flag,
					0); // always 0 additional latency
	 nXfterTime = d->mech->dm_xfertime(d,
			 (struct dm_mech_state *)&stTargetStartPBA,
			 1) * nBlkCount;


	 stpOutputCalcHDD->tSeek_ms = dm_time_itod(nSeekTime);
	 stpOutputCalcHDD->tTransfer_ms = dm_time_itod(nXfterTime);
	 stpOutputCalcHDD->tLatency_ms = dm_time_itod(nLatency);


	 stpOutputCalcHDD->tAccessTime_ms =
			 stpOutputCalcHDD->tSeek_ms +
			 stpOutputCalcHDD->tTransfer_ms +
			 stpOutputCalcHDD->tLatency_ms; // dm_time_itod(nAccessTime);

	 stpOutputCalcHDD->dAdditionalLatency_ms = 0;
	 //////// In most cases, the target access length of data is in one single track
	 /// For the case that the read length
	 LBA_TYPE nEndLBA = nBlkCount + iToLBA;

	 // LBA_TYPE nFirstLBAnext
	 if(nEndLBA > lastLBA_trackTarget)
	 {
		 rc = d->layout->dm_translate_ltop(d, nEndLBA, MAP_FULL, &stEndPBA, 0);
		 if(rc != DM_OK)
		 {
			 iRet = CALC_ERR; stpOutputCalcHDD->nFlagCalculationError = rc;
			 stpOutputCalcHDD->nErrorLine = __LINE__;
			 sprintf(stpOutputCalcHDD->strFilename, "%s", __FILE__);
			 goto __label_hdd_model_calculate_access_time;
		 }

		 LBA_TYPE nFirstLBAnextTrack, nLastLBAnextTrack;
		 nFirstLBAnextTrack = lastLBA_trackTarget + 1;
		 rc = d->layout->dm_translate_ltop(d, nFirstLBAnextTrack, MAP_FULL, &stPBA_FirstSectorFollowingTargetTrack, 0);
		 if(rc != DM_OK)
		 {
			 iRet = CALC_ERR; stpOutputCalcHDD->nFlagCalculationError = rc;
			 stpOutputCalcHDD->nErrorLine = __LINE__;
			 sprintf(stpOutputCalcHDD->strFilename, "%s", __FILE__);
			 goto __label_hdd_model_calculate_access_time;
		 }
		 d->layout->dm_get_track_boundaries(d, &stPBA_FirstSectorFollowingTargetTrack,
				 &nFirstLBAnextTrack, &nLastLBAnextTrack, 0);

		 d->layout->dm_translate_ltop(d, lastLBA_trackTarget, MAP_FULL, &stLastPBA_TargetLBA, 0);
		 skewLastSectorTargetStartLBA =
		 					 d->layout->dm_pbn_skew(d, &stLastPBA_TargetLBA);
		 if(nEndLBA > nLastLBAnextTrack)
		 {
			 iRet = CALC_ERR;
			 stpOutputCalcHDD->nFlagCalculationError = HDD_MODEL_CALCULATOR_MORE_THAN_2_TRACK;
			 stpOutputCalcHDD->nErrorLine = __LINE__;
			 sprintf(stpOutputCalcHDD->strFilename, "%s", __FILE__);
			 goto __label_hdd_model_calculate_access_time;

		 }
		 else
		 {
			 skewNextTrackFollowTargetStartLBA =
					 d->layout->dm_pbn_skew(d, &stPBA_FirstSectorFollowingTargetTrack);

			 uint64_t nAdditionalLatency =
					 d->mech->dm_rottime(d, skewLastSectorTargetStartLBA, skewNextTrackFollowTargetStartLBA);
			 double dAdditionalLatency_ms = dm_time_itod(nAdditionalLatency);

			 // We need to consider the possible seek time and head-switch time
			 uint64_t nSeekTimeOneCylinder =
					 d->mech->dm_seek_time(d,
					 			 (struct dm_mech_state *)&stTargetStartPBA,
					             (struct dm_mech_state *)&stPBA_FirstSectorFollowingTargetTrack,
					             (nFlagRW & DISKSIM_FLAG_BIT_RW));

			 double dSeekTimeOneAdditionalTrack_ms = dm_time_itod(nSeekTimeOneCylinder);

			 if(dAdditionalLatency_ms < dSeekTimeOneAdditionalTrack_ms)
			 {
				 dAdditionalLatency_ms = dSeekTimeOneAdditionalTrack_ms;
			 }
//			 stpOutputCalcHDD->tLatency_ms += dAdditionalLatency_ms;
			 stpOutputCalcHDD->tAccessTime_ms += dAdditionalLatency_ms;
			 stpOutputCalcHDD->dAdditionalLatency_ms = dAdditionalLatency_ms;
		 }
	 }

__label_hdd_model_calculate_access_time:
return iRet;
}
