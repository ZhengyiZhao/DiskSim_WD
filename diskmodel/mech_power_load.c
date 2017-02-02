//// disk power model (version 1.0)
//// Authors: Wan Jie, Zhengyi, XuJun
//// Contributors: Zhengyi
////
//// Copyright (c) of Werstern Digital, 2001-2018.
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
//// CARNEGIE MELLON UNIVERSITY MAKES NO WARRANTIES OF ANY KIND, EITHER
//// EXPRESSED OR IMPLIED AS TO THE MATTER INCLUDING, BUT NOT LIMITED
//// TO: WARRANTY OF FITNESS FOR PURPOSE OR MERCHANTABILITY, EXCLUSIVITY
//// OF RESULTS OR RESULTS OBTAINED FROM USE OF THIS SOFTWARE. Werstern
//// Digital DOES NOT MAKE ANY WARRANTY OF ANY KIND WITH
//// RESPECT TO FREEDOM FROM PATENT, TRADEMARK, OR COPYRIGHT
//// INFRINGEMENT.  COPYRIGHT HOLDERS WILL BEAR NO LIABILITY FOR ANY USE
//// OF THIS SOFTWARE OR DOCUMENTATION.

// Module: mech_power_load.c
// 
#include "modules/modules.h"
#include "dm_power.h"
// #include <math.h>
#include <libparam/libparam.h>
#include <libparam/bitvector.h>

#include <errno.h>
#include <stdio.h>

struct dm_power_if *
dm_mech_power_loadparams(struct lp_block *b, struct dm_disk_if *parent) {

	if (!b || !parent) {
		return NULL;
	}

  struct dm_mech_power *result = calloc(1, sizeof(struct dm_mech_power)); // *result
  if (result == NULL) {
	  return NULL;
  }
  result->hdr = mech_power;
  result->disk = parent;

  //  #include "modules/dm_mech_g1_param.c"
  lp_loadparams(result, b, &dm_mech_power_mod); // follow modules.h

  parent->stpPowerHDD = (struct dm_power_if *) result;
  return (uintptr_t) parent->stpPowerHDD; // (struct dm_power_if *) result;
}

void 
dm_mech_power_load_spindle_zone_curve(char *strFilename, 
				      int *pnTotalCnt,
				      int **aidxZone, 
				      double **afPowerSpindle
                                      )
{
   int rv, mat;
   int lineflag = 1;
   int iZoneCount = 0, nTotalZoneCount = 0, buflen = 128;
   int *aiLoadZoneId;
   double *afLoadPowerSpindle;
   FILE *fptrSpindlePowerZoneTable;
   char linebuf[1024];

   char *strFullPathname = lp_search_path(lp_cwd, strFilename);

   if(strFullPathname) {
     fptrSpindlePowerZoneTable = fopen(strFullPathname, "r");
   }
   else {
	 printf("mechanical power filename: %s", strFilename);
         ddbg_assert2(0, "Seek file not found in path!");
   }

   ddbg_assert3(fptrSpindlePowerZoneTable != 0, ("fopen fptrSpindlePowerZoneTable (%s) failed: %s", 
				strFilename,
				strerror(errno)));


   rv = (fgets(linebuf, sizeof(linebuf), fptrSpindlePowerZoneTable) != 0);

   mat = sscanf(linebuf, "Spindle power measured: %d\n", &nTotalZoneCount);
   if(mat == 1)
   {
	   buflen = nTotalZoneCount;
	   lineflag = 0;
   }

   aiLoadZoneId = calloc(buflen, sizeof(*aiLoadZoneId));
   afLoadPowerSpindle = calloc(buflen, sizeof(*afLoadPowerSpindle));
   
   iZoneCount = 0;
   do {
     int iZoneId;
     double fSpindlePowerAtZone;

     if(!lineflag) {
       rv = (fgets(linebuf, sizeof(linebuf), fptrSpindlePowerZoneTable) != 0);
     }
     else {
       lineflag = 0;
     }
     if(rv)
     {
//    	 iZoneCount ++;  // @012.2, DONOT increment here but below, thanks to Agun
    	 mat = sscanf(linebuf, "%d, %lf\n", &iZoneId, &fSpindlePowerAtZone);

    	 if(mat == 2)
    	 {
    		 if(iZoneCount >= buflen-1)
    		 {
    			 buflen *= 2;
    			 aiLoadZoneId = realloc(aiLoadZoneId, buflen * sizeof(*aiLoadZoneId));
    			 afLoadPowerSpindle = realloc(afLoadPowerSpindle, buflen * sizeof(*afLoadPowerSpindle));
    		 }
    		 aiLoadZoneId[iZoneCount] = iZoneId;
             afLoadPowerSpindle[iZoneCount] = fSpindlePowerAtZone;
        	 iZoneCount ++;  // @012.2
             nTotalZoneCount = iZoneCount;
    	 }
    	 else
    	 {
    		 fprintf(stderr, "*** bogus line in SpindlePowerZone loader (%s:%d): %s\n",  __FILE__, __LINE__, linebuf);
    	 }
     }
   } while(rv);


   fclose(fptrSpindlePowerZoneTable);
   *pnTotalCnt = nTotalZoneCount;
   *aidxZone = aiLoadZoneId;
   *afPowerSpindle = afLoadPowerSpindle;

}

