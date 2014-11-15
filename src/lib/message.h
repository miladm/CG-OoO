/*******************************************************************************
 * message.h
 ******************************************************************************/
#ifndef _MESSAGE_H
#define _MESSAGE_H

#include <map>
#include <string>
#include <string.h>
#include <stdarg.h>
#include "list.h"

class message {
	public:
		message ();
		~message ();
		void simStep (const char* msg);
        void simEvent (const char* fmt, ...);

	private:
		void heading();

		int _simStepCnt;
};

extern message g_msg;

#endif /*_MESSAGE_H*/
