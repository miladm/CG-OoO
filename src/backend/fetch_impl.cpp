/*******************************************************************************
 * fetch_impl.cpp
 *******************************************************************************/

#include <iostream>
#include "fetch_impl.h"

fetch_impl::fetch_impl() {
	_latency = 1; //default
}

fetch_impl::~fetch_impl() {}

CYCLE fetch_impl::tick(CYCLE) {
	return cycle+latency;
}
