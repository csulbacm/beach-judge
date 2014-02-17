//- Standard Library -
#include <iostream>
#include <fstream>
#include <algorithm>
#include <cstring>

//- Beach Judge -
#include <BeachJudge/Base.h>
#include <BeachJudge/Page.h>
#include <BeachJudge/Session.h>
#include <BeachJudge/HTTP.h>
#include <BeachJudge/Team.h>

using namespace std;

const char *wwwPrefix = "../www";

namespace beachjudge
{
	void HTTP::OpenHeader_OK(stringstream &stream)
	{
		stream << "HTTP/1.1 200 OK\r\nConnection: close\r\nContent-type: text/html\r\n";
	}
	void HTTP::OpenHeader_OK_CSS(stringstream &stream)
	{
		stream << "HTTP/1.1 200 OK\r\nConnection: close\r\nContent-type: text/css\r\n";
	}
	void HTTP::OpenHeader_NotFound(std::stringstream &stream)
	{
		stream << "HTTP/1.1 404 Not Found\r\nConnection: close\r\nContent-type: text/html\r\n";
	}
	void HTTP::SetSessionCookie(std::stringstream &stream, std::string target, std::string value)
	{
		stream << "Set-Cookie: " << target << "=" << value << "; Path=/\r\n";
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
		if(request.size() <= 1)
			return;

		istringstream reqStream(request);

		unsigned short port = 0;
		unsigned long addr = 0;
		client->GetPeerIP4Info(&addr, &port);

		Session *session = Session::Lookup(addr);
		if(session)
			session->ResetTimeout();

		string str;

		//- URI Request -
		getline(reqStream, str);
		istringstream uriRequest(str);
		string method, uri, htmlVersion;
		uriRequest >> method >> uri >> htmlVersion;

		string file(wwwPrefix), arguments, type;

		bool e404 = false, loggingOut = false;

		{
			string uriCopy = uri;
			size_t argToken;
			argToken = uri.find_first_of('?');

			if(argToken != string::npos)
			{
				arguments = uriCopy.substr(argToken + 1);
				uriCopy = uriCopy.substr(0, argToken);
			}

			if(uriCopy.length() == 1)
				file.append("/index.html");
			else
			{
				if(!uriCopy.compare("/logout")) //- TODO: Find a better way to do this -
					loggingOut = true;

				string testPath = file;
				testPath.append(uriCopy);
				if(fileExists(testPath.c_str()))
					file = testPath;
				else
				{
					testPath.append(".html");
					if(fileExists(testPath.c_str()))
						file = testPath;
					else
					{
						file.append("/404.html");
						e404 = true;
					}
				}
			}

			size_t lastPeriod = file.find_last_of('.');
			if(lastPeriod != string::npos)
			{
				type = file.substr(lastPeriod + 1);
				transform(type.begin(), type.end(), type.begin(), ::tolower);
			}
		}

//		cout << request << endl;

		string line, contentType;
		unsigned long contentLength = 0;
		do
		{
			getline(reqStream, line);
			stringstream lineStream(line);

			string left, right;
			lineStream >> left;
			getline(lineStream, right);
			size_t rightLen = right.size();
			if(rightLen)
				if(right.at(rightLen - 1) == '\r')
					right = right.substr(1, rightLen - 2);
			if(!left.compare("Content-Type:"))
				contentType = right;
			else if(!left.compare("Content-Length:"))
				contentLength = atoi(right.c_str());
			else if(!left.compare("Cookie:"))
			{
				string cookie;
				stringstream cookieStream(right);
				while(getline(cookieStream, cookie, '='))
				{
					string val;
					getline(cookieStream, val, '&');

					if(!cookie.compare("BEACHJUDGESESSID"))
					{
						unsigned short sessID = atoi(val.c_str());
						//- TODO: Handle invalid cookies -
					}
				}
			}
		}
		while(line.size() > 1);

		if(contentLength)
			if(!method.compare("POST"))
			{
//				cout << "Content: " << contentType << " | " << contentLength << endl;
				if(!contentType.compare("application/x-www-form-urlencoded"))
				{
					string arg, args;
					map<string, string> argMap;
					getline(reqStream, args);
					stringstream argStream(args);
					while(getline(argStream, arg, '='))
					{
						string val;
						getline(argStream, val, '&');
						argMap[arg] = val;
					}
					if(argMap.count("cmd"))
					{
						if(!argMap["cmd"].compare("Login") && !session)
						{
							if(argMap.count("passwd"))
								if(argMap.count("team"))
								{
									Team *team = Team::LookupByName(argMap["team"]);
									if(team)
										if(team->TestPassword(argMap["passwd"]))
											session = Session::Create(addr, port, team);
								}
						}
						else if(!argMap["cmd"].compare("Change") && session)
						{
							if(argMap.count("curPasswd"))
								if(argMap.count("newPasswd"))
								{
									Team *team = session->GetTeam();
									if(team)
										if(team->TestPassword(argMap["curPasswd"]))
										{
											string pass = argMap["newPasswd"];
											if(pass.size())
											{
												team->SetPassword(pass);
												Team::SaveToDatabase();
											}
										}
								}
						}
					}
				}
			}

		stringstream webPageStream;

		bool img = false;
		if(type.length())
			if(!type.compare("jpg") || !type.compare("jpeg") || !type.compare("png") || !type.compare("bmp"))
				img = true;

		if(img)
		{
			LoadImage(webPageStream, file);
			string response = webPageStream.str();
			client->Write((char *)response.c_str(), response.length());
		}
		else
		{
			if(e404)
				OpenHeader_NotFound(webPageStream);
			else if(!type.compare("css"))
				OpenHeader_OK_CSS(webPageStream);
			else
				OpenHeader_OK(webPageStream);
			if(session && !loggingOut)
			{
				char idBuff[8];
				memset(idBuff, 0, 8);

				#ifdef _WIN32
					_itoa_s(session->GetID(), idBuff, 8, 10);
				#else
					sprintf(idBuff, "%d", session->GetID());
				#endif
				SetSessionCookie(webPageStream, "BEACHJUDGESESSID", string(idBuff));
			}
			else
				SetSessionCookie(webPageStream, "BEACHJUDGESESSID", "deleted; Expires=Thu, 01 Jan 1970 00:00:00 GMT");
			CloseHeader(webPageStream);

			Page *index = Page::Create(file);
			index->AddToStream(webPageStream, client, session);
			string webPage = webPageStream.str();
			client->Write((char *)webPage.c_str(), webPage.length());
		}

		if(loggingOut)
			if(session)
			{
				delete session;
				session = 0;
			}
	}
}
