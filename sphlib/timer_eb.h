#ifndef _TIME_EB_H_
#define _TIME_EB_H_

#include <sys/time.h>
#include <rtps_common.h>


#include <string>
#include <vector>
#include <stdio.h>
#include <map> 
#include <string> 

namespace EB {

class RTPS_EXPORT Timer
{
public:
	static std::vector<Timer*> timeList;

private:
	struct timeval t_start, t_end;

	double elapsed;
	float t;
	clock_t t1;
	clock_t t2;
	float scale;
	std::string name;
	std::string unit;
	int count;
	int nbCalls;
	int offset;

public:

	Timer();
	Timer(const char* name, int offset=0, int nbCalls=-1);
	Timer(const Timer&);
	~Timer();
	void reset();
	void begin();
	void end();
	int getCount() { return count;}

	void stop() { end(); }
	void start() { begin(); }

    void set(float t); //add a time from an external timer (GPU)

	static void printAll(FILE* fd=stdout, int label_width=50);
	void print(FILE* fd=stdout, int label_width=50);
    void writeAllToFile(std::string filename="timer_log"); 
	void printReset();
};



class RTPS_EXPORT TimerList : public std::map<std::string, EB::Timer*>
{
    public: 
    void writeToFile(std::string filename) {
        (*(this->begin())).second->writeAllToFile(filename); 
    } 
    void printAll() {
        (*(this->begin())).second->printAll(); 
    } 
};

};
#endif
