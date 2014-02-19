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
#include <BeachJudge/Question.h>
#include <BeachJudge/Problem.h>

using namespace std;

const char *includePrefix = "../www/";

namespace beachjudge
{
	map<string, Page *> g_pageMap;
	map<string, void (*)(stringstream &, Socket *, Session *, string, map<string, string> *)> g_templateMap;

	void TeamID(stringstream &stream, Socket *socket, Session *session, string arg, map<string, string> *targetVars)
	{
		Team *team = session->GetTeam();
		if(team)
			stream << team->GetID();
	}
	void TeamName(stringstream &stream, Socket *socket, Session *session, string arg, map<string, string> *targetVars)
	{
		Team *team = session->GetTeam();
		if(team)
			stream << team->GetName();
	}
	void Echo(stringstream &stream, Socket *socket, Session *session, string arg, map<string, string> *targetVars)
	{
		cout << "Echo: " << arg << endl;
	}
	void LoadAnsweredQuestionsForProblem(stringstream &stream, Socket *socket, Session *session, string arg, map<string, string> *targetVars)
	{
		unsigned short id = atoi(arg.c_str());
		Problem *problem = Problem::LookupByID(id);
		if(problem)
		{
			map<Problem *, vector<Question *> > &questionsByProblem = Question::GetQuestionsByProblem();
			if(questionsByProblem.count(problem))
			{
				vector<Question *> &questions = questionsByProblem[problem];
				unsigned short idx = 0;
				char str[8];
				for(vector<Question *>::iterator it = questions.begin(); it != questions.end(); it++)
				{
					Question *question = *it;
					if(question->IsAnswered())
					{
						idx++;
						memset(str, 0, 8);
						sprintf(str, "%d", idx);
						string idxStr(str);
						string questionKey("question");
						string questionIDKey("questionID");
						string askerKey("asker");
						string answerKey("answer");
						unsigned short qid = question->GetID();
						memset(str, 0, 8);
						sprintf(str, "%d", qid);
						string idStr(str);
						targetVars->operator[](questionKey.append(idxStr)) = question->GetText();
						targetVars->operator[](questionIDKey.append(idxStr)) = idStr;
						targetVars->operator[](askerKey.append(idxStr)) = question->GetTeam()->GetName();
						targetVars->operator[](answerKey.append(idxStr)) = question->GetAnswer();
					}
				}
				memset(str, 0, 8);
				sprintf(str, "%d", idx);
				targetVars->operator[]("questionCount") = string(str);
			}
			else
				targetVars->operator[]("questionCount") = "0";
		}
/*		cout << "Target Map:" << endl;
		for(map<string, string>::iterator it = targetVars->begin(); it != targetVars->end(); it++)
		{
			cout << it->first << " - " << it->second << endl;
		}*/
	}
	void LoadUnansweredQuestionsForProblem(stringstream &stream, Socket *socket, Session *session, string arg, map<string, string> *targetVars)
	{
		unsigned short id = atoi(arg.c_str());
		Problem *problem = Problem::LookupByID(id);
		if(problem)
		{
			map<Problem *, vector<Question *> > &questionsByProblem = Question::GetQuestionsByProblem();
			if(questionsByProblem.count(problem))
			{
				vector<Question *> &questions = questionsByProblem[problem];
				unsigned short idx = 0;
				char str[8];
				for(vector<Question *>::iterator it = questions.begin(); it != questions.end(); it++)
				{
					Question *question = *it;
					if(!question->IsAnswered())
					{
						idx++;
						memset(str, 0, 8);
						sprintf(str, "%d", idx);
						string idxStr(str);
						string questionKey("question");
						string questionIDKey("questionID");
						string askerKey("asker");
						unsigned short qid = question->GetID();
						memset(str, 0, 8);
						sprintf(str, "%d", qid);
						string idStr(str);
						targetVars->operator[](questionKey.append(idxStr)) = question->GetText();
						targetVars->operator[](questionIDKey.append(idxStr)) = idStr;
						targetVars->operator[](askerKey.append(idxStr)) = question->GetTeam()->GetName();
					}
				}
				memset(str, 0, 8);
				sprintf(str, "%d", idx);
				targetVars->operator[]("questionCount") = string(str);
			}
			else
				targetVars->operator[]("questionCount") = "0";
		}
/*		cout << "Target Map:" << endl;
		for(map<string, string>::iterator it = targetVars->begin(); it != targetVars->end(); it++)
		{
			cout << it->first << " - " << it->second << endl;
		}*/
	}

