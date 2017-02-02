/* diskmodel (version 1.1)
 * Authors: John Bucy, Greg Ganger
 * Contributors: John Griffin, Jiri Schindler, Steve Schlosser
 *
 * Copyright (c) of Carnegie Mellon University, 2003-2005
 *
 * This software is being provided by the copyright holders under the
 * following license. By obtaining, using and/or copying this
 * software, you agree that you have read, understood, and will comply
 * with the following terms and conditions:
 *
 * Permission to reproduce, use, and prepare derivative works of this
 * software is granted provided the copyright and "No Warranty"
 * statements are included with all reproductions and derivative works
 * and associated documentation. This software may also be
 * redistributed without charge provided that the copyright and "No
 * Warranty" statements are included in all redistributions.
 *
 * NO WARRANTY. THIS SOFTWARE IS FURNISHED ON AN "AS IS" BASIS.
 * CARNEGIE MELLON UNIVERSITY MAKES NO WARRANTIES OF ANY KIND, EITHER
 * EXPRESSED OR IMPLIED AS TO THE MATTER INCLUDING, BUT NOT LIMITED
 * TO: WARRANTY OF FITNESS FOR PURPOSE OR MERCHANTABILITY, EXCLUSIVITY
 * OF RESULTS OR RESULTS OBTAINED FROM USE OF THIS SOFTWARE. CARNEGIE
 * MELLON UNIVERSITY DOES NOT MAKE ANY WARRANTY OF ANY KIND WITH
 * RESPECT TO FREEDOM FROM PATENT, TRADEMARK, OR COPYRIGHT
 * INFRINGEMENT.  COPYRIGHT HOLDERS WILL BEAR NO LIABILITY FOR ANY USE
 * OF THIS SOFTWARE OR DOCUMENTATION.  
 */


#include "test.h"
#include <diskmodel/dm.h>
#include <diskmodel/modules/modules.h>


void testsUsage(void) {
  fprintf(stderr, "usage: ./test_disksim_mechanical_model <model>\n");
}

#define DEBUG_MATCH_MAXTOR_TRACE
#ifdef DEBUG_MATCH_MAXTOR_TRACE
#define OFFSET_SIM_START_TIME  (32.66107)
#else
#define OFFSET_SIM_START_TIME  (0)
#endif

#include "hdd_model_calculator.h"

