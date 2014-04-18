//- Standard Library -
#include <cstdarg>
#include <cstdio>
#include <ctime>
#include <fstream>
#include <iostream>
#include <string>
#include <sstream>

#ifdef linux
	//- Linux -
	#include <unistd.h>
	#include <sys/time.h>
	#include <sys/stat.h>
#endif

#ifdef _WIN32
	//- Windows Patches -
	#include <Windows.h>
	#include <direct.h>
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
	bool fileDelete(const char *file)
	{
		return remove(file) == 0;
	}
	string fileExt(const char *file)
	{
		string str(file);
		return str.substr(str.find_last_of(".") + 1);
	}
	bool fileRename(const char *file, const char *target)
	{
		return rename(file, target) == 0;
	}
	unsigned long fileSize(const char *file)
	{
		ifstream inFile(file, ios::in | ios::binary);
		inFile.seekg(0, ifstream::end);
		return (unsigned long)inFile.tellg(); 
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

		size_t idx = 0;
		while((idx = str.find("\\")) != string::npos)
			str = str.replace(idx, 1, "/");
		while((idx = str.find("/./")) != string::npos)
			str = str.replace(idx, 3, "/");
		return str;
	}
	bool createFolder(const char *folder)
	{
		stringstream folderStream(folder);
		string curr, final;
		while(getline(folderStream, curr, '/'))
		{
			if(final.size())
				final.push_back('/');
			final.append(curr);
			#ifdef linux
				mkdir(final.c_str(), 0777);
			#endif
			#ifdef _WIN32
				_mkdir(final.c_str());
			#endif
		}

		return true;
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

	unsigned long long g_startTimeMS = getRunTimeMS(); //- TODO: Verify if we can get away with doing this -
	unsigned long long getStartTimeMS()
	{
		return g_startTimeMS;
	}
	unsigned long long getRealTimeMS()
	{
		unsigned long long currTimeMS;

		#ifdef linux
			struct timeval currTime;
			gettimeofday(&currTime, 0);
			currTimeMS = currTime.tv_sec * 1000 + currTime.tv_usec / 1000;
		#endif

		#ifdef _WIN32
			LARGE_INTEGER frequency;
			if(QueryPerformanceFrequency(&frequency))
			{
				LARGE_INTEGER count;
				QueryPerformanceCounter(&count);
				currTimeMS = (unsigned long long)((1000 * count.QuadPart) / frequency.QuadPart);
			}
			else
				currTimeMS = (unsigned long long)GetTickCount();
		#endif

		return currTimeMS;
	}
	unsigned long long getRunTimeMS()
	{
		return getRealTimeMS() - g_startTimeMS;
	}
}
