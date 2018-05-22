/*
 * main.cpp
 *
 *  Created on: 2018年5月18日
 *      Author: wen
 */
#define _CRT_SECURE_NO_WARNINGS  
#include <iostream>
#include <fstream>
#include <io.h>
#include <string.h>
#include <errno.h>
#include <map>
#include "LOG.h"
#include "net.h"
#include "MyMd5.h"
#include "Entry.h"
#include "json/json.h"

using namespace std;
bool fetchFeed(const char* feedPath, const char* filePath);
void loopSend(const char* path);
void sendCommand(const char* currentFile,string revUser);
string getAppPath();
//global
LOG myLog;
map<string,string> sig;
Entry listOfEntry[10];
int COUNT;

int listSize;

const char* in_path;
string fileDir;


string trim(string s)
{
	string str;
	int index = 0;
	if (!s.empty())
	{
		while ((index = s.find(' ', index)) != string::npos)
		{
			s.erase(index, 1);
		}
	}
	str = s;
	return str;
}

int main(){
	ifstream pFile,listFile,pathFile;
	string key, value;
	string p,str;

	Json::Reader reader;
	Json::Value root;


	fileDir = getAppPath();

	myLog.open((fileDir+"log.txt").c_str());
	//读取配置文件
	pathFile.open(fileDir + "path.conf");
	if (!pathFile.is_open()){
		myLog.write("打开路径文件错误");
		exit(-1);
	}
	getline(pathFile, str);
	in_path = str.c_str();
	listFile.open(fileDir + "entry.json");
	if (!listFile.is_open()){
		myLog.write("打开列表文件错误");
		exit(-1);
	}
	if (!reader.parse(listFile, root)){
		myLog.write("遍历json文件出错");
		exit(-1);
	} 
	COUNT = root.size();
	listSize = COUNT;
	while (listSize--){
		listOfEntry[listSize].book = root[listSize]["book"].asCString();
		listOfEntry[listSize].recipe = root[listSize]["recipe"].asCString();
		listOfEntry[listSize].recver = root[listSize]["recver"].asCString();
		listOfEntry[listSize].seed_path = root[listSize]["seed_path"].asCString();
	}


	//检查服务器是否可用
	if (testIP("192.168.99.100", "3000") == false){
		myLog.write("服务器不可用，退出程序");
		exit(-1);
	}


	//检查RSS文件是否更新并转换格式
	pFile.open(fileDir + "sig.txt");
	if (!pFile.is_open())
		myLog.write("打开签名文件错误");
	else{
		while (pFile.peek() != EOF){
			pFile >> key;
			pFile >> value;
			sig[key] = value;
		}
	}
	listSize = COUNT;
	while (listSize--){
		listOfEntry[listSize].is_fetch = true;
		const char* newSig = MD5_file((char*)listOfEntry[listSize].seed_path, 32, 0);
		if (newSig == NULL){
			myLog.write("在验证"+(string)listOfEntry[listSize].seed_path+"时服务器无响应,本次运行不会抓取RSS信息和产生投递行为");
			listOfEntry[listSize].is_fetch = false;
			continue;
		}
		map<string, string>::iterator finder = sig.find((char*)listOfEntry[listSize].seed_path);
		if (finder != sig.end() && finder->second == newSig){
			myLog.write("\"" + (string)(listOfEntry[listSize].seed_path) + "\" 的内容没有更新，本次运行没有拉取RSS新闻\n*****************");
			listOfEntry[listSize].is_fetch = false;
			continue;
		}
		sig[(char*)listOfEntry[listSize].seed_path] = newSig;
		//获取RSS信息
		p=p.assign(in_path).append("\\").c_str();
		if(!fetchFeed((char*)listOfEntry[listSize].recipe, p.assign(in_path).append("\\").c_str()))
			listOfEntry[listSize].is_fetch = false;;
	}
	pFile.close();
	
	//重写sig文件
	ofstream oFile;
	oFile.open(fileDir + "sig.txt", ios::trunc);
	for (map<string, string>::iterator it = sig.begin(); it != sig.end(); it++){
		oFile << trim((string)it->first) << " " << it->second << endl;
	}
	oFile.close();
	
	myLog.write("准备投递文件...");
	loopSend(in_path);
	myLog.write("投递结束，退出程序...");
	myLog.close();
	return 0;
}

bool fetchFeed(const char* feedPath, const char* filePath){
	string commandLine = "ebook-convert \"";
	string name = ((string)feedPath).substr(((string)feedPath).rfind("\\") + 1, (int(((string)feedPath).rfind("."))- (int(((string)feedPath).rfind("\\"))))-1);
	myLog.write("*****************正在抓取"+name);
	system(((string)"date /t >> \"" + fileDir + (string)"\\feedLog.txt\"").c_str());
	system(((string)"time /t >> \"" + fileDir + (string)"\\feedLog.txt\"").c_str());
	commandLine += (string)feedPath + "\" \"" + filePath + name + ".mobi\" --authors \"vinny\" --tags \"微信公众号，新闻\"" + " >> \"" + fileDir+"feedLog.txt\"";
	if (system(commandLine.c_str()) != 0){
		myLog.write("抓取RSS出错\n*****************");
		return false;
	}
	else{
		myLog.write("抓取RSS成功\n*****************");
		return true;
	}
}

void loopSend(const char* path){
	long h_file = 0;
	struct _finddata_t fileInfo;
	string p;
	char temp_path[200];
	Entry currentE;

	if((h_file =_findfirst(p.assign(path).append("\\*").c_str(),&fileInfo)) !=-1){
		do{
			const char* currentFile = p.assign(path).append("\\").append(fileInfo.name).c_str();
			//如果是文件夹
			if((fileInfo.attrib  & _A_SUBDIR)){
				if(strcmp(fileInfo.name,".")!=0 && strcmp(fileInfo.name,"..")!=0){
					loopSend(currentFile);
				}
			}
			//否则
			else {
				listSize = COUNT; 
				while (listSize--){
					currentE = listOfEntry[listSize];
					strcpy(temp_path, (char*)path);
					if (currentE.is_fetch && strcmp(currentFile, (const char*)strcat(temp_path, (char*)currentE.book)) == 0){
						sendCommand(currentFile, currentE.recver);
					}
				}

			}
		}while(_findnext(h_file,&fileInfo)==0);
	}
}

void sendCommand(const char* currentFile,string revUser){

	myLog.write("检测到文件，尝试将 \"" + (string)currentFile + "\" 投递给" + revUser + "...");
	string commandLine = "calibre-smtp -a \"";
	commandLine += (string)currentFile + "\" -r smtp.163.com --port=465 -e SSL -u 18301757612@163.com -p qww201652yuan 18301757612@163.com " + revUser + " \' \'";
	
	if(system(commandLine.c_str())!=0){
		myLog.write("投递出错\n*****************");
	}
	else
		myLog.write("投递成功\n*****************"); 


}


string getAppPath(){
	TCHAR _szPath[MAX_PATH + 1] = { 0 };
	GetModuleFileName(NULL, _szPath, MAX_PATH);
	(_tcsrchr(_szPath, _T('\\')))[1] = 0;//删除文件名，只获得路径 字串  
	string strPath;
	for (int n = 0; _szPath[n]; n++)
	{
		if (_szPath[n] != _T('\\'))
		{
			strPath += _szPath[n];
		}
		else
		{
			strPath += _T("\\\\");
		}
	}
	return strPath;
}