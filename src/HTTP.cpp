//- Beach Judge -
#include <BeachJudge/HTTP.h>
#include <BeachJudge/Base.h>

using namespace std;

const char *header_OK = "HTTP/1.1 200 OK\r\nConnection: close\r\nContent-type: text/html\r\n\r\n";

namespace beachjudge
{
	void HTTP::AppendHeader_OK(string &str)
	{
		str.append(header_OK);
	}
}