#include <math.h>
int test_hdd_model_one_lba(struct dm_disk_if *hdd)
{
	int iRet;
	  struct dm_pbn stTargetStartPBA, stFromPBA, stResultPBA, trkpbn = {0,0,0};
	  char strInputString[128];
	  LBA_TYPE iToLBA, iFromLBA=0;
	  unsigned int nBlkCount = 0, nFlagRW = 1;
	  int iFlagTargetLBA = 0;
	  uint64_t nSeekTime, nAccessTime, nXfterTime, nLatency;
	  double tSeekTime_ms, tAccessTime_ms, tXfterTime_ms, tLatency_ms, tCompletionTime_ms;
	  dm_ptol_result_t rc;
	  LBA_TYPE firstLBN_track=0, lastLBN_track=0;
	  dm_angle_t skew, skewFrom;

	  INPUT_HDD_CALCULATE_ACCESS_TIME stInputCalcHDD;
	  OUTPUT_HDD_CALCULATE_ACCESS_TIME stOutputCalcHDD;

	  tCompletionTime_ms = OFFSET_SIM_START_TIME;

	  printf("Total Number Sectors (MaxLBN): %15.0f\n", (double)hdd->dm_sectors);
	  printf("Input a LBN: [0, %15.0f), -1: for further test or q for quit: ", (double)hdd->dm_sectors);
	  fflush(stdout);
	  scanf("%s", strInputString);
	  while(strInputString[0] != 'q' && strInputString[0] != '-')
	  {

	     printf("You have input LBN: %s\n", strInputString);
	     if(iFlagTargetLBA == 1)
	     {
	    	iFromLBA =  iToLBA + nBlkCount;
	    	//stFromPBA = stResultPBA; // stTargetStartPBA;
	    	rc = hdd->layout->dm_translate_ltop(hdd, iFromLBA, MAP_FULL, &stFromPBA, 0);
	    	skewFrom = hdd->layout->dm_pbn_skew(hdd, &stFromPBA);
	        sscanf(strInputString, "%ld, %d, %d", &iToLBA, &nBlkCount, &nFlagRW);
	        nFlagRW = nFlagRW & DISKSIM_FLAG_BIT_RW;
	     }
	     else
	     {
	        sscanf(strInputString, "%ld", &iToLBA);
	     }
	     rc = hdd->layout->dm_translate_ltop(hdd, iToLBA, MAP_FULL, &stTargetStartPBA, 0);
	     skew = hdd->layout->dm_pbn_skew(hdd, &stTargetStartPBA);


	     printf("Target: <Cyl, Head, Sector, skew_t, skew_d> = <%d, %d, %d, %u, %f>\n",
	    		 stTargetStartPBA.cyl, stTargetStartPBA.head, stTargetStartPBA.sector,
	    		 (unsigned int)skew, dm_angle_itod(skew));

	     hdd->layout->dm_get_track_boundaries(hdd, &stTargetStartPBA, &firstLBN_track, &lastLBN_track, 0);
	     printf("<first, last> lbn / total sectors on track <%d, %d>: <%15.0f, %15.0f>/%d\n",
	    		 stTargetStartPBA.cyl, stTargetStartPBA.head, (double)firstLBN_track, (double)lastLBN_track,
	    		 (lastLBN_track - firstLBN_track + 1));

	     //// additional calculation for target LBA
	     if(iFlagTargetLBA == 1)
	     {
	    	 stInputCalcHDD.nStartLBA = iFromLBA;
	    	 stInputCalcHDD.nTargetLBA = iToLBA;
	    	 stInputCalcHDD.nLen = nBlkCount;
	    	 stInputCalcHDD.nFlag = nFlagRW;
	    	 stInputCalcHDD.tStartSeek_ms = tCompletionTime_ms; // stOutputCalcHDD.tAccessTime_ms;

	    	 hdd_model_calculate_access_time(hdd, &stInputCalcHDD, &stOutputCalcHDD);

	        tSeekTime_ms = stOutputCalcHDD.tSeek_ms;
	        tXfterTime_ms = stOutputCalcHDD.tTransfer_ms;
	        tLatency_ms = stOutputCalcHDD.tLatency_ms;
	        tAccessTime_ms = stOutputCalcHDD.tAccessTime_ms; // dm_time_itod(nAccessTime);
	        tCompletionTime_ms = stInputCalcHDD.tStartSeek_ms + tAccessTime_ms;
	        printf("From %15.0f <%d, %d, %d> to %15.0f <%d, %d, %d>, %c %d blocks\n",
	              (double)iFromLBA,  stFromPBA.cyl, stFromPBA.head, stFromPBA.sector,
	              (double)iToLBA, stTargetStartPBA.cyl, stTargetStartPBA.head, stTargetStartPBA.sector,
	              'R' * nFlagRW + 'W' * (1 - nFlagRW),
	              nBlkCount);
	        printf("Time<Sk, Latency, Xfer, TotalAccess, AdLtcy, Complete>: \n  %9.6f, %9.6f, %9.6f, %9.6f, %9.6f, %9.6f. SkewAfterSk: %9.6f\n",
	        		tSeekTime_ms, tLatency_ms, tXfterTime_ms, tAccessTime_ms,
	        		stOutputCalcHDD.dAdditionalLatency_ms, tCompletionTime_ms,
	        		stOutputCalcHDD.dSkewAfterSeek);
	        if(fabs(tAccessTime_ms - tLatency_ms - tXfterTime_ms - tSeekTime_ms -stOutputCalcHDD.dAdditionalLatency_ms) > 0.001) // Tolerance is 1.0 ns
	        {
	        	printf("Warning: Checksum error, Sum %9.6f != %9.6f, error:%9.6f\n",
	        			tAccessTime_ms, (tLatency_ms + tXfterTime_ms + tSeekTime_ms),
	        			fabs(tAccessTime_ms - tLatency_ms - tXfterTime_ms - tSeekTime_ms));
	        }
	     }
	     //// prompt to input next LBA,
	     printf("Input a target <LBA,BlkCount,FlagRW>: LBA in [0, %15.0f), R1W0. -1 for further test, or q for quit: ", (double)hdd->dm_sectors);
	     fflush(stdout);
	     scanf("%s", strInputString);
	     if(iFlagTargetLBA == 0)
	     {
	    	 iFlagTargetLBA = 1;
	     }

	  }

	iRet = strInputString[0];
	return iRet;
}

