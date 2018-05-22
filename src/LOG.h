/*
 * LOG.h
 *
 *  Created on: 2018Äê5ÔÂ19ÈÕ
 *      Author: wen
 */

#ifndef LOG_H_
#define LOG_H_
#include <string>
#include <fstream>
#include <cTime>
#include <iostream>
using namespace std;

class LOG {
public:
	LOG();
	int open(const char* path);
	void write(string str);
	void close();
	virtual ~LOG();

private:
	ofstream pFile;
	time_t timer;

};

#endif /* LOG_H_ */
