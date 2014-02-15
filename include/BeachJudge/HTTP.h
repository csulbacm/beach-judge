#ifndef _BEACHJUDGE_HTTP_H_
#define _BEACHJUDGE_HTTP_H_

//- Standard Library -
#include <string>

//- Beach Judge -
#include <BeachJudge/Socket.h>

namespace beachjudge
{
	class HTTP
	{
	public:
		static void AppendHeader_OK(std::string &str);
		static void HandleRequest(Socket *client, std::string &request);
	};
}

#endif