#include <stdio.h>

int test_hdd_model_trace_io(struct dm_disk_if *hdd)
{
int iRet;
static char strInputString[512], strOutputFilename[512];

	  printf("Input file name for ascii-trace: (-1 for quit):"); fflush(stdout);
	  scanf("%s", strInputString);
	  // Sirius_SPC1C_WCE5bsu.trace, Sirius_SPC1C_WCE55bsu.trace, Sirius_SPC1C_WCE65bsu.trace
	  sprintf(strOutputFilename, "%s_out.csv", strInputString);

	  while(!(strInputString[0] == '-' && strInputString[1] == '1'))
	  {
		  printf("Load filename:%s\n", strInputString);
		  FILE *fptrTrace, *fptrOutCsv, *fptrCsvNotes;
		  char strLine[201];
		  INPUT_HDD_CALCULATE_ACCESS_TIME stInputCalcHDD;
		  OUTPUT_HDD_CALCULATE_ACCESS_TIME stOutputCalcHDD;
		  int iDevNum;
		  double tLoadTime_ms;
		  stInputCalcHDD.nStartLBA = 0; stInputCalcHDD.tStartSeek_ms = 0;// Assume the starting LBA is 0
		  stOutputCalcHDD.tAccessTime_ms = 0;
		  fptrTrace = fopen(strInputString, "r");
		  if(!fptrTrace)
		  {
			  fprintf(stderr, "!!! error, failed to open trace file\n");
			  goto __label_next_round_load_trace;
		  }
		  fptrOutCsv = fopen(strOutputFilename, "w");
		  if(!fptrOutCsv) fprintf(stderr,"!!! error, failed to open output\n");
		  else
		  {
			   while(!feof(fptrTrace))
			  {
				  fgets(strLine, 200, fptrTrace);
				  LBA_TYPE dLBA64b = 0;
				  sscanf(strLine, "%lf %d %ld %d %x\n",  &(tLoadTime_ms), &iDevNum,
						  &(dLBA64b), &(stInputCalcHDD.nLen), &(stInputCalcHDD.nFlag));
				  stInputCalcHDD.nTargetLBA = (LBA_TYPE)dLBA64b;

				  // Actual time to Start Seek is the larger number of tLoadTime_ms
				  //    (stInputCalcHDD.tStartSeek_ms + stOutputCalcHDD.tAccessTime_ms)
				  // there is no buffer storing the traces
				  if(tLoadTime_ms >= (stInputCalcHDD.tStartSeek_ms + stOutputCalcHDD.tAccessTime_ms))
				  {
					  stInputCalcHDD.tStartSeek_ms = tLoadTime_ms;
				  }
				  else
				  {
					  stInputCalcHDD.tStartSeek_ms =
							  (stInputCalcHDD.tStartSeek_ms + stOutputCalcHDD.tAccessTime_ms);
				  }

				  /// call HDD calculator
				  iRet = hdd_model_calculate_access_time(hdd, &stInputCalcHDD, &stOutputCalcHDD);
				  if(iRet == CALC_OK)
				  {
					  fprintf(fptrOutCsv, "%9.6f, %9.6f, %15.0f, %15.0f, %10d, %d, %10.6f, %10.6f, %10.6f, %10.6f, %10.6f, %d, %d, %d, %15.0f, %15.0f, %d, %d, %d, %15.0f, %15.0f, %10.6f, %10.6f\n",
							  tLoadTime_ms, stInputCalcHDD.tStartSeek_ms,
							  (double)stInputCalcHDD.nStartLBA, (double)stInputCalcHDD.nTargetLBA,
							  stInputCalcHDD.nLen,
							  (DISKSIM_FLAG_BIT_RW & stInputCalcHDD.nFlag), // * 'R' + 'W' * (1 - (stInputCalcHDD.nFlag * DISKSIM_FLAG_BIT_RW)),
							  stOutputCalcHDD.tSeek_ms, stOutputCalcHDD.tLatency_ms,
							  stOutputCalcHDD.tTransfer_ms, stOutputCalcHDD.tAccessTime_ms, stOutputCalcHDD.dAdditionalLatency_ms,
							  stOutputCalcHDD.iFromCylinder, stOutputCalcHDD.iFromHead, stOutputCalcHDD.iFromSector, (double)stOutputCalcHDD.nFirstLBAonTrackFrom, (double)stOutputCalcHDD.nLastLBAonTrackFrom,
							  stOutputCalcHDD.iTargetCylinder, stOutputCalcHDD.iTargetHead, stOutputCalcHDD.iTargetSector, (double)stOutputCalcHDD.nFirstLBAonTrackTarget, (double)stOutputCalcHDD.nLastLBAonTrackTarget,
							  stOutputCalcHDD.dSkewAfterSeek, stOutputCalcHDD.dTargetSkew);
					  stInputCalcHDD.nStartLBA = stInputCalcHDD.nTargetLBA + stInputCalcHDD.nLen;
					  fflush(stdout);
					  fflush(fptrOutCsv);

				  }
				  else
				  {
					  printf("Error in calculating: Trace:%s, Line:%d, ErrorCode:%d\n",
							  strLine, stOutputCalcHDD.nErrorLine, stOutputCalcHDD.nFlagCalculationError);
					  fflush(stdout);
					  fprintf(fptrOutCsv,"Error in calculating: Trace:%s, Line:%d, ErrorCode:%d\n",
							  strLine, stOutputCalcHDD.nErrorLine, stOutputCalcHDD.nFlagCalculationError);
					  fclose(fptrOutCsv);
					  break;
				  }
			  }
		  }
			  fptrCsvNotes = fopen("hddCalculatorOutCsvNote.txt","w");
			  if(fptrCsvNotes)
			  {
				  fprintf(fptrCsvNotes, "TraceArrive_ms, StartSeek_ms, FromLBA,  TargetLBA,  Len,  R/W, SeekTime,  Latency,  Transfer, TotalAccess_ms, AdditionalLatency, FromCyl, FromHead, FromSector, 1stLBAFromTrack, LastLBAFromTrack, TargetCyl, TargetHead, TargetSector, 1stLBATargetTrack, lastLBATargetTrack, skewAfterSeek, skewTargetLBA\n");
			  }
			  fclose(fptrCsvNotes);

		  fclose(fptrOutCsv);fclose(fptrTrace);

__label_next_round_load_trace:
		  /// next round of trace - testing
		  printf("Input file name for ascii-trace: (-1 for quit):"); fflush(stdout);
		  scanf("%s", strInputString);
	  }

	  iRet = strInputString[0];
	  return iRet;
}

