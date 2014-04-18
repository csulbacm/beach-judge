//- Standard Library -
#include <iostream>
#include <fstream>
#include <algorithm>
#include <cstring>

//- Beach Judge -
#include <BeachJudge/Base.h>
#include <BeachJudge/Question.h>
#include <BeachJudge/Competition.h>
#include <BeachJudge/Problem.h>
#include <BeachJudge/Page.h>
#include <BeachJudge/Session.h>
#include <BeachJudge/HTTP.h>
#include <BeachJudge/Team.h>

using namespace std;

#define TEAM_COMM_LIMIT 32 * 1024
#define ANON_COMM_LIMIT 1024

const char *wwwPrefix = "../www";

#ifdef _WIN32
	#define SPRINTF sprintf_s
#else
	#define SPRINTF	sprintf
#endif

#if BEACHJUDGE_USEPOSIXSOCKET
	//- POSIX -
	#include <sys/types.h>
	#include <sys/socket.h>
	#include <netinet/in.h>
	#include <arpa/inet.h>
	#include <cstdio>
	#include <cstdlib>
	#include <cstring>
	#include <unistd.h>
	#include <fcntl.h>

	pthread_mutex_t g_pageAccessMutex;
#endif

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

		while(true)
		{
			char c;
			inFile.get(c);
			if(inFile.eof())
				break;
			img.push_back(c);
		}

		unsigned long size = img.size();
		stream << "Content-Length: " << size << "\r\n\r\n";
		
		for(unsigned long a = 0; a < size; a++)
			stream << img[a];

		inFile.close();
	}
	void HTTP::LoadBinaryFile(stringstream &stream, string file, string name, bool isAttachment)
	{
		ifstream inFile(file.c_str(), ios::binary);

		stream << "HTTP/1.1 200 OK\r\nConnection: close\r\nContent-Disposition:";
		if(isAttachment)
			stream << " attachment;";
		stream << " filename=\"" << name << "\"\r\nContent-type: */*\r\n";

		string img;

		while(true)
		{
			char c;
			inFile.get(c);
			if(inFile.eof())
				break;
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
	void HTTP::Init()
	{
		#if BEACHJUDGE_USEPOSIXSOCKET
			pthread_mutex_init(&g_pageAccessMutex, 0);
		#endif
	}
	void HTTP::Cleanup()
	{
		#if BEACHJUDGE_USEPOSIXSOCKET
			pthread_mutex_destroy(&g_pageAccessMutex);
		#endif
	}
	void HTTP::HandleClient(Socket *client)
	{
		stringstream reqStream(ios::in | ios::out | ios::binary);

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

		#if BEACHJUDGE_USEPOSIXSOCKET
			pthread_mutex_lock(&g_pageAccessMutex);
		#endif

		unsigned short port = 0;
		unsigned long addr = 0;
		client->GetPeerIP4Info(&addr, &port);

		Session *session = 0;
		Competition *competition = Competition::GetCurrent();

		string str;

		//- URI Request -
		getline(reqStream, str);
		istringstream uriRequest(str);
		string method, uri, htmlVersion;
		uriRequest >> method >> uri >> htmlVersion;

		string file(wwwPrefix), arguments, type, requestFileName;

		bool e404 = false, loggingOut = false, fileRequest = false, submissionPoll = false;

		map<string, string> getArgMap;

		{
			string uriCopy = uri;
			size_t argToken;
			argToken = uri.find_first_of('?');

			if(argToken != string::npos)
			{
				arguments = uriCopy.substr(argToken + 1);
				uriCopy = uriCopy.substr(0, argToken);
			}

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

			if(uriCopy.length() == 1)
				file.append("/index.html");
			else
			{
				if(!uriCopy.compare("/file"))
				{
					if(getArgMap.count("f"))
						fileRequest = true;
					else
						e404 = true;
				}
				else
				{
					if(!uriCopy.compare("/logout")) //- TODO: Find a better way to do this -
						loggingOut = true;
					else if(!uriCopy.compare("/submissionPoll"))
						submissionPoll = true;

					string testFile = file;
					testFile.append(uriCopy);
					if(fileExists(testFile.c_str()))
						file = testFile;
					else
					{
						testFile.append(".html");
						if(fileExists(testFile.c_str()))
							file = testFile;
						else if(!submissionPoll)
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
					if(cookie.at(0) == ' ')
						cookie = cookie.substr(1);
					string val;
					getline(cookieStream, val, ';');

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

//		sleepMS(100);
		sleepMS(16);

		if(client->HasRead())
		{
			unsigned short timeout = 0;
			char sbuff[257];
			do
			{
				memset(sbuff, 0, 257);
				unsigned short len = client->Read(sbuff, 256);
				if(len)
				{
					timeout = 0;
					for(unsigned short a = 0; a < len; a++)
						reqStream.put(sbuff[a]);
//					string part(sbuff);
//					reqStream << part;
					if(session)
					{
						if(!session->GetTeam()->IsJudge())
							if(reqStream.str().size() > TEAM_COMM_LIMIT)
								return;
					}
					else if(reqStream.str().size() > ANON_COMM_LIMIT)
						return;
				}
				else
					timeout++;
				//- TODO: Verify upload stability -
//				cout << reqStream.str().size() << endl;
				sleepMS(16);
			}
			while(client->HasRead() && timeout < 15);
		}

//		#if BEACHJUDGE_USEPOSIXSOCKET
//			pthread_mutex_lock(&g_pageAccessMutex);
//		#endif

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
					lastPart = (size_t)reqStream.tellg();
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
							istringstream partStream(part, ios::in | ios::binary);
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
								stringstream partValueStream(ios::in | ios::out | ios::binary);
								while(!partStream.eof())
								{
									char c;
									partStream.get(c);
									partValueStream.put(c);
								}
								piece = partValueStream.str().substr(0, partValueStream.str().size() - 3);
/*								stringstream partValueStream(ios::in | ios::out | ios::binary);
								while(getline(partStream, piece))
									partValueStream << piece << endl;
								piece = partValueStream.str().substr(0, partValueStream.str().size() - 1);*/
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

		if(fileRequest)
		{
			string fileRequestType = getArgMap["f"];
			if(!fileRequestType.compare("info"))
			{
				if(getArgMap.count("p")) //- TODO: Verify security -
				{
					string testFile = "compo/problems/";
					testFile.append(getArgMap["p"]);
					testFile.append(".pdf");
					if(fileExists(testFile.c_str()))
					{
						file = testFile;
						requestFileName = getArgMap["p"];
						requestFileName.append(".pdf");
					}
					else
						e404 = true;
				}
			}
			else if(!fileRequestType.compare("sample"))
			{
				if(getArgMap.count("p")) //- TODO: Verify security -
				{
					string testFile = "compo/problems/";
					testFile.append(getArgMap["p"]);
					testFile.append("-sample.zip");
					if(fileExists(testFile.c_str()))
					{
						file = testFile;
						requestFileName = getArgMap["p"];
						requestFileName.append("-sample.zip");
					}
					else
						e404 = true;
				}
			}
			else if(!fileRequestType.compare("testIn"))
			{
				if(getArgMap.count("p") && getArgMap.count("t")) //- TODO: Verify security -
				{
					string testFile = "compo/problems/";
					testFile.append(getArgMap["p"]);
					testFile.append("-tests/");
					testFile.append(getArgMap["t"]);
					testFile.append(".in");
					if(fileExists(testFile.c_str()))
					{
						file = testFile;
						requestFileName = getArgMap["p"];
						requestFileName.append("-");
						requestFileName.append(getArgMap["t"]);
						requestFileName.append(".in");
					}
					else
						e404 = true;
				}
			}
			else if(!fileRequestType.compare("testOut"))
			{
				if(getArgMap.count("p") && getArgMap.count("t")) //- TODO: Verify security -
				{
					string testFile = "compo/problems/";
					testFile.append(getArgMap["p"]);
					testFile.append("-tests/");
					testFile.append(getArgMap["t"]);
					testFile.append(".out");
					if(fileExists(testFile.c_str()))
					{
						file = testFile;
						requestFileName = getArgMap["p"];
						requestFileName.append("-");
						requestFileName.append(getArgMap["t"]);
						requestFileName.append(".out");
					}
					else
						e404 = true;
				}
			}
			else if(!fileRequestType.compare("source"))
			{
				Team *team = session->GetTeam();
				if(team && getArgMap.count("s")) //- TODO: Verify security -
				{
					unsigned short sid = atoi(getArgMap["s"].c_str());
					Submission *submission = Submission::LookupByID(sid);
					if(submission)
					{
						if(team->IsJudge() || team == submission->GetTeam())
						{
							file = submission->GetSourceFile();
							char sidStr[16];
							memset(sidStr, 0, 16);
							SPRINTF(sidStr, "%d.", sid);
							requestFileName.append(sidStr);
							requestFileName.append(fileExt(file.c_str()));
						}
						else
							e404 = true;
					}
					else
						e404 = true;
				}
			}
			else
				e404 = true;
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
				else if(!cmd.compare("manageTeam"))
				{
					Team *team = session->GetTeam();
					if(team)
						if(team->IsJudge())
							if(postArgMap.count("teamID"))
							{
								Team *targetTeam = Team::LookupByID(atoi(postArgMap["teamID"].c_str()));
								if(targetTeam)
								{
									bool didSomething = false;
									if(postArgMap.count("newTeamName"))
									{
										string name = postArgMap["newTeamName"];
										if(name.length())
											if(!Team::LookupByName(name))
											{
												targetTeam->SetName(name);
												didSomething = true;
											}
									}
									if(postArgMap.count("newTeamPass"))
									{
										string pass = postArgMap["newTeamPass"];
										if(pass.length())
										{
											targetTeam->SetPassword(pass);
											didSomething = true;
										}
									}
									if(didSomething)
										Team::SaveToDatabase();
								}
							}
				}
				else if(!cmd.compare("changeProblemName"))
				{
					Team *team = session->GetTeam();
					if(team)
						if(team->IsJudge())
							if(postArgMap.count("problemID") && postArgMap.count("newProblemName"))
							{
								Problem *problem = Problem::LookupByID(atoi(postArgMap["problemID"].c_str()));
								if(problem)
								{
									problem->SetName(postArgMap["newProblemName"]);
									Problem::SaveToFile("compo/problems.txt");
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
										if(competition)
											competition->SaveToFile("compo/compo.txt");
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
									{
										question->Answer(postArgMap["answer"]);
										if(competition)
											competition->SaveToFile("compo/compo.txt");
									}
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
								{
									delete question;
									if(competition)
										competition->SaveToFile("compo/compo.txt");
								}
							}
				}
				else if(!cmd.compare("grade"))
				{
					Team *team = session->GetTeam();
					if(team)
						if(team->IsJudge())
							if(postArgMap.count("grading") && postArgMap.count("submissionID"))
							{
								Submission *submission = Submission::LookupByID(atoi(postArgMap["submissionID"].c_str()));
								if(submission)
								{
									SubStatus status = (SubStatus)atoi(postArgMap["grading"].c_str());
									Team *team = submission->GetTeam();
									Problem *problem = submission->GetProblem();

									if(status == 0)
									{
										fileDelete(submission->GetSourceFile().c_str());
										delete submission;
									}
									else
									{
										submission->SetStatus(status);
										if(status == SubStatus_Accepted)
										{
											team->AddScore(problem, ((float)submission->GetTimeMS()) / 1000.f);
											problem->AddSolver(team);
										}
										else
											team->AddPenalty(problem);
										Team::SaveScores();
									}

									Competition *compo = Competition::GetCurrent();
									if(compo)
										compo->SaveToFile("compo/compo.txt");
								}
							}
				}
				else if(!cmd.compare("autoTest"))
				{
					Team *team = session->GetTeam();
					if(team)
						if(team->IsJudge())
							if(postArgMap.count("submissionID"))
							{
								Submission *submission = Submission::LookupByID(atoi(postArgMap["submissionID"].c_str()));
								if(submission)
								{
//									print("Autotesting: %d\n", submission->GetID());
									submission->AutoTest();
									session->SetVariable("autotest", 1);
								}
							}
				}
				else if(!cmd.compare("startCompo"))
				{
					Team *team = session->GetTeam();
					if(team)
						if(team->IsJudge())
						{
							Competition *compo = Competition::GetCurrent();
							if(compo)
							{
								compo->Start();
								compo->SaveToFile("compo/compo.txt");
							}
						}
				}
				else if(!cmd.compare("stopCompo"))
				{
					Team *team = session->GetTeam();
					if(team)
						if(team->IsJudge())
						{
							Competition *compo = Competition::GetCurrent();
							if(compo)
							{
								compo->Stop();
								compo->SaveToFile("compo/compo.txt");
							}
						}
				}
				else if(!cmd.compare("clearData"))
				{
					Team *team = session->GetTeam();
					if(team)
						if(team->IsJudge())
						{
							Competition *compo = Competition::GetCurrent();
							if(compo)
								compo->ClearAll();
						}
				}
				else if(!cmd.compare("submit"))
				{
					Team *team = session->GetTeam();
					if(team)
						if(!team->IsJudge())
							if(postArgMap.count("problemID") && postArgMap.count("code") && postArgMap.count("codeType"))
							{
								Problem *problem = Problem::LookupByID(atoi(postArgMap["problemID"].c_str()));
								if(problem)
								{
/*									CodeType codeType = CodeType_Unknown;
									string sourceFile = postArgMap["sourceFile"];
									transform(sourceFile.begin(), sourceFile.end(), sourceFile.begin(), ::tolower);
									string ext = fileExt(sourceFile.c_str());
									if(!ext.compare("cpp"))
										codeType = CodeType_CPP;
									else if(!ext.compare("java"))
										codeType = CodeType_Java;
									else if(!ext.compare("c"))
										codeType = CodeType_C;
*/
									if(postArgMap["code"].length())
									{
										CodeType codeType = (CodeType)atoi(postArgMap["codeType"].c_str());
										unsigned long long timeScore = 0;
										if(competition)
											timeScore = competition->CalculateTimeScore(getRealTimeMS());
										Submission *submission = Submission::Create(team, problem, codeType, timeScore);
										if(submission)
										{
											string codeFile = submission->GetSourceFile();

											ofstream srcFileOut(codeFile.c_str());
											srcFileOut << postArgMap["code"];
											srcFileOut.close();

											if(competition)
												competition->SaveToFile("compo/compo.txt");
										}
										session->SetVariable("submit", 1);
									}
								}
							}
				}
				else if(!cmd.compare("pdf"))
				{
					Team *team = session->GetTeam();
					if(team)
						if(team->IsJudge())
							if(postArgMap.count("problemID"))
								if(postArgMap.count("file"))
								{
									Problem *problem = Problem::LookupByID(atoi(postArgMap["problemID"].c_str()));
									if(problem)
									{
										string sourceFile = postArgMap["sourceFile"];
										transform(sourceFile.begin(), sourceFile.end(), sourceFile.begin(), ::tolower);
										string ext = fileExt(sourceFile.c_str());
										if(!ext.compare("pdf"))
										{
											string pdfFile = "compo/problems/";
											createFolder(pdfFile.c_str());
											char idStr[8];
											memset(idStr, 0, 8);
											SPRINTF(idStr, "%d", problem->GetID());
											pdfFile.append(idStr);
											pdfFile.append(".pdf");
											ofstream pdfFileOut(pdfFile.c_str(), ios::out | ios::binary);
											pdfFileOut << postArgMap["file"];
											pdfFileOut.close();
										}
									}
								}
				}
				else if(!cmd.compare("sample"))
				{
					Team *team = session->GetTeam();
					if(team)
						if(team->IsJudge())
							if(postArgMap.count("problemID"))
								if(postArgMap.count("file"))
								{
									Problem *problem = Problem::LookupByID(atoi(postArgMap["problemID"].c_str()));
									if(problem)
									{
										string sourceFile = postArgMap["sourceFile"];
										transform(sourceFile.begin(), sourceFile.end(), sourceFile.begin(), ::tolower);
										string ext = fileExt(sourceFile.c_str());
										if(!ext.compare("zip"))
										{
											string sampleFile = "compo/problems/";
											createFolder(sampleFile.c_str());
											char idStr[8];
											memset(idStr, 0, 8);
											SPRINTF(idStr, "%d", problem->GetID());
											sampleFile.append(idStr);
											sampleFile.append("-sample.zip");
											ofstream sampleFileOut(sampleFile.c_str(), ios::out | ios::binary);
											sampleFileOut << postArgMap["file"];
											sampleFileOut.close();
										}
									}
								}
				}
				else if(!cmd.compare("testIn"))
				{
					Team *team = session->GetTeam();
					if(team)
						if(team->IsJudge())
							if(postArgMap.count("problemID") && postArgMap.count("file") && postArgMap.count("testSetID"))
							{
								Problem *problem = Problem::LookupByID(atoi(postArgMap["problemID"].c_str()));
								if(problem)
								{
									Problem::TestSet *testSet = problem->LookupTestSetByID(atoi(postArgMap["testSetID"].c_str()));
									if(testSet)
									{
										string sourceFile = postArgMap["sourceFile"];
										transform(sourceFile.begin(), sourceFile.end(), sourceFile.begin(), ::tolower);
										string ext = fileExt(sourceFile.c_str());
										if(!ext.compare("in"))
										{
											string sampleFile = "compo/problems/";
											createFolder(sampleFile.c_str());
											char idStr[8];
											memset(idStr, 0, 8);
											SPRINTF(idStr, "%d", problem->GetID());
											sampleFile.append(idStr);
											sampleFile.append("-tests/");
											createFolder(sampleFile.c_str());
											memset(idStr, 0, 8);
											SPRINTF(idStr, "%d", testSet->GetID());
											sampleFile.append(idStr);
											sampleFile.append(".in");
											ofstream sampleFileOut(sampleFile.c_str(), ios::out | ios::binary);
											sampleFileOut << postArgMap["file"];
											sampleFileOut.close();
										}
									}
								}
							}
				}
				else if(!cmd.compare("testOut"))
				{
					Team *team = session->GetTeam();
					if(team)
						if(team->IsJudge())
							if(postArgMap.count("problemID") && postArgMap.count("file") && postArgMap.count("testSetID"))
							{
								Problem *problem = Problem::LookupByID(atoi(postArgMap["problemID"].c_str()));
								if(problem)
								{
									Problem::TestSet *testSet = problem->LookupTestSetByID(atoi(postArgMap["testSetID"].c_str()));
									if(testSet)
									{
										string sourceFile = postArgMap["sourceFile"];
										transform(sourceFile.begin(), sourceFile.end(), sourceFile.begin(), ::tolower);
										string ext = fileExt(sourceFile.c_str());
										if(!ext.compare("out"))
										{
											string sampleFile = "compo/problems/";
											createFolder(sampleFile.c_str());
											char idStr[8];
											memset(idStr, 0, 8);
											SPRINTF(idStr, "%d", problem->GetID());
											sampleFile.append(idStr);
											sampleFile.append("-tests/");
											createFolder(sampleFile.c_str());
											memset(idStr, 0, 8);
											SPRINTF(idStr, "%d", testSet->GetID());
											sampleFile.append(idStr);
											sampleFile.append(".out");
											ofstream sampleFileOut(sampleFile.c_str(), ios::out | ios::binary);
											sampleFileOut << postArgMap["file"];
											sampleFileOut.close();
										}
									}
								}
							}
				}
				else if(!cmd.compare("addProblem"))
				{
					Team *team = session->GetTeam();
					if(team)
						if(team->IsJudge())
							if(postArgMap.count("problemName"))
							{
								Problem::Create(postArgMap["problemName"]);
								Problem::SaveToFile("compo/problems.txt");
							}
				}
				else if(!cmd.compare("pRemove"))
				{
					Team *team = session->GetTeam();
					if(team)
						if(team->IsJudge())
							if(postArgMap.count("problemID"))
							{
								Problem *problem = Problem::LookupByID(atoi(postArgMap["problemID"].c_str()));

								char idBuff[8];
								memset(idBuff, 0, 8);
								SPRINTF(idBuff, "%d", problem->GetID());

								string base = "compo/problems/";
								string desc(base), test(base);
								test.append(idBuff);
								test.append("-sample.zip");
								desc.append(idBuff);
								desc.append(".pdf");

								fileDelete(test.c_str());
								fileDelete(desc.c_str());
								delete problem;
								//- Delete Files -
								Problem::SaveToFile("compo/problems.txt");
							}
				}
				else if(!cmd.compare("pMoveUp"))
				{
					Team *team = session->GetTeam();
					if(team)
						if(team->IsJudge())
							if(postArgMap.count("problemID"))
							{
								Problem *problem = Problem::LookupByID(atoi(postArgMap["problemID"].c_str()));
								unsigned short id = problem->GetID();
								if(id != 1)
								{
									Problem *prevProblem = Problem::LookupByID(id - 1);
									if(prevProblem)
									{
										prevProblem->SetID(0);
										problem->SetID(id - 1);
										prevProblem->SetID(id);
										Problem::SaveToFile("compo/problems.txt");
									}
								}
							}
				}
				else if(!cmd.compare("pMoveDown"))
				{
					Team *team = session->GetTeam();
					if(team)
						if(team->IsJudge())
							if(postArgMap.count("problemID"))
							{
								Problem *problem = Problem::LookupByID(atoi(postArgMap["problemID"].c_str()));
								unsigned short id = problem->GetID();
								if(id != Problem::GetProblemsByID().size())
								{
									Problem *nextProblem = Problem::LookupByID(id + 1);
									if(nextProblem)
									{
										nextProblem->SetID(0);
										problem->SetID(id + 1);
										nextProblem->SetID(id);
										Problem::SaveToFile("compo/problems.txt");
									}
								}
							}
				}
				else if(!cmd.compare("addTest"))
				{
					Team *team = session->GetTeam();
					if(team)
						if(team->IsJudge())
							if(postArgMap.count("problemID") && postArgMap.count("testName"))
							{
								Problem *problem = Problem::LookupByID(atoi(postArgMap["problemID"].c_str()));
								if(problem)
								{
									Problem::TestSet::Create(problem, postArgMap["testName"]);
									problem->SaveTestSets();
								}
							}
				}
				else if(!cmd.compare("changeTestSetName"))
				{
					Team *team = session->GetTeam();
					if(team)
						if(team->IsJudge())
							if(postArgMap.count("problemID") && postArgMap.count("testSetID") && postArgMap.count("newTestSetName"))
							{
								Problem *problem = Problem::LookupByID(atoi(postArgMap["problemID"].c_str()));
								if(problem)
								{
									Problem::TestSet *testSet = problem->LookupTestSetByID(atoi(postArgMap["testSetID"].c_str()));
									if(testSet)
									{
										testSet->SetName(postArgMap["newTestSetName"]);
										problem->SaveTestSets();
									}
								}
							}
				}
				else if(!cmd.compare("tRemove"))
				{
					Team *team = session->GetTeam();
					if(team)
						if(team->IsJudge())
							if(postArgMap.count("problemID") && postArgMap.count("testSetID"))
							{
								Problem *problem = Problem::LookupByID(atoi(postArgMap["problemID"].c_str()));
								if(problem)
								{
									Problem::TestSet *testSet = problem->LookupTestSetByID(atoi(postArgMap["testSetID"].c_str()));
									if(testSet)
									{
										//- Delete Files -
										char tidBuff[8], pidBuff[8];
										memset(tidBuff, 0, 8);
										memset(pidBuff, 0, 8);
										SPRINTF(tidBuff, "%d", testSet->GetID());
										SPRINTF(pidBuff, "%d", problem->GetID());

										string base = "compo/problems/";
										base.append(pidBuff);
										base.append("-tests/");

										string tBase(base);
										tBase.append(tidBuff);
										string tIn(tBase), tOut(tBase);
										tIn.append(".in");
										tOut.append(".out");
										fileDelete(tIn.c_str());
										fileDelete(tOut.c_str());

										delete testSet;
										problem->SaveTestSets();
									}
								}
							}
				}
				else if(!cmd.compare("tMoveUp"))
				{
					Team *team = session->GetTeam();
					if(team)
						if(team->IsJudge())
							if(postArgMap.count("problemID") && postArgMap.count("testSetID"))
							{
								Problem *problem = Problem::LookupByID(atoi(postArgMap["problemID"].c_str()));
								if(problem)
								{
									Problem::TestSet *testSet = problem->LookupTestSetByID(atoi(postArgMap["testSetID"].c_str()));
									if(testSet)
									{
										unsigned short id = testSet->GetID();
										if(id != 1)
										{
											Problem::TestSet *prevTestSet = problem->LookupTestSetByID(id - 1);
											if(prevTestSet)
											{
												prevTestSet->SetID(0);
												testSet->SetID(id - 1);
												prevTestSet->SetID(id);
												problem->SaveTestSets();
											}
										}
									}
								}
							}
				}
				else if(!cmd.compare("tMoveDown"))
				{
					Team *team = session->GetTeam();
					if(team)
						if(team->IsJudge())
							if(postArgMap.count("problemID") && postArgMap.count("testSetID"))
							{
								Problem *problem = Problem::LookupByID(atoi(postArgMap["problemID"].c_str()));
								if(problem)
								{
									Problem::TestSet *testSet = problem->LookupTestSetByID(atoi(postArgMap["testSetID"].c_str()));
									if(testSet)
									{
										unsigned short id = testSet->GetID();
										if(id != problem->GetTestSets()->size())
										{
											Problem::TestSet *nextTestSet = problem->LookupTestSetByID(id + 1);
											if(nextTestSet)
											{
												nextTestSet->SetID(0);
												testSet->SetID(id + 1);
												nextTestSet->SetID(id);
												problem->SaveTestSets();
											}
										}
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
								{
									session = Session::Create(addr, port, team);
									Session::SaveAll();
								}
						}
			}
		}

		stringstream webPageStream(ios::in | ios::out | ios::binary);

		if(e404) //- TODO: Determine if this is truly redundant and fix it -
		{
			file = wwwPrefix;
			file.append("/404.html");
		}

		bool img = false;
		if(type.length())
			if(!type.compare("jpg") || !type.compare("jpeg") || !type.compare("png") || !type.compare("bmp"))
				img = true;

		if(submissionPoll)
		{
			#if BEACHJUDGE_USEPOSIXSOCKET
				pthread_mutex_unlock(&g_pageAccessMutex);
			#endif

			char subBuff[16];
			memset(subBuff, 0, 16);
			SPRINTF(subBuff, "%ld", Submission::GetPendingSubmissions()->size());
			client->Write(subBuff, strlen(subBuff));
		}
		else if(img)
		{
			LoadImage(webPageStream, file);
			string response = webPageStream.str();

			#if BEACHJUDGE_USEPOSIXSOCKET
				pthread_mutex_unlock(&g_pageAccessMutex);
			#endif

			client->Write((char *)response.c_str(), response.length());
		}
		else if(fileRequest && !e404)
		{
			LoadBinaryFile(webPageStream, file, requestFileName);
			string response = webPageStream.str();

			#if BEACHJUDGE_USEPOSIXSOCKET
				pthread_mutex_unlock(&g_pageAccessMutex);
			#endif

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

			#if BEACHJUDGE_USEPOSIXSOCKET
				pthread_mutex_unlock(&g_pageAccessMutex);
			#endif

			client->Write((char *)webPage.c_str(), webPage.length());
		}

		if(session)
		{
			Team *team = session->GetTeam();
			if(team)
			{
				session->ClearVariable("submit");
				session->ClearVariable("autotest");
			}
		}

		if(loggingOut)
			if(session)
			{
				delete session;
				session = 0;
				Session::SaveAll();
			}
	}
}