// This file is to load the energy per VCM seek
// supporing dual-seek energy profile, both Wr (write) and Rd( Read)
void
dm_mech_power_load_vcm_seek_dual_energy_curve(char *strFilename,
                                      int *pnTotalCnt,
                                      int **aidxSeekDist,
                                      double **afPowerVCM_RdSeekEnergy,
                                      double **afPowerVCM_WrSeekEnergy,
                                      int *iFlagDualSeek
                                      )
{
	   int rv, mat;
	   int lineflag = 1;
	   int iSkDistCount = 0, nTotalSkDistCount = 0, buflen = 128;
	   int *aiLoadSkDistId;
	   double *afLoadPowerVCM_RdSeekEnergy;
	   double *afLoadPowerVCM_WrSeekEnergy;
	   FILE *fptrPowerVCM_SeekEnergyTable;
	   char linebuf[1024];
       double fPowerVCM_RdSeekEnergy, fPowerVCM_WrSeekEnergy;
       int iSkDistId;

	   char *strFullPathname = lp_search_path(lp_cwd, strFilename);

	   if(strFullPathname) {
	     fptrPowerVCM_SeekEnergyTable = fopen(strFullPathname, "r");
	   }
	   else {
		 printf("mechanical power filename: %s", strFilename);
	         ddbg_assert2(0, "File not found in path!");
	   }

	   ddbg_assert3(fptrPowerVCM_SeekEnergyTable != 0,
			   ("file open (%s) failed: %s, %s, %d",
					strFilename,
					__FILE__, __func__, __LINE__));


	   rv = (fgets(linebuf, sizeof(linebuf), fptrPowerVCM_SeekEnergyTable) != 0);

	   mat = sscanf(linebuf, "VCM seek energy measured: %d\n", &nTotalSkDistCount);
	   if(mat == 1)
	   {
		   buflen = nTotalSkDistCount;
		   lineflag = 0;
	   }

	   aiLoadSkDistId = calloc(buflen, sizeof(*aiLoadSkDistId));
	   afLoadPowerVCM_RdSeekEnergy = calloc(buflen, sizeof(*afLoadPowerVCM_RdSeekEnergy));
	   afLoadPowerVCM_WrSeekEnergy = calloc(buflen, sizeof(*afLoadPowerVCM_WrSeekEnergy));

	   iSkDistCount = 0;
	   do {

	     if(!lineflag) {
	       rv = (fgets(linebuf, sizeof(linebuf), fptrPowerVCM_SeekEnergyTable) != 0);
	     }
	     else {
	       lineflag = 0;
	     }
	     if(rv)
	     {
	    	 iSkDistCount ++;
	    	 mat = sscanf(linebuf, "%d, %lf, %lf\n", &iSkDistId,
	    			 &fPowerVCM_RdSeekEnergy, &fPowerVCM_WrSeekEnergy); //

	    	 if(mat == 3)
	    	 {
	    		 if(iSkDistCount >= buflen-1)
	    		 {
	    			 buflen *= 2;
	    			 aiLoadSkDistId = realloc(aiLoadSkDistId, buflen * sizeof(*aiLoadSkDistId));
	    			 afLoadPowerVCM_RdSeekEnergy = realloc(afLoadPowerVCM_RdSeekEnergy, buflen * sizeof(*afLoadPowerVCM_RdSeekEnergy));
	    			 afLoadPowerVCM_WrSeekEnergy = realloc(afLoadPowerVCM_WrSeekEnergy, buflen * sizeof(*afLoadPowerVCM_WrSeekEnergy));
	    		 }
	    		 aiLoadSkDistId[iSkDistCount] = iSkDistId;
	             afLoadPowerVCM_RdSeekEnergy[iSkDistCount] = fPowerVCM_RdSeekEnergy;
	             afLoadPowerVCM_WrSeekEnergy[iSkDistCount] = fPowerVCM_WrSeekEnergy;
	             nTotalSkDistCount = iSkDistCount;
	             *iFlagDualSeek = 1;
	    	 }
	    	 else if(mat == 2)
	    	 {
	    		 if(iSkDistCount >= buflen-1)
	    		 {
	    			 buflen *= 2;
	    			 aiLoadSkDistId = realloc(aiLoadSkDistId, buflen * sizeof(*aiLoadSkDistId));
	    			 afLoadPowerVCM_RdSeekEnergy = realloc(afLoadPowerVCM_RdSeekEnergy, buflen * sizeof(*afLoadPowerVCM_RdSeekEnergy));
	    			 afLoadPowerVCM_WrSeekEnergy = realloc(afLoadPowerVCM_WrSeekEnergy, buflen * sizeof(*afLoadPowerVCM_WrSeekEnergy));
	    		 }
	    		 aiLoadSkDistId[iSkDistCount] = iSkDistId;
	             afLoadPowerVCM_RdSeekEnergy[iSkDistCount] = fPowerVCM_RdSeekEnergy;
	             afLoadPowerVCM_WrSeekEnergy[iSkDistCount] = fPowerVCM_RdSeekEnergy;
	             nTotalSkDistCount = iSkDistCount;
	             *iFlagDualSeek = 0;
	    	 }
	    	 else
	    	 {
	    		 fprintf(stderr, "*** bogus line in %s loader (%s:%d): %s\n",  __func__, __FILE__, __LINE__, linebuf);
	    	 }
	     }
	   } while(rv);

	   fclose(fptrPowerVCM_SeekEnergyTable);
	   *pnTotalCnt = nTotalSkDistCount;
	   *aidxSeekDist = aiLoadSkDistId;
	   *afPowerVCM_RdSeekEnergy = afLoadPowerVCM_RdSeekEnergy;
	   *afPowerVCM_WrSeekEnergy = afLoadPowerVCM_WrSeekEnergy;
}

