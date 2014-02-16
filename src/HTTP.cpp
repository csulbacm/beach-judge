//- Standard Library -
#include <iostream>
#include <fstream>
#include <algorithm>

//- Beach Judge -
#include <BeachJudge/Base.h>
#include <BeachJudge/Page.h>
#include <BeachJudge/Session.h>
#include <BeachJudge/HTTP.h>

using namespace std;

const char *wwwPrefix = "../www/";

namespace beachjudge
{
	void HTTP::OpenHeader_OK(stringstream &stream)
	{
		stream << "HTTP/1.1 200 OK\r\nConnection: close\r\nContent-type: text/html\r\n";
	}
	void HTTP::OpenHeader_NotFound(std::stringstream &stream)
	{
		stream << "HTTP/1.1 404 Not Found\r\nConnection: close\r\nContent-type: text/html\r\n";
	}
	void HTTP::SetSessionCookie(std::stringstream &stream, std::string target, std::string value)
	{
		stream << "Set-Cookie: " << target << "=" << value << "\r\n";
	}
	void HTTP::LoadImage(stringstream &stream, string file)
	{
		ifstream inFile(file.c_str(), ios::binary);

		stream << "HTTP/1.1 200 OK\r\nConnection: close\r\nContent-type: */*\r\n";

		string img;

		while(!inFile.eof())
		{
			char c;
			inFile.get(c);
			img.push_back(c);
		}

		unsigned long size = img.size();
		stream << "Content-Length: " << size << "\r\n\r\n";
		
		for(unsigned long a = 0; a < size; a++)
			stream << img[a];

		inFile.close();
	}
	void HTTP::CloseHeader(stringstream &stream)
	{
		stream << "\r\n";
	}
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

		Session *session = Session::Lookup(addr);
//		Session *session = Session::Create(addr, port, 5);
		stringstream stream(request);
		string method;
		stream >> method;
//		print("[%d: %d %d] Receiving Msg: %d.%d.%d.%d:%d\r\n", getRunTimeMS(), session, session->GetID(), (unsigned short)ip[0], (unsigned short)ip[1], (unsigned short)ip[2], (unsigned short)ip[3], port);
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

			bool e404 = false;
			stringstream webPageStream;
			getline(argStream, arg, '/');
			if(getline(argStream, arg, '/'))
			{
				string testPath = filePath;
				testPath.append(arg);
				if(fileExists(testPath.c_str()))
				{
					filePath = testPath;
				}
				else
				{
					testPath.append(".html");
					if(fileExists(testPath.c_str()))
						filePath = testPath;
					else
					{
						filePath.append("404.html");
						e404 = true;
					}
				}
			}
			else
				filePath.append("index.html");

			unsigned short per = filePath.find_last_of('.');
			string type;
			if(per != string::npos)
				type = filePath.substr(per + 1);

			bool img = false;
			if(type.length())
			{
				transform(type.begin(), type.end(), type.begin(), ::tolower);
				if(!type.compare("jpg") || !type.compare("jpeg") || !type.compare("png") || !type.compare("bmp"))
					img = true;
			}

			if(img)
			{
				LoadImage(webPageStream, filePath);
				string response = webPageStream.str();
				client->Write((char *)response.c_str(), response.length());
			}
			else
			{
				if(e404)
					OpenHeader_NotFound(webPageStream);
				else
					OpenHeader_OK(webPageStream);
				CloseHeader(webPageStream);

				Page *index = Page::Create(filePath);
				index->AddToStream(webPageStream, client, session);
				string webPage = webPageStream.str();
				client->Write((char *)webPage.c_str(), webPage.length());
			}
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
}
