#ifndef _BEACHJUDGE_BASE_H_
#define _BEACHJUDGE_BASE_H_

//- Standard Library -
#include <string>

//- Beach Judge -
#include <BeachJudge/Config.h>

namespace beachjudge
{
	//- Time -
	void sleepMS(unsigned long timeMS);
	unsigned long long getRunTimeMS();
	unsigned long long getRealTimeMS();
	unsigned long long getStartTimeMS();

	//- File IO -
	bool fileExists(const char *file);
	bool fileDelete(const char *file);
	bool fileRename(const char *file, const char *target);
	std::string fileExt(const char *file);
	std::string filePath(const char *file);
	std::string fileCompressPath(const char *file);
	unsigned long fileSize(const char *file);
	bool createFolder(const char *folder);

	//- Messages -
	void error(const char *format, ...);
	void print(const char *format, ...);

	//- Version -
	unsigned int getVersionMajor();
	unsigned int getVersionMinor();
	std::string getVersionString();
}

#endif
