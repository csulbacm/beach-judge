#ifndef _JUDGE_BASE_H_
#define _JUDGE_BASE_H_

#include <time.h>
#include <string.h>

// beachJudge
#include <Judge/Types.h>

namespace judge {

inline u64 getRealTimeMS()
{
	struct timespec t;
	clock_gettime(CLOCK_REALTIME, &t);
	return t.tv_sec * 1000 + t.tv_nsec / 1000000;
}

inline const char *getMimeType(const char *file)
{
	int n = strlen(file);
	if (n < 5)
		return 0;
	if (!strcmp(&file[n - 4], ".ico"))
		return "image/x-icon";
	if (!strcmp(&file[n - 4], ".png"))
		return "image/png";
	if (!strcmp(&file[n - 5], ".html"))
		return "text/html";
	if (!strcmp(&file[n - 4], ".css"))
		return "text/css";
	if (!strcmp(&file[n - 3], ".js"))
		return "text/javascript";
	return 0;
}

}

#endif