int test_hdd_model_power_trace_io(struct dm_disk_if *hdd)
{
int iRet;
static char strInputString[512], strOutputFilename[512];

	  printf("Disk Power: Input file name for ascii-trace: (-1 for quit):"); fflush(stdout);
	  scanf("%s", strInputString);
	  // Sirius_SPC1C_WCE5bsu.trace, Sirius_SPC1C_WCE55bsu.trace, Sirius_SPC1C_WCE65bsu.trace
	  sprintf(strOutputFilename, "%s_out.csv", strInputString);

	  while(!(strInputString[0] == '-' && strInputString[1] == '1'))
	  {
		  printf("Load filename:%s\n", strInputString);
		  FILE *fptrTrace, *fptrOutCsv, *fptrCsvNotes;
		  char strLine[201];
		  INPUT_HDD_CALCULATE_ACCESS_TIME stInputCalcHDD;
		  OUTPUT_HDD_CALCULATE_TIME_AND_ENERGY stOutputCalcHDD;
		  stInputCalcHDD.nRegLastZone = 0;
		  stInputCalcHDD.tRegEndSeekTime = 0;
		  stInputCalcHDD.tRegLastEndRW = 0;

		  int iDevNum;
		  double tLoadTime_ms;
		  stInputCalcHDD.nStartLBA = 0; stInputCalcHDD.tStartSeek_ms = 0;// Assume the starting LBA is 0
		  stOutputCalcHDD.tAccessTime_ms = 0;
		  fptrTrace = fopen(strInputString, "r");
		  if(!fptrTrace)
		  {
			  fprintf(stderr, "!!! error, failed to open trace file\n");
			  goto __label_next_round_load_trace;
		  }
		  fptrOutCsv = fopen(strOutputFilename, "w");
		  if(!fptrOutCsv) fprintf(stderr,"!!! error, failed to open output\n");
		  else
		  {
			   while(!feof(fptrTrace))
			  {
				  fgets(strLine, 200, fptrTrace);
				  LBA_TYPE dLBA64b = 0;
				  sscanf(strLine, "%lf %d %ld %d %x\n",  &(tLoadTime_ms), &iDevNum,
						  &(dLBA64b), &(stInputCalcHDD.nLen), &(stInputCalcHDD.nFlag));
				  stInputCalcHDD.nTargetLBA = (LBA_TYPE)dLBA64b;

				  // Actual time to Start Seek is the larger number of tLoadTime_ms
				  //    (stInputCalcHDD.tStartSeek_ms + stOutputCalcHDD.tAccessTime_ms)
				  // there is no buffer storing the traces
				  if(tLoadTime_ms >= (stInputCalcHDD.tStartSeek_ms + stOutputCalcHDD.tAccessTime_ms))
				  {
					  stInputCalcHDD.tStartSeek_ms = tLoadTime_ms;
				  }
				  else
				  {
					  stInputCalcHDD.tStartSeek_ms =
							  (stInputCalcHDD.tStartSeek_ms + stOutputCalcHDD.tAccessTime_ms);
				  }

				  /// call HDD calculator
				  iRet = hdd_model_calculate_access_time_and_power(hdd, &stInputCalcHDD, &stOutputCalcHDD);
				  if(iRet == CALC_OK)
				  {
					  fprintf(fptrOutCsv, "%9.6f, %9.6f, %15.0f, %15.0f, %10d, %d, %10.6f, %10.6f, %10.6f, %10.6f, %10.6f, %d, %d, %d, %15.0f, %15.0f, %d, %d, %d, %15.0f, %15.0f, %10.6f, %10.6f, %10.6f, %10.6f, %10.6f, %10.6f, %10.6f, %10.6f, %d\n",
							  tLoadTime_ms, stInputCalcHDD.tStartSeek_ms,
							  (double)stInputCalcHDD.nStartLBA, (double)stInputCalcHDD.nTargetLBA,
							  stInputCalcHDD.nLen,
							  (DISKSIM_FLAG_BIT_RW & stInputCalcHDD.nFlag), // * 'R' + 'W' * (1 - (stInputCalcHDD.nFlag * DISKSIM_FLAG_BIT_RW)),
							  stOutputCalcHDD.tSeek_ms, stOutputCalcHDD.tLatency_ms,
							  stOutputCalcHDD.tTransfer_ms, stOutputCalcHDD.tAccessTime_ms, stOutputCalcHDD.dAdditionalLatency_ms,
							  stOutputCalcHDD.iFromCylinder, stOutputCalcHDD.iFromHead, stOutputCalcHDD.iFromSector, (double)stOutputCalcHDD.nFirstLBAonTrackFrom, (double)stOutputCalcHDD.nLastLBAonTrackFrom,
							  stOutputCalcHDD.iTargetCylinder, stOutputCalcHDD.iTargetHead, stOutputCalcHDD.iTargetSector, (double)stOutputCalcHDD.nFirstLBAonTrackTarget, (double)stOutputCalcHDD.nLastLBAonTrackTarget,
							  stOutputCalcHDD.dSkewAfterSeek, stOutputCalcHDD.dTargetSkew,
							  stOutputCalcHDD.dEnergySpindle_watt_sec,
							  stOutputCalcHDD.dEnergyBiasVCM_watt_sec, stOutputCalcHDD.dEnergySeekVCM_watt_sec,
							  stOutputCalcHDD.dEnergyHeadReadWrite_watt_sec, stOutputCalcHDD.dEnergyHeadServo_watt_sec,
							  stOutputCalcHDD.dEnergyIdle_watt_sec, stOutputCalcHDD.nFlagIdleMode); // @012.2
					  stInputCalcHDD.nStartLBA = stInputCalcHDD.nTargetLBA + stInputCalcHDD.nLen;
					  fflush(stdout);
					  fflush(fptrOutCsv);

#ifdef __DISK_POWER__
					  stInputCalcHDD.nRegLastZone = stOutputCalcHDD.nTargetZone;
					  stInputCalcHDD.tRegEndSeekTime = stOutputCalcHDD.tRegEndSeekTime;
					  stInputCalcHDD.tRegLastEndRW = stOutputCalcHDD.tRegEndRW_Time;
#endif
				  }
				  else
				  {
					  printf("Error in calculating: Trace:%s, Line:%d, ErrorCode:%d\n",
							  strLine, stOutputCalcHDD.nErrorLine, stOutputCalcHDD.nFlagCalculationError);
					  fflush(stdout);
					  fprintf(fptrOutCsv,"Error in calculating: Trace:%s, Line:%d, ErrorCode:%d\n",
							  strLine, stOutputCalcHDD.nErrorLine, stOutputCalcHDD.nFlagCalculationError);
					  fclose(fptrOutCsv);
					  break;
				  }
			  }
		  }
			  fptrCsvNotes = fopen("hddCalculatorOutCsvNote.txt","w");
			  if(fptrCsvNotes)
			  {
				  fprintf(fptrCsvNotes, "TraceArrive_ms, StartSeek_ms, FromLBA,  TargetLBA,  Len,  R/W, SeekTime,  Latency,  Transfer, TotalAccess_ms, AdditionalLatency, FromCyl, FromHead, FromSector, 1stLBAFromTrack, LastLBAFromTrack, TargetCyl, TargetHead, TargetSector, 1stLBATargetTrack, lastLBATargetTrack, skewAfterSeek, skewTargetLBA, EnergySpindle, EnergyVCMBias, EnergySeek, EnergyRW, EnergyServo, EnergyIdle, IdleMode\n");
			  }
			  fclose(fptrCsvNotes);

		  fclose(fptrOutCsv);fclose(fptrTrace);

__label_next_round_load_trace:
		  /// next round of trace - testing
		  printf("Input file name for ascii-trace: (-1 for quit):"); fflush(stdout);
		  scanf("%s", strInputString);
	  }

	  iRet = strInputString[0];
	  return iRet;
}
int
test_get_zone_meta_data(struct dm_disk_if *hdd)
{
	int iRet;
	  struct dm_pbn stTargetStartPBA, stFromPBA, stResultPBA, trkpbn = {0,0,0};
	  char strInputString[128];
	  LBA_TYPE iToLBA, iFromLBA=0;
	  unsigned int nBlkCount = 0, nFlagRW = 1;
	  int iFlagTargetLBA = 0;
	  uint64_t nSeekTime, nAccessTime, nXfterTime, nLatency;
	  double tSeekTime_ms, tAccessTime_ms, tXfterTime_ms, tLatency_ms, tCompletionTime_ms;
	  dm_ptol_result_t rc;
	  LBA_TYPE firstLBN_track=0, lastLBN_track=0;
	  dm_angle_t skew, skewFrom;

	  INPUT_HDD_CALCULATE_ACCESS_TIME stInputCalcHDD;
	  OUTPUT_HDD_CALCULATE_ACCESS_TIME stOutputCalcHDD;

	  struct dm_layout_zone stZone_meta_data;
	  int nZone_id;

	  tCompletionTime_ms = OFFSET_SIM_START_TIME;

	  printf("Total Number Sectors (MaxLBN): %15.0f\n", (double)hdd->dm_sectors);
	  printf("Input a LBN: [0, %15.0f), -1: for further test or q for quit: ", (double)hdd->dm_sectors);
	  fflush(stdout);
	  scanf("%s", strInputString);
	  while(strInputString[0] != 'q' && strInputString[0] != '-')
	  {

	     printf("You have input LBN: %s\n", strInputString);
	     if(iFlagTargetLBA == 1)
	     {
	    	iFromLBA =  iToLBA + nBlkCount;
	    	//stFromPBA = stResultPBA; // stTargetStartPBA;
	    	rc = hdd->layout->dm_translate_ltop(hdd, iFromLBA, MAP_FULL, &stFromPBA, 0);
	    	skewFrom = hdd->layout->dm_pbn_skew(hdd, &stFromPBA);
	        sscanf(strInputString, "%ld, %d, %d", &iToLBA, &nBlkCount, &nFlagRW);
	        nFlagRW = nFlagRW & DISKSIM_FLAG_BIT_RW;
	     }
	     else
	     {
	        sscanf(strInputString, "%ld", &iToLBA);
	     }
	     rc = hdd->layout->dm_translate_ltop(hdd, iToLBA, MAP_FULL, &stTargetStartPBA, 0);
	     skew = hdd->layout->dm_pbn_skew(hdd, &stTargetStartPBA);
	     nZone_id = hdd->layout->dm_get_zone_meta_data_by_lbn(hdd, iToLBA, &stZone_meta_data);


	     printf("Zone_id: %d, lbn_<low, high>: <%ld, %ld>, cyl_<low, high>: <%d, %d> \n",
	    		 nZone_id, stZone_meta_data.lbn_low, stZone_meta_data.lbn_high,
	    		 stZone_meta_data.cyl_low, stZone_meta_data.cyl_high);
	     printf("Clusters per zone: %d, Spt_<od, id>: <%ld, %ld>, cyl_per_cluster_<od, id>: <%d, %d> \n",
	    		 stZone_meta_data.nCluster_per_zone,
	    		 stZone_meta_data.nSector_per_track_od, stZone_meta_data.nSector_per_track_id,
	    		 stZone_meta_data.nTrack_per_cluster_od, stZone_meta_data.nTrack_per_cluster_id);

	     printf("PBN: <Cyl, Head, Sector, skew_t, skew_d> = <%d, %d, %d, %u, %f>\n",
	    		 stTargetStartPBA.cyl, stTargetStartPBA.head, stTargetStartPBA.sector,
	    		 (unsigned int)skew, dm_angle_itod(skew));

	     hdd->layout->dm_get_track_boundaries(hdd, &stTargetStartPBA, &firstLBN_track, &lastLBN_track, 0);
	     printf("<first, last> lbn / total sectors on track <%d, %d>: <%15.0f, %15.0f>/%d\n",
	    		 stTargetStartPBA.cyl, stTargetStartPBA.head, (double)firstLBN_track, (double)lastLBN_track,
	    		 (lastLBN_track - firstLBN_track + 1));

	     //// prompt to input next LBA,
	     printf("Input a target <LBA,BlkCount,FlagRW>: LBA in [0, %15.0f), R1W0. -1 for further test, or q for quit: ", (double)hdd->dm_sectors);
	     fflush(stdout);
	     scanf("%s", strInputString);
	     if(iFlagTargetLBA == 0)
	     {
	    	 iFlagTargetLBA = 1;
	     }


	  }

	iRet = strInputString[0];
	return iRet;
}

