#pragma once
#include <string>
#include "cocos2d.h"
#include "math.h"
using std::string;
using cocos2d::CCLog;

inline const char* logTime(){
	time_t rawtime;
	struct tm * timeinfo;
	time ( &rawtime );
	timeinfo = localtime ( &rawtime );
	char* time_string =asctime(timeinfo);
	time_string[strlen(time_string)-1]=0;
	return time_string;
}

class LifeCircleLogger{
	string m_msg;
	clock_t tik;
	LifeCircleLogger(){}
public:
	LifeCircleLogger(const string& msg);
	~LifeCircleLogger();
};

#define LOG_FUNCTION_LIFE LifeCircleLogger __tmp____FUNCTION__(__FUNCTION__); 
