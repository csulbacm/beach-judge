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

	//- File IO -
	bool fileExists(const char *file);
	std::string fileExt(const char *file);
	std::string filePath(const char *file);
	std::string fileCompressPath(const char *file);

	//- Messages -
	void error(const char *format, ...);
	void print(const char *format, ...);

	//- Version -
	unsigned int getVersionMajor();
	unsigned int getVersionMinor();
	std::string getVersionString();
}

#endif