// This file is to load the energy per VCM seek
void 
dm_mech_power_load_vcm_seek_energy_curve(char *strFilename,
                                      int *pnTotalCnt,
                                      int **aidxSeekDist,
                                      double **afPowerVCM_SeekEnergy
                                      )
{
	   int rv, mat;
	   int lineflag = 1;
	   int iSkDistCount = 0, nTotalSkDistCount = 0, buflen = 128;
	   int *aiLoadSkDistId;
	   double *afLoadPowerVCM_SeekEnergy;
	   FILE *fptrPowerVCM_SeekEnergyTable;
	   char linebuf[1024];

	   char *strFullPathname = lp_search_path(lp_cwd, strFilename);

	   if(strFullPathname) {
	     fptrPowerVCM_SeekEnergyTable = fopen(strFullPathname, "r");
	   }
	   else {
		 printf("mechanical power filename: %s", strFilename);
	         ddbg_assert2(0, "File not found in path!");
	   }

	   ddbg_assert3(fptrPowerVCM_SeekEnergyTable != 0,
			   ("file open (%s) failed: %s, %s, %d",
					strFilename,
					__FILE__, __func__, __LINE__));


	   rv = (fgets(linebuf, sizeof(linebuf), fptrPowerVCM_SeekEnergyTable) != 0);

	   mat = sscanf(linebuf, "VCM seek energy measured: %d\n", &nTotalSkDistCount);
	   if(mat == 1)
	   {
		   buflen = nTotalSkDistCount;
		   lineflag = 0;
	   }

	   aiLoadSkDistId = calloc(buflen, sizeof(*aiLoadSkDistId));
	   afLoadPowerVCM_SeekEnergy = calloc(buflen, sizeof(*afLoadPowerVCM_SeekEnergy));

	   iSkDistCount = 0;
	   do {
	     int iSkDistId;
	     double fPowerVCM_SeekEnergy;

	     if(!lineflag) {
	       rv = (fgets(linebuf, sizeof(linebuf), fptrPowerVCM_SeekEnergyTable) != 0);
	     }
	     else {
	       lineflag = 0;
	     }
	     if(rv)
	     {
	    	 iSkDistCount ++;
	    	 mat = sscanf(linebuf, "%d, %f\n", &iSkDistId, &fPowerVCM_SeekEnergy);

	    	 if(mat == 2)
	    	 {
	    		 if(iSkDistCount >= buflen-1)
	    		 {
	    			 buflen *= 2;
	    			 aiLoadSkDistId = realloc(aiLoadSkDistId, buflen * sizeof(*aiLoadSkDistId));
	    			 afLoadPowerVCM_SeekEnergy = realloc(afLoadPowerVCM_SeekEnergy, buflen * sizeof(*afLoadPowerVCM_SeekEnergy));
	    		 }
	    		 aiLoadSkDistId[iSkDistCount] = iSkDistId;
	             afLoadPowerVCM_SeekEnergy[iSkDistCount] = fPowerVCM_SeekEnergy;
	             nTotalSkDistCount = iSkDistCount;
	    	 }
	    	 else
	    	 {
	    		 fprintf(stderr, "*** bogus line in %s loader (%s:%d): %s\n",  __func__, __FILE__, __LINE__, linebuf);
	    	 }
	     }
	   } while(rv);

	   fclose(fptrPowerVCM_SeekEnergyTable);
	   *pnTotalCnt = nTotalSkDistCount;
	   *aidxSeekDist = aiLoadSkDistId;
	   *afPowerVCM_SeekEnergy = afLoadPowerVCM_SeekEnergy;
}

