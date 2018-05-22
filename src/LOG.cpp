/*
 * LOG.cpp
 *
 *  Created on: 2018Äê5ÔÂ19ÈÕ
 *      Author: wen
 */
#define _CRT_SECURE_NO_WARNINGS  
#include "LOG.h"

LOG::LOG() {
	// TODO Auto-generated constructor stub

}

LOG::~LOG() {
	// TODO Auto-generated destructor stub
}

int LOG::open(const char* path){
	pFile.open(path, ios::app);
	if(!pFile.is_open())
		return -1;
	pFile << "--------------------" << endl;
	return 0;
}

void LOG::write(string str){
	time(&timer);
	struct tm * timeinfo;
	timeinfo = localtime(&timer);
	pFile <<  asctime(timeinfo) << ": " << str << endl;
}

void LOG::close(){
	pFile << "--------------------" << endl << endl;
	pFile.close();
}
