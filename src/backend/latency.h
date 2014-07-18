/*******************************************************************************
 * latency.h computes the latency of each mem access
 ******************************************************************************/
#ifndef _LATENCY_H
#define _LATENCY_H
#include <stdint.h>
#include <stdlib.h>
#include "../global/global.h"


int getLatency(int hitLevel, int nonStdLat);
long int totalLatency(int lat);
long int missLatency (int lat);
long int hitLatency (int lat);
long int missCount (int lat);
long int hitCount (int lat);
int L1hitCount();
int L2hitCount();
int L3hitCount();
int MEMhitCount();


#endif
