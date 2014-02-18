//- Standard Library -
#include <iostream>
#include <fstream>
#include <map>
#include <vector>
#include <algorithm>
#include <cstring>

//- Beach Judge -
#include <BeachJudge/Base.h>
#include <BeachJudge/Page.h>
#include <BeachJudge/Problem.h>

using namespace std;

const char *includePrefix = "../www/";

namespace beachjudge
{
	map<string, Page *> g_pageMap;
	map<string, void (*)(stringstream &, Socket *, Session *, std::string)> g_templateMap;

	void TeamID(stringstream &stream, Socket *socket, Session *session, string arg)
	{
		Team *team = session->GetTeam();
		if(team)
			stream << team->GetID();
	}
	void TeamName(stringstream &stream, Socket *socket, Session *session, string arg)
	{
		Team *team = session->GetTeam();
		if(team)
			stream << team->GetName();
	}

	void Page::RegisterTemplate(string entry, void (*func)(stringstream &, Socket *, Session *, string))
	{
		g_templateMap[entry] = func;
	}
	void Page::RegisterDefaultTemplates()
	{
		RegisterTemplate("teamID", &TeamID);
		RegisterTemplate("teamName", &TeamName);
	}
	Page *Page::Create(string file)
	{
		if(!fileExists(file.c_str()))
			return 0;

		if(g_pageMap.count(file))
			return g_pageMap[file];

		string html, lineIn;
		ifstream inFile(file.c_str());
		while(getline(inFile, lineIn))
		{
			html = html.append(lineIn);
			html = html.append("\r\n");
		}

		Page *page = new Page();
		page->m_fileSource = file;
		page->m_html = html;
		page->m_isTemp = true; //- TODO: Remove this for caching -
		g_pageMap[file] = page;
		return page;
	}
	Page *Page::CreateFromHTML(string html)
	{
		Page *page = new Page();
		page->m_fileSource = "";
		page->m_html = html;
		return page;
	}
	void Page::Cleanup()
	{
		while(g_pageMap.size())
			delete g_pageMap.begin()->second;
	}