// This is to load the VCM power for the bias-current, w.r.t. zone
void 
dm_mech_power_load_vcm_bias_current_zone(char *strFilename,
                                       int *pnTotalCnt,
                                       int **aidxZone,
                                       double **afPowerVCM_Bias
                                       )
{
	   int rv, mat;
	   int lineflag = 1;
	   int iZoneCount = 0, nTotalZoneCount = 0, buflen = 128;
	   int *aiLoadZoneId;
	   double *afLoadPowerVCM_Bias;
	   FILE *fptrPowerVCM_BiasZoneTable;
	   char linebuf[1024];

	   char *strFullPathname = lp_search_path(lp_cwd, strFilename);

	   if(strFullPathname) {
	     fptrPowerVCM_BiasZoneTable = fopen(strFullPathname, "r");
	   }
	   else {
		 printf("mechanical power filename: %s", strFilename);
	         ddbg_assert2(0, "File not found in path!");
	   }

	   ddbg_assert3(fptrPowerVCM_BiasZoneTable != 0,
			   ("file open (%s) failed: %s, %s, %d",
					strFilename,
					__FILE__, __func__, __LINE__));


	   rv = (fgets(linebuf, sizeof(linebuf), fptrPowerVCM_BiasZoneTable) != 0);

	   mat = sscanf(linebuf, "VCM Bias power measured: %d\n", &nTotalZoneCount);
	   if(mat == 1)
	   {
		   buflen = nTotalZoneCount;
		   lineflag = 0;
	   }

	   aiLoadZoneId = calloc(buflen, sizeof(*aiLoadZoneId));
	   afLoadPowerVCM_Bias = calloc(buflen, sizeof(*afLoadPowerVCM_Bias));

	   iZoneCount = 0;
	   do {
	     int iZoneId;
	     double fPowerVCM_BiasAtZone;

	     if(!lineflag) {
	       rv = (fgets(linebuf, sizeof(linebuf), fptrPowerVCM_BiasZoneTable) != 0);
	     }
	     else {
	       lineflag = 0;
	     }
	     if(rv)
	     {
	    	 iZoneCount ++;
	    	 mat = sscanf(linebuf, "%d, %lf\n", &iZoneId, &fPowerVCM_BiasAtZone);

	    	 if(mat == 2)
	    	 {
	    		 if(iZoneCount >= buflen-1)
	    		 {
	    			 buflen *= 2;
	    			 aiLoadZoneId = realloc(aiLoadZoneId, buflen * sizeof(*aiLoadZoneId));
	    			 afLoadPowerVCM_Bias = realloc(afLoadPowerVCM_Bias, buflen * sizeof(*afLoadPowerVCM_Bias));
	    		 }
	    		 aiLoadZoneId[iZoneCount] = iZoneId;
	             afLoadPowerVCM_Bias[iZoneCount] = fPowerVCM_BiasAtZone;
	             nTotalZoneCount = iZoneCount;
	    	 }
	    	 else
	    	 {
	    		 fprintf(stderr, "*** bogus line in %s loader (%s:%d): %s\n",  __func__, __FILE__, __LINE__, linebuf);
	    	 }
	     }
	   } while(rv);


	   fclose(fptrPowerVCM_BiasZoneTable);
	   *pnTotalCnt = nTotalZoneCount;
	   *aidxZone = aiLoadZoneId;
	   *afPowerVCM_Bias = afLoadPowerVCM_Bias;

}

