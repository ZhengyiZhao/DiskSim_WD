
/* diskmodel (version 1.0)
 * Authors: John Bucy, Greg Ganger
 * Contributors: John Griffin, Jiri Schindler, Steve Schlosser
 *
 * Copyright (c) of Carnegie Mellon University, 2001-2008.
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



#include "dm.h"
#include "dm_power.h"
#include <libparam/libparam.h>
#include <libparam/bitvector.h>
#include <string.h>
#include <stdlib.h>

#include "modules/modules.h"
#include "modules/dm_disk_param.h"

#include "marshal.h"
// @011 for legacy support of g1 and g2 format
double dm_vcm_seek_energy_dummy(struct dm_disk_if *disk,
			   struct dm_mech_state *start_track,
			   struct dm_mech_state *end_track,
			   int rw)
{
	return 0.0;
}

double dm_vcm_bias_power_dummy(struct dm_disk_if *disk,
		int nZoneId)
{
	return 0.0;
}

double dm_spindle_power_dummy(struct dm_disk_if *disk,
		int nZoneId)
{
	return 0.0;
}

extern struct dm_power_if mech_power;


struct dm_disk_if *
dm_disk_loadparams(struct lp_block *b, int *junk)
{
  struct dm_disk_if *result = calloc(1, sizeof(*result));
  int ii;

  // @012 initialize memory modules
#define __DEF_MAX_CYLINDER_DIST__   500000

  DISK_POWER_PARAMETER *stpDiskPowerParameter;
  result->stpPowerHDD = calloc(1, sizeof(struct dm_power_if));
  stpDiskPowerParameter = (DISK_POWER_PARAMETER*) result->stpPowerHDD;
  stpDiskPowerParameter->nTotalCntVoiceCoilSeekTable = __DEF_MAX_CYLINDER_DIST__;
  stpDiskPowerParameter->aidxSeekCylinderDist =
		  calloc(__DEF_MAX_CYLINDER_DIST__, sizeof(int));
  for(ii=0; ii<__DEF_MAX_CYLINDER_DIST__; ii++)
  {
	  stpDiskPowerParameter->aidxSeekCylinderDist[ii] = ii;
  }
  stpDiskPowerParameter->afPowerVCM_RdSeekEnergy =
		  calloc(__DEF_MAX_CYLINDER_DIST__, sizeof(stpDiskPowerParameter->afPowerVCM_RdSeekEnergy));
  stpDiskPowerParameter->afPowerVCM_WrSeekEnergy =
		  calloc(__DEF_MAX_CYLINDER_DIST__, sizeof(stpDiskPowerParameter->afPowerVCM_WrSeekEnergy));
//  stpDiskPowerParameter->hdr = mech_power;
//  stpDiskPowerParameter->disk = result;
  result->stpPowerHDD->dm_spindle_power = dm_spindle_power_dummy;
  result->stpPowerHDD->dm_vcm_bias_power = dm_vcm_bias_power_dummy;
  result->stpPowerHDD->dm_vcm_seek_energy = dm_vcm_seek_energy_dummy;
  result->nTotal_wedges_per_track = 296; // Default for BongKok SMR, @019

  //#include "modules/dm_disk_param.c"
  lp_loadparams(result, b, &dm_disk_mod);


  if((result->iFlag_cmr_smr_combo & DEF_MEDIA_HD_CMR_BIT == DEF_MEDIA_HD_CMR_BIT) ||
		  result->iFlag_cmr_smr_combo == 0)
  {
	  result->cFlag_is_cmr = 1;
  }

  if(result->iFlag_cmr_smr_combo & DEF_MEDIA_HD_SMR_BIT)
  {
	  result->cFlag_is_smr = 1;
  }

  if(result->stpPowerHDD->dm_spindle_power == NULL)
  {
	  result->stpPowerHDD->dm_spindle_power = dm_spindle_power_dummy;
  }
  if(result->stpPowerHDD->dm_vcm_bias_power == NULL)
  {
	  result->stpPowerHDD->dm_vcm_bias_power = dm_vcm_bias_power_dummy;
  }
  if(result->stpPowerHDD->dm_vcm_seek_energy == NULL)
  {
	  result->stpPowerHDD->dm_vcm_seek_energy = dm_vcm_seek_energy_dummy;
  }

  return result;
}



