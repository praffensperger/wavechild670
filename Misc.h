/************************************************************************************
* 
* Wavechild670 v0.1 
* 
* Misc.h
* 
* By Peter Raffensperger 11 March 2014
* 
* Reference:
* Toward a Wave Digital Filter Model of the Fairchild 670 Limiter, Raffensperger, P. A., (2012). 
* Proc. of the 15th International Conference on Digital Audio Effects (DAFx-12), 
* York, UK, September 17-21, 2012.
* 
* Note:
* Fairchild (R) a registered trademark of Avid Technology, Inc., which is in no way associated or 
* affiliated with the author.
* 
* License:
* Wavechild670 is licensed under the GNU GPL v2 license. If you use this
* software in an academic context, we would appreciate it if you referenced the original
* paper.
* 
************************************************************************************/



#ifndef MISC_H
#define MISC_H


#include <math.h>
#include <stdio.h>
#include <string>
#include <sstream>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <iostream>
#include <map>
#include <vector>
#include <queue>
#include <algorithm>

#include <memory>
#include <utility>

#ifdef USE_BOOST
#include <boost/foreach.hpp>
#define foreach BOOST_FOREACH
#endif

using std::vector;
using std::string;
using std::ostringstream;
using namespace std;

typedef unsigned long u32;
typedef unsigned long ulong;
typedef unsigned char u8;
typedef unsigned int uint; //Must be >=32 bits

typedef double Real;

#define DBG_LOG_INNER_LOOP 6
#define DBG_LOG_SAMPLE2 5
#define DBG_LOG_SAMPLE1 4
#define DBG_LOG_INFO 3
#define DBG_LOG_WARNING 2
#define DBG_LOG_ERROR 1
#define DBG_LOG_CRITICAL 0
#define DEBUG_LOG_MESSAGE_LEVEL DBG_LOG_WARNING
#define DBG_LOG(level, msg) if(level <= DEBUG_LOG_MESSAGE_LEVEL) cout << msg << endl

#define LOG_INFO(msg) DBG_LOG(DBG_LOG_INFO,"INFO: " << msg)
#define LOG_WARNING(msg) DBG_LOG(DBG_LOG_WARNING,"WARNING: " << msg)
#define LOG_ERROR(msg) DBG_LOG(DBG_LOG_ERROR,"ERROR: " << msg)
#define LOG_CRITICAL(msg) DBG_LOG(DBG_LOG_CRITICAL,"CRITICAL ERROR: " << msg)
#define LOG_SAMPLE1(msg) DBG_LOG(DBG_LOG_SAMPLE1,msg)
#define LOG_SAMPLE2(msg) DBG_LOG(DBG_LOG_SAMPLE2,msg)
#define LOG_INNER_LOOP(msg) DBG_LOG(DBG_LOG_INNER_LOOP,msg)

#ifndef Assert

#ifndef DEBUG_MODE
#define DEBUG_MODE true
#endif

#if DEBUG_MODE

#define Confirm(cond) if(!(cond)) do_assert_failed(__FILE__, __LINE__)
#define Assert(cond) if(!(cond)) do_assert_failed(__FILE__, __LINE__)
#define Require(cond) if(!(cond)) do_assert_failed(__FILE__, __LINE__)

#else

#define Confirm(cond)
#define Assert(cond)
#define Require(cond)

#endif

#endif

#define loop(var, start, end) for(int var = start, endV = end; var < endV; ++var)

#define Failure() do_assert_failed(__FILE__, __LINE__)

void do_assert_failed(const char *file, int line);

template < class T >
string ToString(const T &arg){
	ostringstream out;
	out << arg;
	return(out.str());
}

#endif
