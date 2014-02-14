//- Standard Library -
#include <cstdarg>
#include <cstdio>
#include <fstream>
#include <iostream>
#include <string>

#ifdef linux
	//- Linux -
	#include <unistd.h>
#endif

#ifdef _WIN32
	//- Windows Patches -
	#include <Windows.h>
	#define snprintf _snprintf_s
	#define vsnprintf _vsnprintf_s
#endif

//- Beach Judge -
#include <BeachJudge/Base.h>

using namespace std;

namespace beachjudge
{
	string compileMessage(const char *format, va_list ap)
	{
		char text[256];
		vsnprintf(text, 256, format, ap);
		return string(text);
	}

	bool fileExists(const char *file)
	{
		ifstream in(file, ifstream::in | ifstream::binary);
		bool exists = in.is_open();
		in.close();
		return exists;
	}
	string fileExt(const char *file)
	{
		string str(file);
		return str.substr(str.find_last_of(".") + 1);
	}
	string filePath(const char *file)
	{
		string str(file);
		int a,b,c;
		a = str.find_last_of("/");
		b = str.find_last_of("\\");
		c = a > b ? a : b;
		return str.substr(0, c + 1);
	}
	string fileCompressPath(const char *file)
	{
		string str(file);

		int idx = 0;
		while((idx = str.find("\\")) != string::npos)
			str = str.replace(idx, 1, "/");
		while((idx = str.find("/./")) != string::npos)
			str = str.replace(idx, 3, "/");
		return str;
	}

	void error(const char *format, ...)
	{
		va_list	ap;
		va_start(ap, format);
		string msg = compileMessage(format, ap);
		cerr << msg;
		va_end(ap);
	}
	void print(const char *format, ...)
	{
		va_list	ap;
		va_start(ap, format);
		string msg = compileMessage(format, ap);
		cout << msg;
		va_end(ap);
	}

	unsigned int getVersionMajor()
	{
		return BEACHJUDGE_VERSION_MAJOR;
	}
	unsigned int getVersionMinor()
	{
		return BEACHJUDGE_VERSION_MINOR;
	}
	string getVersionString()
	{
		char text[8];
		snprintf(text, 8, "%i.%.2i", BEACHJUDGE_VERSION_MAJOR, BEACHJUDGE_VERSION_MINOR);
		return string(text);
	}

	void sleepMS(unsigned long timeMS)
	{
		#ifdef linux
			usleep(timeMS * 1000);
		#endif

		#ifdef _WIN32
			Sleep(timeMS);
		#endif
	}
}
