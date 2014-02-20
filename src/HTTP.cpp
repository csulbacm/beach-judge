//- Standard Library -
#include <iostream>
#include <fstream>
#include <algorithm>
#include <cstring>

//- Beach Judge -
#include <BeachJudge/Base.h>
#include <BeachJudge/Question.h>
#include <BeachJudge/Page.h>
#include <BeachJudge/Session.h>
#include <BeachJudge/HTTP.h>
#include <BeachJudge/Team.h>

using namespace std;

#define TEAM_COMM_LIMIT 32 * 1024
#define ANON_COMM_LIMIT 1024

const char *wwwPrefix = "../www";

namespace beachjudge
{
	void HTTP::OpenHeader_OK(stringstream &stream)
	{
		stream << "HTTP/1.1 200 OK\r\nConnection: close\r\nContent-type: text/html\r\n";
	}
	void HTTP::OpenHeader_OK_MultiPart(stringstream &stream)
	{
		stream << "HTTP/1.1 200 OK\r\nConnection: keep-alive\r\nContent-type: text/html\r\n";
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
	void HTTP::HandleClient(Socket *client)
	{
		stringstream reqStream;

		{
			char sbuff[1025];
			memset(sbuff, 0, 1025);
			unsigned short len = client->Read(sbuff, 1024);
			if(len)
			{
				string part(sbuff);
				reqStream << part;
			}
		}

		if(reqStream.str().size() <= 1)
			return;

		unsigned short port = 0;
		unsigned long addr = 0;
		client->GetPeerIP4Info(&addr, &port);

		Session *session = 0;

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

//		cout << reqStream.str() << endl;

		string line, contentType, boundary("--");
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
			{
				istringstream rightStream(right);
				getline(rightStream, contentType, ';');
				if(!contentType.compare("multipart/form-data"))
				{
					size_t b = right.find("boundary=");
					if(b != string::npos)
						boundary.append(right.substr(b + 9));
				}
			}
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
						Session *sess = Session::Lookup(sessID);
						if(sess)
							if(sess->GetAddress() == addr)
							{
								sess->ResetTimeout();
								session = sess;
							}
					}
				}
			}
		}
		while(line.size() > 1);

		sleepMS(10);

		if(client->HasRead())
		{
			char sbuff[257];
			do
			{
				memset(sbuff, 0, 257);
				unsigned short len = client->Read(sbuff, 256);
				if(len)
				{
					string part(sbuff);
					reqStream << part;
					if(session)
					{
						if(!session->GetTeam()->IsJudge())
							if(reqStream.str().size() > TEAM_COMM_LIMIT)
								return;
					}
					else if(reqStream.str().size() > ANON_COMM_LIMIT)
						return;
				}
			}
			while(client->HasRead());
		}

		map<string, string> getArgMap;
		{
			string arg, args;
			stringstream argStream(arguments);
			while(getline(argStream, arg, '='))
			{
				string val;
				getline(argStream, val, '&');
				if(arg.size() && val.size())
					getArgMap[arg] = val;
			}
		}

