
/*
 * test.cpp
 *
 *  Created on: Apr 8, 2020
 *      Author: karsh
 */

#include "test.h"
#include "../header/msTimer.h"
#include <sys/time.h>

using namespace std;


//============ Benchmarking =====================
typedef unsigned long long timestamp_t;

static timestamp_t get_timestamp ()
{
  struct timeval now;
  gettimeofday (&now, NULL);
  return  now.tv_usec + (timestamp_t)now.tv_sec * 1000000;
}

static timestamp_t probe1 = 0;
static timestamp_t probe2 = 0;

static void placeProbe(uint8_t num){
	switch(num){
	case 1:
		probe2 = 0;
		probe1 = get_timestamp();
		break;
	case 2:
		probe2 = get_timestamp();
		cout<< "Diff time (us)="<<probe2-probe1<<endl;
		cout<< "Diff time (ms)="<<(probe2-probe1)/1000.0<<endl;
		break;
	default:
		break;
	}
}




//===================================================
void timeoutFunc(void* arg){
	//placeProbe(2);
	std::cout<<"timedout"<<std::endl;
	//placeProbe(1);

}

void msTimerUnitTest(){

	msTimer timer1(msTimer::ONE_SHOT,timeoutFunc,NULL);
	if(timer1.create()==false){
		cout<<"Fail to create"<<endl;
	}
	sleep(1);

	if(timer1.start(1000)==false){
		cout<<"Fail to start"<<endl;
    }
//	sleep(1);
//	cout<<"stopping"<<endl;
//	if(timer1.stop()==false){
//		cout<<"Fail to stop"<<endl;
//    }
//	placeProbe(1);
//	if(timer1.start(1000)==false){
//		cout<<"Fail to start"<<endl;
//    }
	while(1){
		sleep(1);
	}
}

























int StopWatchTest()
{
   //Namespace alias
   namespace sw = stopwatch;

   //Create and start a stopwatch
   sw::Stopwatch my_watch;
   my_watch.start();



   //Record laps
   for(std::size_t i = 1; i <= 500000; i++){

	  //doing some dummy activity at every count divisible by 10
      if( i%10 == 0){
         auto a= 5;
         auto b = a+7;
         a=  b+a;
      }

      //Record laps every count of divisible by 100000
      if( i % 100000 == 0){
    	  std::cout<<" recording laps at count ="<<i<<std::endl;
          my_watch.lap();
      }
   }




   //Get elapsed time..
   // .. in nanoseconds
   std::uint64_t elapsed_ns = my_watch.elapsed<sw::nanoseconds>();
   // .. in microseconds
   std::uint64_t elapsed_mus = my_watch.elapsed<sw::microseconds>();
   // .. in milliseconds (default template argument, therefore not needed)
   std::uint64_t elapsed_ms = my_watch.elapsed();
   // .. in seconds
   std::uint64_t elapsed_s = my_watch.elapsed<sw::seconds>();




   //Get lap times (Total time laps and all individual time laps)
   auto laps = my_watch.elapsed_laps();



   //Print to screen
   std::cout << "---------------" << std::endl;
   std::cout << elapsed_ns << " Total time lapsed in ns" << std::endl;
   std::cout << elapsed_mus << " Total time lapsed in mus" << std::endl;
   std::cout << elapsed_ms << " Total time lapsed in ms" << std::endl;
   std::cout << elapsed_s << " Total time lapsed in s" << std::endl;

   //Show individual laps
   std::cout << "---------------" << std::endl;
   std::cout << "Lap Times: " << sw::Stopwatch::show_times(my_watch.elapsed_laps<sw::mus>());


   return 0;
}