double dm_spindle_power_g4(struct dm_disk_if *disk,
		int nZoneId)
{
	double fSpindlePower = 0.0;
	DISK_POWER_PARAMETER *stpPowerParameter = (DISK_POWER_PARAMETER *)disk->stpPowerHDD;

	double fP1,fP2;
	int z1, z2;
	int i;

    for(i = 0; i < stpPowerParameter->nTotalCntSpindlePowerAtZone; i++)
    {
      if(nZoneId == stpPowerParameter->aidxZoneSpindlePower[i]) {
    	  fSpindlePower = stpPowerParameter->afPowerSpindle[i];
    	  break;
      }
      // The computation here will also do linear extrapolation if
      // we're past the end.
      else if (nZoneId <= stpPowerParameter->aidxZoneSpindlePower[i]
		|| (i == stpPowerParameter->nTotalCntSpindlePowerAtZone - 1))
      {
			 fP1 = stpPowerParameter->afPowerSpindle[i-1];
			 fP2 = stpPowerParameter->afPowerSpindle[i];
			 z1 = stpPowerParameter->aidxZoneSpindlePower[i-1];
			 z2 = stpPowerParameter->aidxZoneSpindlePower[i];
	 // didn't find it exactly; do some interpolation

			 fSpindlePower = stpPowerParameter->afPowerSpindle[(i-1)];

		 if(fP1 > fP2) {
			 fSpindlePower -= 1.0 * (nZoneId - z1) * (fP1 - fP2) / (z2 - z1);
		 }
		 else {
			 fSpindlePower += 1.0 * (nZoneId - z1) * (fP2 - fP1) / (z2 - z1);
		 }
		 break;
      }
    }

	return fSpindlePower;
}

double dm_vcm_bias_power(struct dm_disk_if *disk,
		int nZoneId)
{
	double fBiasPowerVCM = 0.0;
	DISK_POWER_PARAMETER *stpPowerParameter = (DISK_POWER_PARAMETER *)disk->stpPowerHDD;

	double fP1,fP2;
	int z1, z2;
	int i;

    for(i = 0; i < stpPowerParameter->nTotalCntVoiceCoilBiasPowerAtZone; i++)
    {
      if(nZoneId == stpPowerParameter->aidxZoneVoiceCoilBiasPower[i]) {
    	  fBiasPowerVCM = stpPowerParameter->afPowerVCM_Bias[i];
    	  break;
      }
      // The computation here will also do linear extrapolation if
      // we're past the end.
      else if (nZoneId <= stpPowerParameter->aidxZoneVoiceCoilBiasPower[i]
		|| (i == stpPowerParameter->nTotalCntVoiceCoilBiasPowerAtZone - 1))
      {
			 fP1 = stpPowerParameter->afPowerVCM_Bias[i-1];
			 fP2 = stpPowerParameter->afPowerVCM_Bias[i];
			 z1 = stpPowerParameter->aidxZoneVoiceCoilBiasPower[i-1];
			 z2 = stpPowerParameter->aidxZoneVoiceCoilBiasPower[i];
	 // didn't find it exactly; do some interpolation

			 fBiasPowerVCM = stpPowerParameter->afPowerVCM_Bias[(i-1)];

		 if(fP1 > fP2) {
			 fBiasPowerVCM -= 1.0 * (nZoneId - z1) * (fP1 - fP2) / (z2 - z1);
		 }
		 else {
			 fBiasPowerVCM += 1.0 * (nZoneId - z1) * (fP2 - fP1) / (z2 - z1);
		 }
		 break;
      }
    }

	return fBiasPowerVCM;
}

