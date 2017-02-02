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


#include <libparam/libparam.h>
#include <libddbg/libddbg.h>

#include <stdio.h>
#include <stdlib.h>

#include <sys/time.h>
#include <time.h>


//extern doTests(struct dm_disk_if *, int, char **);
//extern void testsUsage(void);
//extern int minargs;


struct timingbucket {
  double tot;
  int n;
  double start;
  char *name;
};

#define MAXBUCKETS 10
struct timingbucket buckets[MAXBUCKETS];
int buckets_len = 0;

int addBucket(char *name) {
  struct timingbucket *b;
  ddbg_assert(buckets_len+1 < MAXBUCKETS);
  b = &buckets[buckets_len];
  b->name = name;
  b->tot = 0.0;
  b->n = 0;

  buckets_len++;
}

double tv2d(struct timeval *tv) {
  double result = tv->tv_usec;
  result += tv->tv_sec * 1000000;
  return result;
}

double now(void) {
  struct timeval tv;
  gettimeofday(&tv, 0);
  return tv2d(&tv);
}

// one outstanding time per bucket
int startClock(int b) {
  buckets[b].start = now();
}

int stopClock(int b) {
  buckets[b].tot += now() - buckets[b].start;
  buckets[b].n++;
}

int printTimes(void) {
  int i;
  for(i = 0; i < buckets_len; i++) {
    struct timingbucket *b = &buckets[i];
    if(b->tot == 0)
    {
    	printf("NOT tested.");
    }
    else
    {
    	printf("%s: n %d mean %f\n", b->name, b->n, b->tot / b->n);
    }
  }
}
