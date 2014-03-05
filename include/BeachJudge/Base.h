#ifndef _CE_BASE_H_
#define _CE_BASE_H_

//- Standard Library -
#include <string>

//- Beach Judge -
#include <BeachJudge/Config.h>

namespace beachjudge
{
	//- Time -
	void sleepMS(unsigned long timeMS);
	unsigned long getRunTimeMS();
	unsigned long getRealTimeMS();
	unsigned long getStartTimeMS();

	//- File IO -
	bool fileExists(const char *file);
	bool fileDelete(const char *file);
	std::string fileExt(const char *file);
	std::string filePath(const char *file);
	std::string fileCompressPath(const char *file);
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
