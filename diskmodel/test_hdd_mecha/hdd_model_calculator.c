
//Id    DDMMYYYY    Noter         Comment
//@012  ??032015    Zhengyi       Add-in power calculation, only for FCFS scheduling
//@012.1  10042015    Zhengyi       Bug correction, thanks for Grant
//@012.2  10062015    Zhengyi       Bug correction, thanks for Michael Agun
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "hdd_model_calculator.h"
#include <diskmodel/dm.h>


int hdd_model_calculate_access_time_and_power(struct dm_disk_if *d,
		INPUT_HDD_CALCULATE_ACCESS_TIME *stpInputCalcHDD,
		OUTPUT_HDD_CALCULATE_TIME_AND_ENERGY *stpOutputCalcHDD)
{
uint64_t nSeekTime, nAccessTime, nXfterTime, nLatency;
struct dm_pbn stTargetStartPBA, stFromPBA, stEndPBA;
struct dm_mech_state stFromMechState, stTargetMechState; // @1
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

    hdd_convert_pbn_ch_to_mech_state(&stFromPBA, &stFromMechState); // @012.1
	hdd_convert_pbn_ch_to_mech_state(&stTargetStartPBA, &stTargetMechState);  // @012.1
	/// Calculation of the
	nSeekTime = d->mech->dm_seek_time(d,
			 &stFromMechState,    // (struct dm_mech_state *)&stFromPBA,  // @012.1
             &stTargetMechState,  // (struct dm_mech_state *)&stTargetStartPBA, // @012.1
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

	 stpOutputCalcHDD->tRegEndRW_Time = stpInputCalcHDD->tStartSeek_ms + stpOutputCalcHDD->tAccessTime_ms;
	 double tIdle_wait_ms = stpInputCalcHDD->tStartSeek_ms - stpInputCalcHDD->tRegLastEndRW;
#define __DISK_POWER__
#ifdef __DISK_POWER__
        // @012
	 int nRegLastZone = stpInputCalcHDD->nRegLastZone;
	 double tRegEndSeekTimeLastTrace_ms = stpInputCalcHDD->tRegEndSeekTime;
	 double tSeekTime_ms = stpOutputCalcHDD->tSeek_ms;

	 double fVCMSeekEnergy =
        		d->stpPowerHDD->dm_vcm_seek_energy(d,
        		&stFromMechState, &stTargetMechState, (nFlagRW & DISKSIM_FLAG_BIT_RW));  // @012.1

        stpOutputCalcHDD->dEnergySeekVCM_watt_sec =
        		0.001 * fVCMSeekEnergy * tSeekTime_ms;  // @012

        /// for the below modulse, we need to consider idle-{1,2,3} modes
        DISK_POWER_PARAMETER *stpPowerParameter = (DISK_POWER_PARAMETER *)(d->stpPowerHDD);
        double fSpindlePowerPrevZone;
        fSpindlePowerPrevZone =
        		d->stpPowerHDD->dm_spindle_power(d, nRegLastZone);
        // The time's unit is 1ms, so it needs a factor of 0.001
        stpOutputCalcHDD->dEnergySpindle_watt_sec = 0.001 * fSpindlePowerPrevZone *
        		(tStartSeek_ms + tSeekTime_ms - tRegEndSeekTimeLastTrace_ms) ;

        double fVCM_BiasPowerPrevZone;
        fVCM_BiasPowerPrevZone =
        		d->stpPowerHDD->dm_vcm_bias_power(d, nRegLastZone);
        stpOutputCalcHDD->dEnergyBiasVCM_watt_sec =
        		0.001 * fVCM_BiasPowerPrevZone *
        		(tStartSeek_ms + tSeekTime_ms - tRegEndSeekTimeLastTrace_ms);

        int nTargetZone;
        nTargetZone = d->layout->dm_get_zone_id_by_lbn(d,iToLBA);
        stpOutputCalcHDD->nTargetZone = nTargetZone;

        stpOutputCalcHDD->tRegEndSeekTime = tStartSeek_ms + tSeekTime_ms; // @012

        double fHeadEnergyRW;
        if(nFlagRW & DISKSIM_FLAG_BIT_RW)
        {
        	fHeadEnergyRW =
        			((stpPowerParameter->stPowerHeadRW_w.fHeadPowerRead )*
        					stpOutputCalcHDD->tTransfer_ms * 0.001);

        }
        else
        {
        	fHeadEnergyRW =  // stpPowerParameter->stPowerHeadRW_w.fHeadServoPower +
        			((stpPowerParameter->stPowerHeadRW_w.fHeadPowerWrite) *
        					stpOutputCalcHDD->tTransfer_ms * 0.001);
        }
        stpOutputCalcHDD->dEnergyHeadReadWrite_watt_sec = fHeadEnergyRW;

        stpOutputCalcHDD->dEnergyHeadServo_watt_sec = 0.001 *
        		stpPowerParameter->stPowerHeadRW_w.fHeadServoPower *
        		(stpOutputCalcHDD->tTransfer_ms + stpOutputCalcHDD->tLatency_ms + stpOutputCalcHDD->tSeek_ms);


        int nFlagIdleMode = 0;
        double dEnergyIdle_watt_sec = 0; // @012.2
        double tIdle_m_1, tIdle_m_1n2, tIdle_m_1n2n3, tIdle_m_1n2n3nStandby;
        tIdle_m_1 = tIdle_wait_ms - stpPowerParameter->stPowerIdleModeSystemCfg.fHDD_idle_1_inactive_wait_ms;
        if(tIdle_m_1 < stpPowerParameter->stPowerIdleModeSystemCfg.fHDD_idle_1_recover_time_to_active) // default 18)
        { /// all the time active
        	nFlagIdleMode = 0;
        	dEnergyIdle_watt_sec =  0.001 * tIdle_wait_ms *  // @012.2
        			( fSpindlePowerPrevZone
        			  + stpPowerParameter->stPowerHeadRW_w.fHeadServoPower
        			  + fVCM_BiasPowerPrevZone
        			  );
        }
        else
        {
        	tIdle_m_1n2 = tIdle_m_1 - stpPowerParameter->stPowerIdleModeSystemCfg.fHDD_idle_2_inactive_wait_ms;
        	if(tIdle_m_1n2 < stpPowerParameter->stPowerIdleModeSystemCfg.fHDD_idle_2_recover_time_to_active) // default 30
        	{ /// active for fHDD_idle_1_inactive_wait_ms
        		/// idle-1 for tIdle_m_1
        		nFlagIdleMode = 1;
            	dEnergyIdle_watt_sec =  // @012.2
            			( 0.001 * stpPowerParameter->stPowerIdleModeSystemCfg.fHDD_idle_1_inactive_wait_ms *
            			  ( fSpindlePowerPrevZone
            			  + stpPowerParameter->stPowerHeadRW_w.fHeadServoPower
            			  + fVCM_BiasPowerPrevZone
            			  )
            			)
            			  +
            			  (0.001 * tIdle_m_1 *
            					  (fSpindlePowerPrevZone
            	            			  + stpPowerParameter->stPowerHeadRW_w.fHeadServoPower
            	            			  + fVCM_BiasPowerPrevZone
            	            			  + stpPowerParameter->stPowerIdleModeSystemCfg.fHDD_idle_1_power));

        	}
        	else
        	{
        		tIdle_m_1n2n3 = tIdle_m_1n2 - stpPowerParameter->stPowerIdleModeSystemCfg.fHDD_idle_3_inactive_wait_ms;

        		if(tIdle_m_1n2n3 < stpPowerParameter->stPowerIdleModeSystemCfg.fHDD_idle_3_recover_time_to_active)
        		{ /// active for stpPowerParameter->stPowerIdleModeSystemCfg.fHDD_idle_1_inactive_wait_ms
        			/// idle-1  for stpPowerParameter->stPowerIdleModeSystemCfg.fHDD_idle_2_inactive_wait_ms
        			/// idle-2  for tIdle_m_1n2
        			nFlagIdleMode = 2;
                	dEnergyIdle_watt_sec =  // @012.2
                			( 0.001 * stpPowerParameter->stPowerIdleModeSystemCfg.fHDD_idle_1_inactive_wait_ms *
                			  ( fSpindlePowerPrevZone
                			  + stpPowerParameter->stPowerHeadRW_w.fHeadServoPower
                			  + fVCM_BiasPowerPrevZone
                			  )
                			)
                			  +
                			  (0.001 * stpPowerParameter->stPowerIdleModeSystemCfg.fHDD_idle_2_inactive_wait_ms *
                					  (fSpindlePowerPrevZone
                	            			  + stpPowerParameter->stPowerHeadRW_w.fHeadServoPower
                	            			  + fVCM_BiasPowerPrevZone
                	            			  + stpPowerParameter->stPowerIdleModeSystemCfg.fHDD_idle_1_power))
                	          +(0.001 * tIdle_m_1n2 *
                					  (fSpindlePowerPrevZone
                	            			  + fVCM_BiasPowerPrevZone
                	            			  + stpPowerParameter->stPowerIdleModeSystemCfg.fHDD_idle_2_power)
                	        		  );

        		}
        		else
        		{
        			tIdle_m_1n2n3nStandby = tIdle_m_1n2n3 - stpPowerParameter->stPowerIdleModeSystemCfg.fHDD_idle_4_standby_timer_ms;
        			// Suppose standby is triggered by timer also
    		        double fSpindlePowerZoneId; // in Idle-3, the spindle has minimum power s.t. head parked, approximated by Id
    		        fSpindlePowerZoneId =
    		        		d->stpPowerHDD->dm_spindle_power(d, d->layout->dm_get_numzones(d)- 1);

        			if(tIdle_m_1n2n3nStandby < stpPowerParameter->stPowerIdleModeSystemCfg.fHDD_standby_recover_time_to_active)
        			{
        				/// Active: fHDD_idle_1_inactive_wait_ms
        				/// Idle-1: fHDD_idle_2_inactive_wait_ms
        				/// Idle-2: fHDD_idle_3_inactive_wait_ms
        				/// Idle-3: tIdle_m_1n2n3,
        				nFlagIdleMode = 3;
                    	dEnergyIdle_watt_sec =  // @012.2
                    			( 0.001 * stpPowerParameter->stPowerIdleModeSystemCfg.fHDD_idle_1_inactive_wait_ms *
                    			  ( fSpindlePowerPrevZone
                    			  + stpPowerParameter->stPowerHeadRW_w.fHeadServoPower
                    			  + fVCM_BiasPowerPrevZone
                    			  )
                    			)
                    			  +
                    			  (0.001 * stpPowerParameter->stPowerIdleModeSystemCfg.fHDD_idle_2_inactive_wait_ms *
                    					  (fSpindlePowerPrevZone
                    	            			  + stpPowerParameter->stPowerHeadRW_w.fHeadServoPower
                    	            			  + fVCM_BiasPowerPrevZone
                    	            			  + stpPowerParameter->stPowerIdleModeSystemCfg.fHDD_idle_1_power))
       	            			  +
       	            			  (0.001 * stpPowerParameter->stPowerIdleModeSystemCfg.fHDD_idle_3_inactive_wait_ms *
       	            					  (fSpindlePowerPrevZone
       	            							  + fVCM_BiasPowerPrevZone
       	            							  + stpPowerParameter->stPowerIdleModeSystemCfg.fHDD_idle_2_power)
                    	            			                  	        		  )
                    	          +
                    	          (0.001 * tIdle_m_1n2n3 *
                    	        		  (fSpindlePowerZoneId + stpPowerParameter->stPowerIdleModeSystemCfg.fHDD_idle_3_power));

        			}
        			else
        			{
        				/// Active: fHDD_idle_1_inactive_wait_ms
        				/// Idle-1: fHDD_idle_2_inactive_wait_ms
        				/// Idle-2: fHDD_idle_3_inactive_wait_ms
        				/// Idle-3: fHDD_idle_4_standby_timer_ms
        				/// Stand-by: tIdle_m_1n2n3nStandby
        				nFlagIdleMode = 4;
        				dEnergyIdle_watt_sec =  // @012.2
        						( 0.001 * stpPowerParameter->stPowerIdleModeSystemCfg.fHDD_idle_1_inactive_wait_ms *
        						                    			  ( fSpindlePowerPrevZone
        						                    			  + stpPowerParameter->stPowerHeadRW_w.fHeadServoPower
        						                    			  + fVCM_BiasPowerPrevZone
        						                    			  )
        						    )
        						    +
        						 (0.001 * stpPowerParameter->stPowerIdleModeSystemCfg.fHDD_idle_2_inactive_wait_ms *
        								 (fSpindlePowerPrevZone
        										 + stpPowerParameter->stPowerHeadRW_w.fHeadServoPower
        										 + fVCM_BiasPowerPrevZone
        										 + stpPowerParameter->stPowerIdleModeSystemCfg.fHDD_idle_1_power)
        						 )
        						 +
        						 (0.001 * stpPowerParameter->stPowerIdleModeSystemCfg.fHDD_idle_3_inactive_wait_ms *
        								 (fSpindlePowerPrevZone
        										 + fVCM_BiasPowerPrevZone
        										 + stpPowerParameter->stPowerIdleModeSystemCfg.fHDD_idle_2_power)
        						  )
        						    +
       						    (0.001 * stpPowerParameter->stPowerIdleModeSystemCfg.fHDD_standby_recover_time_to_active *
        						    		(fSpindlePowerZoneId + stpPowerParameter->stPowerIdleModeSystemCfg.fHDD_idle_3_power)
        						    		)
        						    +(0.001 * tIdle_m_1n2n3nStandby
        						    		* stpPowerParameter->stPowerIdleModeSystemCfg.fSpindleIdlePower_w
        						    		);
        			}
        		}
        	}

        }
        stpOutputCalcHDD->dEnergyIdle_watt_sec = dEnergyIdle_watt_sec;  // @012.2
        stpOutputCalcHDD->nFlagIdleMode = nFlagIdleMode ;

#endif // @012

__label_hdd_model_calculate_access_time:
return iRet;
}
