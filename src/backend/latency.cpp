#include "latency.h"
#include <stdio.h>
#include "../lib/list.h"

extern int cacheLat[MEM_HIGHERARCHY];

int getLatency(int hitLevel, int nonStdLat)
{
    int lat;
    switch ( hitLevel )
	{
		case 0:
			lat = nonStdLat;
			break;
		case 1:
			lat = cacheLat[0];
			break;
		case 2:
			lat = cacheLat[1];
			break;
		case 3:
			lat = cacheLat[2];
			break;
		case 4:
			lat = cacheLat[3];
			break;
		default:
			lat = -1;
			break;
	}

	Assert(lat != -1);
	if (lat <= cacheLat[0])
		L1hitCount();
	else if (lat <= cacheLat[1])
		L2hitCount();
	else if (lat <= cacheLat[2])
		L3hitCount();
	else if (lat <= cacheLat[3])
		MEMhitCount();

	missCount(lat);
	hitCount(lat);
	missLatency(lat);
	hitLatency(lat);
	totalLatency(lat);
	return lat;
}

int L1hitCount() {
	static int L1HitCounter = 0;
	L1HitCounter++;
	return L1HitCounter;
}
int L2hitCount() {
	static int L2HitCounter = 0;
	L2HitCounter++;
	return L2HitCounter;
}
int L3hitCount() {
	static int L3HitCounter = 0;
	L3HitCounter++;
	return L3HitCounter;
}
int MEMhitCount() {
	static int MemHitCounter = 0;
	MemHitCounter++;
	return MemHitCounter;
}


long int totalLatency (int lat) {
	static long int latency = 0;
	latency += lat;
	return latency;
}


long int missLatency (int lat) {
	static long int missLatency = 0;
	if (lat > cacheLat[0]) {missLatency += lat;}
	return missLatency;
}

long int hitLatency (int lat) {
	static long int hitLatency = 0;
	if (lat <= cacheLat[0]) {hitLatency += lat;}
	return hitLatency;
}


long int hitCount (int lat) {
	static long int hitCounter = 0;
	if (lat <= cacheLat[0]) {hitCounter++;}
	return hitCounter;
}

long int missCount (int lat) {
	static long int missCounter = 0;
	if (lat > cacheLat[0]) {missCounter++;}
	return missCounter;
}

