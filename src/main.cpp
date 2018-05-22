/*
 * main.cpp
 *
 *  Created on: 2018��5��18��
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
	//��ȡ�����ļ�
	pathFile.open(fileDir + "path.conf");
	if (!pathFile.is_open()){
		myLog.write("��·���ļ�����");
		exit(-1);
	}
	getline(pathFile, str);
	in_path = str.c_str();
	listFile.open(fileDir + "entry.json");
	if (!listFile.is_open()){
		myLog.write("���б��ļ�����");
		exit(-1);
	}
	if (!reader.parse(listFile, root)){
		myLog.write("����json�ļ�����");
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


	//���������Ƿ����
	if (testIP("192.168.99.100", "3000") == false){
		myLog.write("�����������ã��˳�����");
		exit(-1);
	}


	//���RSS�ļ��Ƿ���²�ת����ʽ
	pFile.open(fileDir + "sig.txt");
	if (!pFile.is_open())
		myLog.write("��ǩ���ļ�����");
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
			myLog.write("����֤"+(string)listOfEntry[listSize].seed_path+"ʱ����������Ӧ,�������в���ץȡRSS��Ϣ�Ͳ���Ͷ����Ϊ");
			listOfEntry[listSize].is_fetch = false;
			continue;
		}
		map<string, string>::iterator finder = sig.find((char*)listOfEntry[listSize].seed_path);
		if (finder != sig.end() && finder->second == newSig){
			myLog.write("\"" + (string)(listOfEntry[listSize].seed_path) + "\" ������û�и��£���������û����ȡRSS����\n*****************");
			listOfEntry[listSize].is_fetch = false;
			continue;
		}
		sig[(char*)listOfEntry[listSize].seed_path] = newSig;
		//��ȡRSS��Ϣ
		p=p.assign(in_path).append("\\").c_str();
		if(!fetchFeed((char*)listOfEntry[listSize].recipe, p.assign(in_path).append("\\").c_str()))
			listOfEntry[listSize].is_fetch = false;;
	}
	pFile.close();
	
	//��дsig�ļ�
	ofstream oFile;
	oFile.open(fileDir + "sig.txt", ios::trunc);
	for (map<string, string>::iterator it = sig.begin(); it != sig.end(); it++){
		oFile << trim((string)it->first) << " " << it->second << endl;
	}
	oFile.close();
	
	myLog.write("׼��Ͷ���ļ�...");
	loopSend(in_path);
	myLog.write("Ͷ�ݽ������˳�����...");
	myLog.close();
	return 0;
}

bool fetchFeed(const char* feedPath, const char* filePath){
	string commandLine = "ebook-convert \"";
	string name = ((string)feedPath).substr(((string)feedPath).rfind("\\") + 1, (int(((string)feedPath).rfind("."))- (int(((string)feedPath).rfind("\\"))))-1);
	myLog.write("*****************����ץȡ"+name);
	system(((string)"date /t >> \"" + fileDir + (string)"\\feedLog.txt\"").c_str());
	system(((string)"time /t >> \"" + fileDir + (string)"\\feedLog.txt\"").c_str());
	commandLine += (string)feedPath + "\" \"" + filePath + name + ".mobi\" --authors \"vinny\" --tags \"΢�Ź��ںţ�����\"" + " >> \"" + fileDir+"feedLog.txt\"";
	if (system(commandLine.c_str()) != 0){
		myLog.write("ץȡRSS����\n*****************");
		return false;
	}
	else{
		myLog.write("ץȡRSS�ɹ�\n*****************");
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
			//������ļ���
			if((fileInfo.attrib  & _A_SUBDIR)){
				if(strcmp(fileInfo.name,".")!=0 && strcmp(fileInfo.name,"..")!=0){
					loopSend(currentFile);
				}
			}
			//����
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

	myLog.write("��⵽�ļ������Խ� \"" + (string)currentFile + "\" Ͷ�ݸ�" + revUser + "...");
	string commandLine = "calibre-smtp -a \"";
	commandLine += (string)currentFile + "\" -r smtp.163.com --port=465 -e SSL -u 18301757612@163.com -p qww201652yuan 18301757612@163.com " + revUser + " \' \'";
	
	if(system(commandLine.c_str())!=0){
		myLog.write("Ͷ�ݳ���\n*****************");
	}
	else
		myLog.write("Ͷ�ݳɹ�\n*****************"); 


}


string getAppPath(){
	TCHAR _szPath[MAX_PATH + 1] = { 0 };
	GetModuleFileName(NULL, _szPath, MAX_PATH);
	(_tcsrchr(_szPath, _T('\\')))[1] = 0;//ɾ���ļ�����ֻ���·�� �ִ�  
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