double dm_vcm_seek_energy_g4(struct dm_disk_if *disk,
			   struct dm_mech_state *start_track,
			   struct dm_mech_state *end_track,
			   int rw)
{
	double fVoiceCoilSeekEnergy = 0.0;

	  int i;
	  int nCylDist = abs(end_track->cyl - start_track->cyl);
	  DISK_POWER_PARAMETER *stpPowerParameter = (DISK_POWER_PARAMETER *)disk->stpPowerHDD;

	  double fE1,fE2;
	  int d1, d2;

	   if(nCylDist) {
	     // linear search of extracted seek curve
	     // might want to do something faster
	     // binsearch -- d_i <= d < d_{i+1}


	     for(i = 0; i < stpPowerParameter->nTotalCntVoiceCoilSeekTable; i++) {
	       if(nCylDist == stpPowerParameter->aidxSeekCylinderDist[i]) {
	    // @4.1
	    	   if(stpPowerParameter->iFlagDualSeek == 0)
	    	   {
	    		   fVoiceCoilSeekEnergy = stpPowerParameter->afPowerVCM_RdSeekEnergy[i];
	    	   }
	    	   else
	    	   {
	    		   if(rw == 1) // 1: read, 0: write
	    		   {
	    			   fVoiceCoilSeekEnergy = stpPowerParameter->afPowerVCM_RdSeekEnergy[i];
	    		   }
	    		   else
	    		   {
	    			   fVoiceCoilSeekEnergy = stpPowerParameter->afPowerVCM_WrSeekEnergy[i];
	    		   }
	    	   }
		// @4.1
		 break;
	       }
	       // The computation here will also do linear extrapolation if
	       // we're past the end.
	       else if (nCylDist <= stpPowerParameter->aidxSeekCylinderDist[i]
			|| (i == stpPowerParameter->nTotalCntVoiceCoilSeekTable-1))
	       {
	    	   // @4.1
	    	   if(stpPowerParameter->iFlagDualSeek == 0)
	    	   {
				 fE1 = stpPowerParameter->afPowerVCM_RdSeekEnergy[i-1];
				 fE2 = stpPowerParameter->afPowerVCM_RdSeekEnergy[i];
				 d1 = stpPowerParameter->aidxSeekCylinderDist[i-1];
				 d2 = stpPowerParameter->aidxSeekCylinderDist[i];
	    	   }
	    	   else
	    	   {
	    		   if(rw == 1) // 1: read, 0: write
	    		   {
	    				 fE1 = stpPowerParameter->afPowerVCM_RdSeekEnergy[i-1];
	    				 fE2 = stpPowerParameter->afPowerVCM_RdSeekEnergy[i];
	    				 d1 = stpPowerParameter->aidxSeekCylinderDist[i-1];
	    				 d2 = stpPowerParameter->aidxSeekCylinderDist[i];
	    		   }
	    		   else // afPowerVCM_WrSeekEnergy
	    		   {
	    				 fE1 = stpPowerParameter->afPowerVCM_WrSeekEnergy[i-1];
	    				 fE2 = stpPowerParameter->afPowerVCM_WrSeekEnergy[i];
	    				 d1 = stpPowerParameter->aidxSeekCylinderDist[i-1];
	    				 d2 = stpPowerParameter->aidxSeekCylinderDist[i];
	    		   }
	    	   }
	    	   // @4.1
		 // didn't find it exactly; do some interpolation

		 fVoiceCoilSeekEnergy = stpPowerParameter->afPowerVCM_RdSeekEnergy[(i-1)];

		 if(fE1 > fE2) {
			 fVoiceCoilSeekEnergy -= 1.0 * (nCylDist - d1) * (fE1 - fE2) / (d2 - d1);
		 }
		 else {
			 fVoiceCoilSeekEnergy += 1.0 * (nCylDist - d1) * (fE2 - fE1) / (d2 - d1);
		 }
		 break;
	       }
	     }
	   }

	return fVoiceCoilSeekEnergy;
}

struct dm_power_if mech_power = {
		dm_spindle_power_g4,
		dm_vcm_seek_energy_g4,
		dm_vcm_bias_power
};