	Page::Page()
	{
		m_isTemp = false;
	}
	Page::~Page()
	{
		if(m_fileSource.size())
			g_pageMap.erase(m_fileSource);
	}
	void Page::AddToStream(stringstream &stream, Socket *client, Session *session, map<string, string> *masterLocalVars)
	{
		stringstream pageStream(m_html);
		string chunk, varChunk, arg, val, loopTarget;
		stringstream loopStream;
		vector<string> ifStack;
		map<string, string> localVars, *targetVars = 0;
		if(masterLocalVars)
			targetVars = masterLocalVars;
		else
			targetVars = &localVars;
		bool doLoop = false;

		while(getline(pageStream, chunk, '$'))
		{
			bool valid = true;
			if(ifStack.size())
			{
				if(session)
				{
					for(vector<string>::iterator it = ifStack.begin(); it != ifStack.end(); it++)
						if(session->GetVariable(*it) == 0)
						{
							valid = false;
							break;
						}
				}
				else if(ifStack.front().compare("loggedOut"))
					valid = false;
			}

			if(valid)
			{
				if(doLoop)
					loopStream << chunk;
				else
					stream << chunk;
			}

			if(pageStream.eof())
				break;

			varChunk = "";
			while(true)
			{
				char peek = pageStream.peek();
				if((peek >= 'A' && peek <= 'Z') || (peek >= 'a' && peek <= 'z'))
				{
					varChunk.push_back(peek);
					pageStream.get();
				}
				else if(peek == ':')
				{
					arg = "";
					pageStream.get();
					while(true)
					{
						char argPeek = pageStream.peek();
						if((argPeek >= 'A' && argPeek <= 'Z') || (argPeek >= 'a' && argPeek <= 'z') || argPeek == '.' || argPeek == '/')
						{
							arg.push_back(argPeek);
							pageStream.get();
						}
						else if(argPeek == '{' && !doLoop)
						{
							pageStream.get();
							string eval;
							while(true)
							{
								char evalPeek = pageStream.peek();
								if((evalPeek >= 'A' && evalPeek <= 'Z') || (evalPeek >= 'a' && evalPeek <= 'z') || (evalPeek >= '0' && evalPeek <= '9') || evalPeek == '.' || evalPeek == '/' || evalPeek == ' ')
								{
									eval.push_back(evalPeek);
									pageStream.get();
								}
								else
									break;
							}
							char lastPeek = pageStream.peek();
							if(lastPeek == '}')
							{
								pageStream.get();
								if(targetVars->count(eval))
								{
									eval = targetVars->operator[](eval);
									arg.append(eval);
								}
								else
								{
									arg.push_back('{');
									arg.append(eval);
									arg.push_back('}');
								}
							}
							else
							{
								arg.push_back('{');
								arg.append(eval);
							}
						}
						else if(argPeek == '=')
						{
							val = "";
							pageStream.get();
							while(true)
							{
								char valPeek = pageStream.peek();
								if((valPeek >= 'A' && valPeek <= 'Z') || (valPeek >= 'a' && valPeek <= 'z') || (valPeek >= '0' && valPeek <= '9') || valPeek == '.' || valPeek == '/' || valPeek == ' ')
								{
									val.push_back(valPeek);
									pageStream.get();
								}
								else if(valPeek == '{' && !doLoop)
								{
									pageStream.get();
									string eval;
									while(true)
									{
										char evalPeek = pageStream.peek();
										if((evalPeek >= 'A' && evalPeek <= 'Z') || (evalPeek >= 'a' && evalPeek <= 'z') || (evalPeek >= '0' && evalPeek <= '9') || evalPeek == '.' || evalPeek == '/' || evalPeek == ' ')
										{
											eval.push_back(evalPeek);
											pageStream.get();
										}
										else
											break;
									}
									char lastPeek = pageStream.peek();
									if(lastPeek == '}')
									{
										pageStream.get();
										if(targetVars->count(eval))
										{
											eval = targetVars->operator[](eval);
											val.append(eval);
										}
										else
										{
											val.push_back('{');
											val.append(eval);
											val.push_back('}');
										}
									}
									else
									{
										val.push_back('{');
										val.append(eval);
									}
								}
								else
									break;
							}
						}
						else
							break;
					}
				}
				else
					break;
			}
			
			if(!varChunk.compare("if"))
			{
				if(doLoop)
					loopStream << "$" << varChunk << ":" << arg;
				else
					ifStack.push_back(arg);
			}
			else if(!varChunk.compare("endif"))
			{
				if(doLoop)
					loopStream << "$" << varChunk << ":" << arg;
				else
					ifStack.erase(find(ifStack.begin(), ifStack.end(), arg));
			}
			else if(valid)
			{
				if(!varChunk.compare("startLoop"))
				{
					if(doLoop)
						loopStream << "$" << varChunk << ":" << arg;
					else
					{
						if(arg.size())
						{
							loopTarget = arg;
							loopStream.str(string());
							doLoop = true;
						}
						else
							stream << "$" << varChunk << ":" << arg;
					}
				}
				else if(!varChunk.compare("endLoop"))
				{
					if(doLoop)
					{
						if(arg.size())
						{
							if(!arg.compare(loopTarget))
							{
								map<unsigned short, Team *>::iterator teamIt;
								map<unsigned short, Team *> &teamsByID = Team::GetTeamsByID();
								map<unsigned short, Problem *>::iterator problemIt;
								map<unsigned short, Problem *> &problemsByID = Problem::GetProblemsByID();

								bool done = false;
								if(!loopTarget.compare("teams"))
									teamIt = teamsByID.begin();
								else if(!loopTarget.compare("problems"))
									problemIt = problemsByID.begin();
								else
									done = true;
								Page *embPage = Page::CreateFromHTML(loopStream.str());
								unsigned short idx = 0;
								while(!done)
								{
									idx++;
									if(!loopTarget.compare("teams"))
									{
										targetVars->operator[]("teamName") = teamIt->second->GetName();
										char str[8];
										memset(str, 0, 8);
										sprintf(str, "%d", teamIt->first);
										targetVars->operator[]("teamID") = string(str);
										memset(str, 0, 8);
										sprintf(str, "%d", idx);
										targetVars->operator[]("teamIdx") = string(str);
										teamIt++;
										if(teamIt == teamsByID.end())
											done = true;
									}
									else if(!loopTarget.compare("problems"))
									{
										targetVars->operator[]("problemName") = problemIt->second->GetName();
										char str[8];
										memset(str, 0, 8);
										sprintf(str, "%d", problemIt->first);
										targetVars->operator[]("problemID") = string(str);
										memset(str, 0, 8);
										sprintf(str, "%d", idx);
										targetVars->operator[]("problemIdx") = string(str);
										problemIt++;
										if(problemIt == problemsByID.end())
											done = true;
									}
									embPage->AddToStream(stream, client, session, targetVars);
								}
								delete embPage;

								if(!loopTarget.compare("teams"))
								{
									targetVars->erase("teamName");
									targetVars->erase("teamID");
									targetVars->erase("teamIdx");
								}
								else if(!loopTarget.compare("problems"))
								{
									targetVars->erase("problemName");
									targetVars->erase("problemID");
									targetVars->erase("problemIdx");
								}
								doLoop = false;
							}
							else
								loopStream << "$" << varChunk << ":" << arg;
						}
						else
							stream << "$" << varChunk << ":" << arg;
					}
					else
						stream << "$" << varChunk << ":" << arg;
				}
				else if(!varChunk.compare("set"))
				{
					if(doLoop)
						loopStream << "$" << varChunk << ":" << arg << "=" << val;
					else
					{
						if(arg.size() && val.size())
							targetVars->operator[](arg) = val;
						else
							stream << "$" << varChunk << ":" << arg << "=" << val;
					}
				}
				else if(!varChunk.compare("get"))
				{
					if(doLoop)
						loopStream << "$" << varChunk << ":" << arg;
					else
					{
						if(targetVars->count(arg))
							stream << targetVars->operator[](arg);
						else
							stream << "$" << varChunk << ":" << arg;
					}
				}
				else if(!varChunk.compare("include"))
				{
					if(doLoop)
						loopStream << "$" << varChunk << ":" << arg;
					else
					{
						string file(includePrefix);
						file.append(arg);
						if(fileExists(file.c_str()))
						{
							Page *page = Page::Create(file);
							page->AddToStream(stream, client, session, targetVars);
						}
					}
				}
				else if(doLoop)
					loopStream << "$" << varChunk;
				else if(g_templateMap.count(varChunk))
					g_templateMap[varChunk](stream, client, session, arg);
				else
					stream << "$" << varChunk;
			}
		}

		if(m_isTemp)
			delete this;
	}
}
