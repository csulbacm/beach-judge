//- Standard Library -
#include <iostream>

//- Beach Judge -
#include <BeachJudge/Base.h>
#include <BeachJudge/Page.h>
#include <BeachJudge/Session.h>
#include <BeachJudge/HTTP.h>

using namespace std;
//\r\nSet-Cookie: BEACHJUDGESESSID=123456789
const char *header_OK = "HTTP/1.1 200 OK\r\nConnection: close\r\nContent-type: text/html\r\n\r\n";
const char *wwwPrefix = "../www/";

namespace beachjudge
{
	void HTTP::HandleRequest(Socket *client, std::string &request)
	{
		unsigned short port = 0;
		unsigned long addr = 0;
		client->GetPeerIP4Info(&addr, &port);

		unsigned char ip[4], *ipPtr = (unsigned char *)&addr;
		for(unsigned char a = 0; a < 4; a++)
		{
			ip[a] = *ipPtr & 0xFF;
			ipPtr++;
		}

		Session *session = Session::Create(addr, port, 5);
		stringstream stream(request);
		string method;
		stream >> method;
		print("[%d: %d %d] Receiving Msg: %d.%d.%d.%d:%d\r\n", getRunTimeMS(), session, session->GetID(), (unsigned short)ip[0], (unsigned short)ip[1], (unsigned short)ip[2], (unsigned short)ip[3], port);
		cout << request << endl;

		if(!method.compare("GET"))
		{
			string arguments;
			stream >> arguments;

			string in;
			while(stream >> in)
			{
				if(!in.compare("Cookie:"))
				{
					string cookie;
					while(getline(stream, cookie, '='))
					{
						string value;
						stream >> value;
						if(!value.empty())
							if(*value.rbegin() == ';')
								value = value.substr(0, value.size() - 1);

						if(!cookie.compare(" BEACHJUDGESESSID"))
						{
							print("Beach Judge Sess ID: %s\r\n", value.c_str());
						}
					}

					break;
				}
			}

			stringstream argStream(arguments);
			string arg, filePath = wwwPrefix;

			stringstream webPageStream;
			webPageStream << header_OK;

			getline(argStream, arg, '/');
			if(getline(argStream, arg, '/'))
			{
				string testPath = filePath;
				testPath.append(arg);
				if(fileExists(testPath.c_str()))
					filePath = testPath;
				else
				{
					testPath.append(".html");
					if(fileExists(testPath.c_str()))
						filePath = testPath;
					else
						filePath.append("404.html");
				}
			}
			else
				filePath.append("index.html");

			Page *index = Page::Create(filePath);
			index->AddToStream(webPageStream, client, session);
			string webPage = webPageStream.str();
			client->Write((char *)webPage.c_str(), webPage.length());
		}
		else if(!method.compare("POST"))
		{
			string arguments;
			stream >> arguments;

			string in;
			while(stream >> in)
			{
				if(!in.compare("Cookie:"))
				{
					string cookie;
					while(getline(stream, cookie, '='))
					{
						string value;
						stream >> value;
						if(!value.empty())
							if(*value.rbegin() == ';')
								value = value.substr(0, value.size() - 1);

						if(!cookie.compare(" BEACHJUDGESESSID"))
						{
							print("Beach Judge Sess ID: %s\r\n", value.c_str());
						}
						if(stream.peek() == '\r')
							break;
					}
					getline(stream, cookie);
					getline(stream, cookie);

					string postArgs;
					stream >> postArgs;

					string arg;
					stringstream argStream(postArgs);
					while(getline(argStream, arg, '='))
					{
						string val;
						getline(argStream, val, '&');
						cout << "ARG: " << arg << " = " << val << endl;
					}

					break;
				}
			}
		}
	}
	void HTTP::AppendHeader_OK(string &str)
	{
		str.append(header_OK);
	}
}
