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
		g_pageMap[file] = page;
		return page;
	}
	void Page::Cleanup()
	{
		while(g_pageMap.size())
			delete g_pageMap.begin()->second;
	}

	Page::Page()
	{
	}
	Page::~Page()
	{
		g_pageMap.erase(m_fileSource);
	}
	void Page::AddToStream(stringstream &stream, Socket *client, Session *session, map<string, string> *masterLocalVars)
	{
		stringstream pageStream(m_html);
		string chunk, varChunk, arg, val, loopTarget;
		stringstream loopStream;
		vector<string> ifStack;
		map<string, string> localVars;
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
				if(!varChunk.compare("loop"))
				{
					if(arg.size() && !doLoop)
					{
						loopTarget = arg;
						loopStream.str(string());
						doLoop = true;
					}
					else
						stream << "$" << varChunk << ":" << arg;
				}
				else if(!varChunk.compare("endLoop"))
				{
					map<unsigned short, Team *>::iterator teamIt;
					map<unsigned short, Team *> &teamsByID = Team::GetTeamsByID();
					if(doLoop)
					{
						bool done = false;
						if(!loopTarget.compare("teams"))
							teamIt = teamsByID.begin();
						else
							done = true;
						while(!done)
						{
							stringstream inLoopStream(loopStream.str());
							if(!loopTarget.compare("teams"))
							{
								localVars["teamName"] = teamIt->second->GetName();
								char idStr[8];
								memset(idStr, 0, 8);
								sprintf(idStr, "%d", teamIt->first);
								localVars["teamID"] = string(idStr);
								teamIt++;
								if(teamIt == teamsByID.end())
									done = true;
							}

							string embChunk, embVarChunk;
							while(getline(inLoopStream, embChunk, '$'))
							{
								bool embValid = true;
								if(ifStack.size())
								{
									if(session)
									{
										for(vector<string>::iterator it = ifStack.begin(); it != ifStack.end(); it++)
											if(session->GetVariable(*it) == 0)
											{
												embValid = false;
												break;
											}
									}
									else if(ifStack.front().compare("loggedOut"))
										embValid = false;
								}

								if(embValid)
									stream << embChunk;

								if(inLoopStream.eof())
									break;

								embVarChunk = "";
								while(true)
								{
									char peek = inLoopStream.peek();
									if((peek >= 'A' && peek <= 'Z') || (peek >= 'a' && peek <= 'z'))
									{
										embVarChunk.push_back(peek);
										inLoopStream.get();
									}
									else if(peek == ':')
									{
										arg = "";
										inLoopStream.get();
										while(true)
										{
											char argPeek = inLoopStream.peek();
											if((argPeek >= 'A' && argPeek <= 'Z') || (argPeek >= 'a' && argPeek <= 'z') || argPeek == '.' || argPeek == '/')
											{
												arg.push_back(argPeek);
												inLoopStream.get();
											}
											else if(argPeek == '=')
											{
												val = "";
												inLoopStream.get();
												while(true)
												{
													char valPeek = inLoopStream.peek();
													if((valPeek >= 'A' && valPeek <= 'Z') || (valPeek >= 'a' && valPeek <= 'z') || (valPeek >= '0' && valPeek <= '9') || valPeek == '.' || valPeek == '/' || valPeek == ' ')
													{
														val.push_back(valPeek);
														inLoopStream.get();
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
								
								if(!embVarChunk.compare("if"))
									ifStack.push_back(arg);
								else if(!embVarChunk.compare("endif"))
									ifStack.erase(find(ifStack.begin(), ifStack.end(), arg));
								else if(embValid)
								{
									if(!embVarChunk.compare("set"))
									{
										if(arg.size() && val.size())
										{
											if(masterLocalVars)
												masterLocalVars->operator[](arg) = val;
											else
												localVars[arg] = val;
										}
										else
											stream << "$" << embVarChunk << ":" << arg << "=" << val;
									}
									else if(!embVarChunk.compare("get"))
									{
										bool fail = false;
										if(masterLocalVars)
										{
											if(masterLocalVars->count(arg))
												stream << masterLocalVars->operator[](arg);
											else
												fail = true;
										}
										else if(localVars.count(arg))
											stream << localVars[arg];
										else
											fail = true;

										if(fail)
											stream << "$" << embVarChunk << ":" << arg;
									}
									else if(!embVarChunk.compare("include"))
									{
										string file(includePrefix);
										file.append(arg);
										if(fileExists(file.c_str()))
										{
											Page *page = Page::Create(file);
											page->AddToStream(stream, client, session, &localVars);
										}
									}
									else if(g_templateMap.count(embVarChunk))
										g_templateMap[embVarChunk](stream, client, session, arg);
									else
										stream << "$" << embVarChunk;
								}
							}
						}

						if(!loopTarget.compare("teams"))
						{
							localVars.erase("teamName");
							localVars.erase("teamID");
						}

						doLoop = false;
					}
					else
						stream << "$" << varChunk;
				}
				else if(!varChunk.compare("set"))
				{
					if(doLoop)
						loopStream << "$" << varChunk << ":" << arg << "=" << val;
					else
					{
						if(arg.size() && val.size())
						{
							if(masterLocalVars)
								masterLocalVars->operator[](arg) = val;
							else
								localVars[arg] = val;
						}
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
						bool fail = false;
						if(masterLocalVars)
						{
							if(masterLocalVars->count(arg))
								stream << masterLocalVars->operator[](arg);
							else
								fail = true;
						}
						else if(localVars.count(arg))
							stream << localVars[arg];
						else
							fail = true;

						if(fail)
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
							page->AddToStream(stream, client, session, &localVars);
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

		delete this;
	}
}