int
test_cvt_abs_headcylinder(struct dm_disk_if *hdd)
{
int iRet, rc;
static char strInputString[512], strOutputFilename[512];
int64_t i64_cyl;
struct dm_pbn stPbn;
struct dm_smr_hdcyl_g4_pbn stOutputG4PBN_SMR;
LBA_TYPE firstLBN_track, lastLBN_track;
struct dm_pbn stVerifyPBA;
int nSlipsCount;

	  printf("Input a positive number for abs headcylinder: (-1 for quit):"); fflush(stdout);
	  scanf("%s", strInputString);
	  while(!(strInputString[0] == '-' && strInputString[1] == '1'))
	  {
		  printf("You have input: %s, \n", strInputString);
		  sscanf(strInputString, "%ld", &i64_cyl);
		  hdd->layout->dm_get_pbn_by_abs_cyl(hdd, i64_cyl, &stOutputG4PBN_SMR);

		  printf("Zone: %d, PatternTK:%d, Cluster:%d, TrackOffset:%d, \nHead: %d, Cylinder:%d, StartLBN:%ld\n",
				  stOutputG4PBN_SMR.nZoneId,  stOutputG4PBN_SMR.nIdxPatternTK,
				  stOutputG4PBN_SMR.nClusterOffset, stOutputG4PBN_SMR.nTrackOffset,
				  stOutputG4PBN_SMR.stPBN.head , stOutputG4PBN_SMR.stPBN.cyl, stOutputG4PBN_SMR.nStartLBA);

//		  nSlipsCount = hdd->layout->dm_get_slip_count_bef_lbn(hdd, stOutputG4PBN_SMR.nStartLBA);

		  rc = hdd->layout->dm_translate_ltop(hdd, stOutputG4PBN_SMR.nStartLBA, MAP_FULL, &stVerifyPBA, &nSlipsCount);
		  hdd->layout->dm_get_track_boundaries(hdd, &stOutputG4PBN_SMR.stPBN, &firstLBN_track, &lastLBN_track, 0);
		  printf("VerifyPBN: Cyl:%d, Head:%d, VerifyBoundaryLBAs: first:%ld, SlipCount:%d\n", stVerifyPBA.cyl, stVerifyPBA.head, firstLBN_track,
				  nSlipsCount);

		  /// Next round
		  printf("Input a positive number for abs headcylinder: (-1 for quit):"); fflush(stdout);
		  scanf("%s", strInputString);
	  }

return iRet;
}