	void Page::RegisterTemplate(string entry, void (*func)(stringstream &, Socket *, Session *, string, map<string, string> *))
	{
		g_templateMap[entry] = func;
	}
	void Page::RegisterDefaultTemplates()
	{
		RegisterTemplate("echo", &Echo);
		RegisterTemplate("teamID", &TeamID);
		RegisterTemplate("teamName", &TeamName);
		RegisterTemplate("loadUnansweredQuestionsForProblem", &LoadUnansweredQuestionsForProblem);
		RegisterTemplate("loadAnsweredQuestionsForProblem", &LoadAnsweredQuestionsForProblem);
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
	void Page::AddToStream(stringstream &stream, Socket *client, Session *session, map<string, string> *GETMap, map<string, string> *masterLocalVars)
	{
		stringstream pageStream(m_html);
		string chunk, varChunk, arg, val, loopTarget;
		string strictArg, strictVal, strictLoopTarget;
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
					strictArg = "";
					pageStream.get();
					while(true)
					{
						char argPeek = pageStream.peek();
						if((argPeek >= 'A' && argPeek <= 'Z') || (argPeek >= 'a' && argPeek <= 'z') || (argPeek >= '0' && argPeek <= '9') ||  argPeek == '.' || argPeek == '/')
						{
							arg.push_back(argPeek);
							strictArg.push_back(argPeek);
							pageStream.get();
						}
						else if(argPeek == '}' && doLoop)
						{
							arg.push_back(argPeek);
							strictArg.push_back(argPeek);
							pageStream.get();
						}
						else if(argPeek == '{')
						{
							pageStream.get();
							strictArg.push_back(argPeek);
							if(doLoop)
								arg.push_back(argPeek);
							else
							{
								string eval;
								while(true)
								{
									char evalPeek = pageStream.peek();
									if((evalPeek >= 'A' && evalPeek <= 'Z') || (evalPeek >= 'a' && evalPeek <= 'z') || (evalPeek >= '0' && evalPeek <= '9') || evalPeek == '.' || evalPeek == '/' || evalPeek == ' ')
									{
										eval.push_back(evalPeek);
										strictArg.push_back(evalPeek);
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
									strictArg.push_back(lastPeek);
								}
								else
								{
									arg.push_back('{');
									arg.append(eval);
								}
							}
						}
						else if(argPeek == '=')
						{
							val = "";
							strictVal = "";
							pageStream.get();
							while(true)
							{
								char valPeek = pageStream.peek();
								if((valPeek >= 'A' && valPeek <= 'Z') || (valPeek >= 'a' && valPeek <= 'z') || (valPeek >= '0' && valPeek <= '9') || valPeek == '.' || valPeek == '/' || valPeek == ' ')
								{
									val.push_back(valPeek);
									strictVal.push_back(valPeek);
									pageStream.get();
								}
								else if(valPeek == '}' && doLoop)
								{
									val.push_back(valPeek);
									strictVal.push_back(valPeek);
									pageStream.get();
								}
								else if(valPeek == '{')
								{
									pageStream.get();
									strictVal.push_back(valPeek);
									if(doLoop)
										val.push_back(valPeek);
									else
									{
										string eval;
										while(true)
										{
											char evalPeek = pageStream.peek();
											if((evalPeek >= 'A' && evalPeek <= 'Z') || (evalPeek >= 'a' && evalPeek <= 'z') || (evalPeek >= '0' && evalPeek <= '9') || evalPeek == '.' || evalPeek == '/' || evalPeek == ' ')
											{
												eval.push_back(evalPeek);
												strictVal.push_back(evalPeek);
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
											strictVal.push_back(lastPeek);
										}
										else
										{
											val.push_back('{');
											val.append(eval);
										}
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
//						cout << "Start Loop: " << strictArg << endl;
						if(arg.size())
						{
							loopTarget = arg;
							strictLoopTarget = strictArg;
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
							if(!arg.compare(strictLoopTarget))
							{
//								cout << "End Loop: " << arg << endl;
								map<unsigned short, Team *>::iterator teamIt;
								map<unsigned short, Team *> &teamsByID = Team::GetTeamsByID();
								map<unsigned short, Problem *>::iterator problemIt;
								map<unsigned short, Problem *> &problemsByID = Problem::GetProblemsByID();
								unsigned short num = 0;
								if(loopTarget.size() > 1)
									num = atoi(&loopTarget.c_str()[1]);
								string numIdx;
								if(num)
									numIdx.push_back(loopTarget[0]);

								bool done = false;
								if(!loopTarget.compare("teams"))
									teamIt = teamsByID.begin();
								else if(!loopTarget.compare("problems"))
									problemIt = problemsByID.begin();
								else if(!num)
									done = true;
								Page *embPage = Page::CreateFromHTML(loopStream.str());
//								cout << "LoopStr: " << loopStream.str() << endl;
								unsigned short idx = 0;
								while(!done)
								{
									idx++;
									char str[8];
									memset(str, 0, 8);
									sprintf(str, "%d", idx);
									targetVars->operator[]("idx") = string(str);
									if(num)
										targetVars->operator[](numIdx) = string(str);
									if(!loopTarget.compare("teams"))
									{
										if(teamIt->first == 0)
											teamIt++;
										if(teamIt != teamsByID.end())
										{
											targetVars->operator[]("teamName") = teamIt->second->GetName();
											memset(str, 0, 8);
											sprintf(str, "%d", teamIt->first);
											targetVars->operator[]("teamID") = string(str);
											memset(str, 0, 8);
											sprintf(str, "%d", idx);
											targetVars->operator[]("teamIdx") = string(str);
											teamIt++;
										}
										if(teamIt == teamsByID.end())
											done = true;
									}
									else if(!loopTarget.compare("problems"))
									{
										targetVars->operator[]("problemName") = problemIt->second->GetName();
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
									else if(num)
										if(idx >= num)
											done = true;
									embPage->AddToStream(stream, client, session, GETMap, targetVars);
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
				else if(!varChunk.compare("GET"))
				{
					if(doLoop)
						loopStream << "$" << varChunk << ":" << arg;
					else
					{
						if(GETMap->count(arg))
							stream << GETMap->operator[](arg);
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
							page->AddToStream(stream, client, session, GETMap, targetVars);
						}
					}
				}
				else if(g_templateMap.count(varChunk))
				{
					if(doLoop)
					{
						loopStream << "$" << varChunk;
						if(arg.size())
							loopStream << ":" << arg;
					}
					else
						g_templateMap[varChunk](stream, client, session, arg, targetVars);
				}
				else if(doLoop)
					loopStream << "$" << varChunk;
				else
					stream << "$" << varChunk;
			}
		}

		if(m_isTemp)
			delete this;
	}
}
