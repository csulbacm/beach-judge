#ifndef _BEACHJUDGE_HTTP_H_
#define _BEACHJUDGE_HTTP_H_

//- Standard Library -
#include <string>
#include <sstream>

//- Beach Judge -
#include <BeachJudge/Socket.h>

namespace beachjudge
{
	class HTTP
	{
	public:
		static void OpenHeader_OK(std::stringstream &stream);
		static void OpenHeader_NotFound(std::stringstream &stream);
		static void SetSessionCookie(std::stringstream &stream, std::string target, std::string value);
		static void CloseHeader(std::stringstream &stream);
		static void LoadImage(std::stringstream &stream, std::string file);

		static void HandleRequest(Socket *client, std::string &request);
	};
}

#endif
