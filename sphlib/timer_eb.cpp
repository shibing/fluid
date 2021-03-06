
#include <timer_eb.h>

#include <iostream>
#include <cstdio>
#include <cstdlib>

using namespace EB;

std::vector<EB::Timer*> EB::Timer::timeList;

//----------------------------------------------------------------------
Timer::Timer()
{
	static int const_count = 0; 
	if (!const_count) {
		timeList.resize(0); 
	}
	name = "";
	scale = 0.;
	count = 0;
	unit = "ms";
	t = 0.0; 
	t1 = 0;
	t2 = 0;

	nbCalls = 0;
	offset = 0;
	reset();
}
//----------------------------------------------------------------------
Timer::Timer(const char* name_, int offset, int nbCalls)
{
	name = name_;

	switch (CLOCKS_PER_SEC) {
	case 1000000:
		scale = 1000. / (float) CLOCKS_PER_SEC;
		break;
	case 1000:
		scale = 1. / (float) CLOCKS_PER_SEC;
		break;
	default:
        std::cerr << "Timer does handle this case\n";
		exit(0);
	}
	count = 0;
	unit = "ms";
	t = 0.0;
	t1 = 0;
	t2 = 0;

	this->nbCalls = nbCalls;
	this->offset = offset;
	timeList.push_back(this);
	reset();
}
//----------------------------------------------------------------------
Timer::Timer(const Timer& t)
{
	name = t.name;
	scale = t.scale;
	count = t.count;
	this->t = t.t;
	this->t1 = t.t1;
	this->t2 = t.t2;
	this->nbCalls = t.nbCalls;
	this->offset = t.offset;
	timeList.push_back(this);
	reset();
}
//----------------------------------------------------------------------
Timer::~Timer()
{
}
//----------------------------------------------------------------------
void Timer::reset()
{
	t = 0.0;
	t1 = clock();
	count = 0;
}
//----------------------------------------------------------------------
void Timer::begin()
{
	if (count < offset) {
		count++;
		return;
	}
	gettimeofday(&t_start, NULL);
	t1 = clock();
	t2 = 0.0;
	count++;
}
//----------------------------------------------------------------------
void Timer::end()
{
	if (count <= offset) return;

	gettimeofday(&t_end, NULL);
	double tt = (t_end.tv_sec - t_start.tv_sec) +
	     (t_end.tv_usec - t_start.tv_usec) * 1.e-6;
	t += 1000*tt;
	if (count == nbCalls) {
		print();
		reset();
	}
}

void Timer::set(float tt)
{
    count++;
    if (count <= offset) return;
    t += tt;
    if (count == nbCalls) {
        print();
        reset();
    }
}
//----------------------------------------------------------------------
void Timer::print(FILE* fd, int label_width)
{
	if (count <= 0) return;
	int real_count = count - offset;
	if (name.length() > label_width) { 
        fprintf(fd, "%-*.*s...  |  avg: %10.4f  |  tot: %10.4f  |  count=%6d\n", 
                label_width-3, label_width-3, 
                name.c_str(), t/real_count, t, real_count);
    } else {
	fprintf(fd, "%-*.*s  |  avg: %10.4f  |  tot: %10.4f  |  count=%6d\n", 
            label_width, label_width, 
            name.c_str(), t/real_count, t, real_count);
    }
}
//----------------------------------------------------------------------
void Timer::printReset()
{
	print();
	reset();
}
//----------------------------------------------------------------------
void Timer::printAll(FILE* fd, int label_width)
{
	fprintf(fd, "====================================\n"); 
	fprintf(fd, "Timers [All times in ms (1/1000 s)]: \n"); 		
	fprintf(fd, "====================================\n\n");     
	for (int i=0; i < timeList.size(); i++) {
		Timer& tim = *(timeList[i]);
		tim.print(fd, label_width);
	}
	fprintf(fd, "\nNOTE: only timers that have called Timer::start() are shown. \n");
	fprintf(fd, "      [A time of 0.0 may indicate the timer was not stopped.]\n"); 
	fprintf(fd, "====================================\n"); 
}
//----------------------------------------------------------------------
void Timer::writeAllToFile(std::string filename) 
{
    int label_width = 50; 
    for (int i = 0; i < timeList.size(); i++) {
        Timer& tim = *(timeList[i]); 
        if (tim.name.length() > label_width) {
            label_width = tim.name.length(); 
        }
    }

    FILE* fd = fopen(filename.c_str(), "w"); 
    printAll(fd, label_width); 
    fclose(fd); 
}