//		cout << reqStream.str() << endl;
		map<string, string> postArgMap;

		if(contentLength)
			if(!method.compare("POST"))
			{
				if(!contentType.compare("application/x-www-form-urlencoded"))
				{
					string arg, args;
					getline(reqStream, args);
					stringstream argStream(args);
					while(getline(argStream, arg, '='))
					{
						string val;
						getline(argStream, val, '&');
						if(arg.size() && val.size())
							postArgMap[arg] = val;
					}
				}
				else if(!contentType.compare("multipart/form-data"))
				{
					string part;
					size_t nextPart, lastPart, boundarySize = boundary.size();
					lastPart = reqStream.tellg();
					bool doIt = true;
					while(doIt)
					{
						nextPart = reqStream.str().find(boundary, lastPart);
						if(nextPart == string::npos)
						{
							nextPart = reqStream.str().size();
							doIt = false;
						}
						part = reqStream.str().substr(lastPart, nextPart - lastPart);
						if(part.size() > 2)
						{
							istringstream partStream(part);
							string contentDisposition, cdArg, postArgName;
							getline(partStream, contentDisposition);
							getline(partStream, contentDisposition);
							
							istringstream contentDispositionStream(contentDisposition);
							contentDispositionStream >> cdArg >> cdArg;
							bool readFile = false;
							while(getline(contentDispositionStream, cdArg, ';'))
							{
								if(cdArg.at(cdArg.size() - 1) == '\r')
									cdArg = cdArg.substr(1, cdArg.size() - 2);
								else
									cdArg = cdArg.substr(1);

								istringstream cdArgStream(cdArg);
								string argName, argVal;
								getline(cdArgStream, argName, '=');
								getline(cdArgStream, argVal);
								argVal = argVal.substr(1, argVal.size() - 2);
								if(!argName.compare("name"))
									postArgName = argVal;
								else if(!argName.compare("filename"))
								{
									postArgMap["sourceFile"] = argVal;
									readFile = true;
								}
							}
							if(readFile)
							{
								string fileContentType;
								getline(partStream, fileContentType);
							}

							string piece;
							getline(partStream, piece);

							if(readFile)
							{
								stringstream partValueStream;
								while(getline(partStream, piece))
									partValueStream << piece << endl;
								piece = partValueStream.str().substr(0, partValueStream.str().size() - 1);
							}
							else
								getline(partStream, piece, '\r');

							if(postArgName.size())
								postArgMap[postArgName] = piece;
						}
						lastPart = nextPart + boundarySize;
					}
				}
			}

		if(postArgMap.count("cmd"))
		{
			if(session)
			{
				string &cmd = postArgMap["cmd"];
				if(!cmd.compare("Change"))
				{
					if(postArgMap.count("curPasswd"))
						if(postArgMap.count("newPasswd"))
						{
							Team *team = session->GetTeam();
							if(team)
								if(team->TestPassword(postArgMap["curPasswd"]))
								{
									string pass = postArgMap["newPasswd"];
									team->SetPassword(pass);
									Team::SaveToDatabase();
								}
						}
				}
				else if(!cmd.compare("createTeam"))
				{
					Team *team = session->GetTeam();
					if(team)
						if(team->IsJudge())
							if(postArgMap.count("newTeamName"))
								if(postArgMap.count("newTeamPass"))
								{
									string name = postArgMap["newTeamName"];
									if(!Team::LookupByName(name))
									{
										string pass = postArgMap["newTeamPass"];
										Team::Create(name, pass);
										Team::SaveToDatabase();
									}
								}
				}
				else if(!cmd.compare("askQuestion"))
				{
					Team *team = session->GetTeam();
					if(team)
						if(!team->IsJudge())
							if(postArgMap.count("question"))
								if(postArgMap.count("problemID"))
								{
									Problem *problem = Problem::LookupByID(atoi(postArgMap["problemID"].c_str()));
									if(problem)
									{
										string question = postArgMap["question"];
										Question::Create(question, team, problem);
									}
								}
				}
				else if(!cmd.compare("answerQuestion"))
				{
					Team *team = session->GetTeam();
					if(team)
						if(team->IsJudge())
							if(postArgMap.count("answer"))
								if(postArgMap.count("questionID"))
								{
									Question *question = Question::LookupByID(atoi(postArgMap["questionID"].c_str()));
									if(question)
										question->Answer(postArgMap["answer"]);
								}
				}
				else if(!cmd.compare("dismissQuestion"))
				{
					Team *team = session->GetTeam();
					if(team)
						if(team->IsJudge())
							if(postArgMap.count("questionID"))
							{
								Question *question = Question::LookupByID(atoi(postArgMap["questionID"].c_str()));
								if(question)
									delete question;
							}
				}
				else if(!cmd.compare("submit"))
				{
					Team *team = session->GetTeam();
					if(team)
						if(!team->IsJudge())
							if(postArgMap.count("problemID"))
								if(postArgMap.count("code"))
								{
									Problem *problem = Problem::LookupByID(atoi(postArgMap["problemID"].c_str()));
									if(problem)
									{
										CodeType codeType = CodeType_Unknown;
										string sourceFile = postArgMap["sourceFile"];
										transform(sourceFile.begin(), sourceFile.end(), sourceFile.begin(), ::tolower);
										string ext = fileExt(sourceFile.c_str());
										if(!ext.compare("cpp"))
											codeType = CodeType_CPP;
										else if(!ext.compare("java"))
											codeType = CodeType_Java;
										else if(!ext.compare("c"))
											codeType = CodeType_C;

										Submission *submission = Submission::Create(team, problem, codeType, getRunTimeMS());
										if(submission)
										{
											string codeFile = submission->GetSourceFile();

											ofstream srcFileOut(codeFile.c_str());
											srcFileOut << postArgMap["code"];
											srcFileOut.close();
										}
									}
								}
				}
			}
			else
			{
				if(!postArgMap["cmd"].compare("login"))
					if(postArgMap.count("passwd"))
						if(postArgMap.count("team"))
						{
							Team *team = Team::LookupByName(postArgMap["team"]);
							if(team)
								if(team->TestPassword(postArgMap["passwd"]))
									session = Session::Create(addr, port, team);
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
			index->AddToStream(webPageStream, client, session, &getArgMap);
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