int
layout_test_simple(struct dm_disk_if *hdd)
{
  int i, lbn, count = 0, runlbn = 0;
  int iRet = 0;

//  static char strInputString[512], strOutputFilename[512];

  // 1. (a) test the single LBA to PBA translation,
  //    (b) disk-access time from one LBA to another LBA
  iRet = test_hdd_model_one_lba(hdd);

  // 1.1. test function for zone meta data
  if(iRet == '-')
  {
	  iRet = test_get_zone_meta_data(hdd);
  }

  // 2. test loading from a trace file and performs hdd model calculator
  if(iRet == '-')
  {
	  iRet = test_hdd_model_trace_io(hdd);
  }

  // 3. test function for Disk Power
  if(iRet == '-')
  {
	  iRet = test_hdd_model_power_trace_io(hdd);
  }

  // 4. test function for SMR head-cylinder to
  if(iRet == '-')
  {
	  iRet = test_cvt_abs_headcylinder(hdd);
  }
  printf("Above calculation is abstracted by zhengyi.zhao@wdc.com, from DiskSim \n DiskSim's Authors: Greg Ganger, John Bucy and etc.\n");

  return iRet;
}

int minargs = 0;
// struct dm_disk_if *hdd,
int doTests(int argc, char **argv)
{
	  int c, iRet;

  FILE *modelfile;
  struct dm_disk_if *disk;
  char *modelname;

  //// Step-1, load the hdd model file,
  /// it is assumed that the seek file and layout file are located at the same folder as the model file
  modelfile = fopen(argv[1], "r");
  if(!modelfile) {
    fprintf(stderr, "*** error: failed to open \"%s\"\n", argv[1]);
  }

  /// (1-a)registration and memory allocation
  for(c = 0; c <= DM_MAX_MODULE; c++) {
    struct lp_mod *mod;

    if(c == DM_MOD_DISK) {
      mod = dm_mods[c];
    }
    else {
      mod = dm_mods[c];
    }

    lp_register_module(mod);
  }
  lp_init_typetbl();

  /// (1-b) load disk model parameters from the files
  lp_loadfile(modelfile, 0, 0, argv[1], 0, 0);
  fclose(modelfile);

  modelname = argc >= 3 ? argv[2] : 0;

  disk = (struct dm_disk_if *)lp_instantiate("foo", modelname);
  //  printf("*** got a dm_disk with %d sectors!\n", disk->dm_sectors);

  //// Step-2 perform the simple layout test,
  iRet = layout_test_simple(disk);

  return iRet;
}


int main(int argc, char **argv)
{
  int iRet;

  ddbg_assert_setfile(stderr); // badct

  if(argc < (minargs+2)) {
    testsUsage();
    exit(1);
  }

  //// Initialization for the driver.
  //addBucket("ltop()");
  //addBucket("ptol()");

  printf("*** %s starting\n", argv[0]);

  iRet = doTests( argc, argv); // disk,

  printf("*** %s finished.\n", argv[0]);


//  printTimes();

  exit(0);
  // NOTREACHED
  return 0;
}

