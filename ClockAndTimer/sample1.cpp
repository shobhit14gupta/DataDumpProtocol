/*
 * sample1.c
 *
 *  Created on: Apr 8, 2020
 *      Author: karsh
 */


#include <unistd.h>
#include <time.h>
#include "stdlib.h"
#include "sample1.h"



using namespace std;

void CT_SamplePrg1()
{
	struct timespec     clock_resolution;
	int stat;

	stat = clock_getres(CLOCK_REALTIME, &clock_resolution);

	cout<<"Clock resolution is"<< clock_resolution.tv_sec <<" seconds, "<<clock_resolution.tv_nsec<<" nanoseconds\n"<<endl;
}


void CT_SamplePrg2()
{
struct timespec ts;

      /* Call time */
cout<<"time returns "<< time(NULL) <<" seconds\n"<< endl; ;
       /* Call clock_gettime */

clock_gettime(CLOCK_REALTIME, &ts);
cout<<"clock_gettime returns:\n"<<endl;
cout<<ts.tv_sec<<" seconds and "<<ts.tv_nsec<<" nanoseconds\n"<<endl;
}






