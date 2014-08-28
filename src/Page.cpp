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
#include <BeachJudge/Competition.h>

using namespace std;

const char *includePrefix = "../www/";

#ifdef _WIN32
	#define SPRINTF sprintf_s
#else
	#define SPRINTF	sprintf
#endif

namespace beachjudge
{
	map<string, Page *> g_pageMap;
	map<string, void (*)(stringstream &, Socket *, Session *, string, map<string, string> *)> g_templateMap;

	void TimeLeft(stringstream &stream, Socket *socket, Session *session, string arg, map<string, string> *targetVars)
	{
		Competition *competition = Competition::GetCurrent();
		if(competition)
		{
			if(competition->IsRunning())
				stream << competition->GetTimeLeft() / 1000;
			else
				stream << 0;
		}
		else
			stream << 0;
	}
	void TeamTotalScore(stringstream &stream, Socket *socket, Session *session, string arg, map<string, string> *targetVars)
	{
		Team *team = session->GetTeam();
		if(team)
			stream << team->GetTotalScore();
		else
			stream << 0;
	}
	void TeamTotalPenalties(stringstream &stream, Socket *socket, Session *session, string arg, map<string, string> *targetVars)
	{
		Team *team = session->GetTeam();
		if(team)
			stream << team->GetTotalPenalties();
		else
			stream << 0;
	}
	void Duration(stringstream &stream, Socket *socket, Session *session, string arg, map<string, string> *targetVars)
	{
		Competition *competition = Competition::GetCurrent();
		if(competition)
			stream << competition->GetDuration() / 1000;
		else
			stream << 1;
	}
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
	void LoadSubmission(stringstream &stream, Socket *socket, Session *session, string arg, map<string, string> *targetVars)
	{
		unsigned short id = atoi(arg.c_str());
		Team *team = 0;
		if(session)
			team = session->GetTeam();
		Submission *submission = Submission::LookupByID(id);
		if(submission)
		{
			if(team == submission->GetTeam() || team->IsJudge())
			{
				Problem *problem = submission->GetProblem();
				char str[8];
				memset(str, 0, 8);
				SPRINTF(str, "%d", submission->GetID());
				targetVars->operator[]("loadedSubmissionID") = string(str);
				memset(str, 0, 8);
				SPRINTF(str, "%d", problem->GetID());
				targetVars->operator[]("loadedSubmissionProblemID") = string(str);
				targetVars->operator[]("loadedSubmissionProblemName") = problem->GetName();
				memset(str, 0, 8);
				SPRINTF(str, "%d", submission->GetTeam()->GetID());
				targetVars->operator[]("loadedSubmissionTeamID") = string(str);
				targetVars->operator[]("loadedSubmissionTeamName") = submission->GetTeam()->GetName();
				targetVars->operator[]("loadedSubmissionStatus") = Submission::GetStatusText(submission->GetStatus());
				targetVars->operator[]("loadedSubmissionCodeType") = submission->GetCodeTypeText();
			}
		}
	}
	void LoadProblem(stringstream &stream, Socket *socket, Session *session, string arg, map<string, string> *targetVars)
	{
		unsigned short id = atoi(arg.c_str());
		Team *team = 0;
		if(session)
			team = session->GetTeam();
		Problem *problem = Problem::LookupByID(id);
		if(problem)
		{
			char str[8];
			memset(str, 0, 8);
			SPRINTF(str, "%d", problem->GetID());
			targetVars->operator[]("loadedProblemID") = string(str);
			targetVars->operator[]("loadedProblemName") = problem->GetName();
		}
	}
	void GetCode(stringstream &stream, Socket *socket, Session *session, string arg, map<string, string> *targetVars)
	{
		unsigned short id = atoi(arg.c_str());
		Team *team = 0;
		if(session)
			team = session->GetTeam();
		Submission *submission = Submission::LookupByID(id);
		if(submission)
		{
			string file = submission->GetSourceFile(), in;
			if(fileExists(file.c_str()))
			{
				ifstream inFile(file.c_str(), ios::in | ios::binary);
				while(getline(inFile, in))
				{
					for(string::iterator it = in.begin(); it != in.end(); it++)
					{
						char &c = *it;
						if((c >= '0' && c <= '9') || (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == ' ')
							stream.put(c);
						else if((c >= '!' && c <= '/') || (c >= ':' && c <= '@') || (c >= '[' && c <= '`') || (c >= '{' && c <= '~') || c == '\t')
						{
							char part[8];
							memset(part, 0, 8);
							SPRINTF(part, "&#%d;", c);
							stream << part;
						}
					}
					stream << "<br/>";
				}
			}
		}
	}
	void LoadQuestion(stringstream &stream, Socket *socket, Session *session, string arg, map<string, string> *targetVars)
	{
		unsigned short id = atoi(arg.c_str());
		Team *team = 0;
		if(session)
			team = session->GetTeam();
		Question *question = Question::LookupByID(id);
		if(question)
		{
			if(question->IsAnswered() || team == question->GetTeam() || team->IsJudge())
			{
				Problem *problem = question->GetProblem();
				char str[8];
				memset(str, 0, 8);
				SPRINTF(str, "%d", question->GetID());
				targetVars->operator[]("question") = question->GetText();
				targetVars->operator[]("questionID") = string(str);
				targetVars->operator[]("asker") = question->GetTeam()->GetName();
				targetVars->operator[]("answer") = question->GetAnswer();
				memset(str, 0, 8);
				SPRINTF(str, "%d", problem->GetID());
				targetVars->operator[]("questionProblemID") = string(str);
				targetVars->operator[]("questionProblemName") = problem->GetName();
			}
		}
	}
	void LoadTeam(stringstream &stream, Socket *socket, Session *session, string arg, map<string, string> *targetVars)
	{
		unsigned short id = atoi(arg.c_str());
		if(id == 0)
			return;
		Team *team = Team::LookupByID(id);
		if(team)
		{
			char str[8];
			memset(str, 0, 8);
			SPRINTF(str, "%d", team->GetID());
			targetVars->operator[]("loadedTeamName") = team->GetName();
			targetVars->operator[]("loadedTeamID") = string(str);
		}
	}
	void LoadProblemScores(stringstream &stream, Socket *socket, Session *session, string arg, map<string, string> *targetVars)
	{
		unsigned short id = atoi(arg.c_str());
		Problem *problem = Problem::LookupByID(id);
		if(problem)
		{
			char str[16];
			memset(str, 0, 16);
			SPRINTF(str, "%d", id - 1);
			string probIdxStr(str);
			vector<Team *> *solvers = problem->GetSolvers();
			if(solvers->size())
			{
				unsigned short idx = 0;
				for(vector<Team *>::iterator itB = solvers->begin(); itB != solvers->end(); itB++)
				{
					Team *team = *itB;
					idx++;
					memset(str, 0, 16);
					SPRINTF(str, "%d", idx);
					string idxStr(str);
					string teamNameKey("teamName");
					string teamIDKey("teamID");
					string scoreKey("score");
					string penaltyKey("penalties");
					unsigned short tID = team->GetID();
					memset(str, 0, 16);
					SPRINTF(str, "%d", tID);
					string idStr(str);
					targetVars->operator[](teamNameKey.append(idxStr)) = team->GetName();
					targetVars->operator[](teamIDKey.append(idxStr)) = idStr;
					float score = team->GetScore(problem);
					memset(str, 0, 16);
					SPRINTF(str, "%0.2f", score);
					targetVars->operator[](scoreKey.append(idxStr)) = string(str);
					memset(str, 0, 16);
					if(score == 0.f)
						SPRINTF(str, "0");
					else
						SPRINTF(str, "%d", team->GetPenalties(problem));
					targetVars->operator[](penaltyKey.append(idxStr)) = string(str);
				}
				memset(str, 0, 16);
				SPRINTF(str, "%d", (unsigned short)solvers->size());
				targetVars->operator[]("solverCount") = string(str);
			}
			else
				targetVars->operator[]("solverCount") = "0";
		}
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
						SPRINTF(str, "%d", idx);
						string idxStr(str);
						string questionKey("question");
						string questionIDKey("questionID");
						string askerKey("asker");
						string answerKey("answer");
						unsigned short qid = question->GetID();
						memset(str, 0, 8);
						SPRINTF(str, "%d", qid);
						string idStr(str);
						targetVars->operator[](questionKey.append(idxStr)) = question->GetText();
						targetVars->operator[](questionIDKey.append(idxStr)) = idStr;
						targetVars->operator[](askerKey.append(idxStr)) = question->GetTeam()->GetName();
						targetVars->operator[](answerKey.append(idxStr)) = question->GetAnswer();
					}
				}
				memset(str, 0, 8);
				SPRINTF(str, "%d", idx);
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
						Problem *problem = question->GetProblem();
						idx++;
						memset(str, 0, 8);
						SPRINTF(str, "%d", idx);
						string idxStr(str);
						string questionKey("question");
						string questionIDKey("questionID");
						string askerKey("asker");
						string problemKey("problemName");
						string problemIDKey("problemID");
						unsigned short qid = question->GetID();
						memset(str, 0, 8);
						SPRINTF(str, "%d", qid);
						string idStr(str);
						targetVars->operator[](questionKey.append(idxStr)) = question->GetText();
						targetVars->operator[](questionIDKey.append(idxStr)) = idStr;
						targetVars->operator[](askerKey.append(idxStr)) = question->GetTeam()->GetName();
						targetVars->operator[](problemKey.append(idxStr)) = problem->GetName();
						memset(str, 0, 8);
						SPRINTF(str, "%d", problem->GetID());
						targetVars->operator[](problemIDKey.append(idxStr)) = string(str);
					}
				}
				memset(str, 0, 8);
				SPRINTF(str, "%d", idx);
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
	void LoadTestSets(stringstream &stream, Socket *socket, Session *session, string arg, map<string, string> *targetVars)
	{
		unsigned short id = atoi(arg.c_str());
		Problem *problem = Problem::LookupByID(id);
		if(!problem)
			return;

		map<unsigned short, Problem::TestSet *> *testSets = problem->GetTestSets();
		if(testSets->size())
		{
			unsigned short idx = 0;
			char str[8];
			for(map<unsigned short, Problem::TestSet *>::iterator it = testSets->begin(); it != testSets->end(); it++)
			{
				Problem::TestSet *testSet = it->second;
				idx++;
				memset(str, 0, 8);
				SPRINTF(str, "%d", idx);
				string idxStr(str);
				string testSetNameKey("testSetName");
				string testSetIDKey("testSetID");
				unsigned short tid = testSet->GetID();
				memset(str, 0, 8);
				SPRINTF(str, "%d", tid);
				string idStr(str);
				targetVars->operator[](testSetNameKey.append(idxStr)) = testSet->GetName();
				targetVars->operator[](testSetIDKey.append(idxStr)) = idStr;
			}
			memset(str, 0, 8);
			SPRINTF(str, "%d", idx);
			targetVars->operator[]("testSetCount") = string(str);
		}
		else
			targetVars->operator[]("testSetCount") = "0";
	}
	void LoadAutoTestData(stringstream &stream, Socket *socket, Session *session, string arg, map<string, string> *targetVars)
	{
		unsigned short id = atoi(arg.c_str());
		Submission *submission = Submission::LookupByID(id);
		if(!submission)
			return;
		targetVars->operator[]("testStatus") = Submission::GetStatusText(submission->GetAutoTestStatus());

		Problem *problem = submission->GetProblem();
		map<unsigned short, bool> *verdicts = submission->GetAutoTestVerdicts();
		map<unsigned short, Problem::TestSet *> *testSets = problem->GetTestSets();
		if(testSets->size())
		{
			unsigned short idx = 0;
			char str[8];
			for(map<unsigned short, Problem::TestSet *>::iterator it = testSets->begin(); it != testSets->end(); it++)
			{
				Problem::TestSet *testSet = it->second;
				idx++;
				memset(str, 0, 8);
				SPRINTF(str, "%d", idx);
				string idxStr(str);
				string testSetNameKey("testSetName");
				string testSetIDKey("testSetID");
				string testSetVerdictKey("testSetVerdict");
				unsigned short tid = testSet->GetID();
				memset(str, 0, 8);
				SPRINTF(str, "%d", tid);
				string idStr(str);
				targetVars->operator[](testSetNameKey.append(idxStr)) = testSet->GetName();
				targetVars->operator[](testSetIDKey.append(idxStr)) = idStr;
				if(!verdicts->count(tid))
					targetVars->operator[](testSetVerdictKey.append(idxStr)) = "-";
				else if(verdicts->operator[](tid))
					targetVars->operator[](testSetVerdictKey.append(idxStr)) = "Success";
				else
					targetVars->operator[](testSetVerdictKey.append(idxStr)) = "Wrong Answer";
			}
			memset(str, 0, 8);
			SPRINTF(str, "%d", idx);
			targetVars->operator[]("testSetCount") = string(str);
		}
		else
			targetVars->operator[]("testSetCount") = "0";
	}

	void Page::RegisterTemplate(string entry, void (*func)(stringstream &, Socket *, Session *, string, map<string, string> *))
	{
		g_templateMap[entry] = func;
	}
	void Page::RegisterDefaultTemplates()
	{
		RegisterTemplate("echo", &Echo);
		RegisterTemplate("timeLeft", &TimeLeft);
		RegisterTemplate("duration", &Duration);
		RegisterTemplate("getCode", &GetCode);
		RegisterTemplate("teamName", &TeamName);
		RegisterTemplate("teamTotalScore", &TeamTotalScore);
		RegisterTemplate("teamTotalPenalties", &TeamTotalPenalties);
		RegisterTemplate("loadTeam", &LoadTeam);
		RegisterTemplate("loadQuestion", &LoadQuestion);
		RegisterTemplate("loadSubmission", &LoadSubmission);
		RegisterTemplate("loadProblem", &LoadProblem);
		RegisterTemplate("loadUnansweredQuestionsForProblem", &LoadUnansweredQuestionsForProblem);
		RegisterTemplate("loadAnsweredQuestionsForProblem", &LoadAnsweredQuestionsForProblem);
		RegisterTemplate("loadTestSets", &LoadTestSets);
		RegisterTemplate("loadAutoTestData", &LoadAutoTestData);
		RegisterTemplate("loadProblemScores", &LoadProblemScores);
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
		page->m_isTemp = true; //TODO: Remove this for caching
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
		Team *team = 0;
		if(session)
			team = session->GetTeam();
		stringstream pageStream(m_html);
		string chunk, varChunk, arg, val, loopTarget;
		string strictArg, strictVal, strictLoopTarget;
		stringstream loopStream;
		vector<pair<unsigned char, string> > ifStack;
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
					for(vector<pair<unsigned char, string> >::iterator it = ifStack.begin(); it != ifStack.end(); it++)
					{
						unsigned char ifType = it->first;
						if(ifType == 0)
						{
							if(session->GetVariable(it->second) == 0)
							{
								valid = false;
								break;
							}
						}
						else if(ifType == 'n')
						{
							if(!it->second.compare("0"))
							{
								valid = false;
								break;
							}
						}
						else if(ifType == 'z')
						{
							if(it->second.compare("0"))
							{
								valid = false;
								break;
							}
						}
						else if(ifType == 'u')
						{
							if(it->second.size())
							{
								valid = false;
								break;
							}
						}
						else if(ifType == 'd')
						{
							if(!it->second.size())
							{
								valid = false;
								break;
							}
						}
					}
				}
				else if(ifStack.front().second.compare("loggedOut"))
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
						if((argPeek >= 'A' && argPeek <= 'Z') || (argPeek >= 'a' && argPeek <= 'z') || (argPeek >= '0' && argPeek <= '9') ||  argPeek == '.' || argPeek == '/' || argPeek == '_')
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
									if((evalPeek >= 'A' && evalPeek <= 'Z') || (evalPeek >= 'a' && evalPeek <= 'z') || (evalPeek >= '0' && evalPeek <= '9') || evalPeek == '.' || evalPeek == '/' || evalPeek == ' ' || evalPeek == '_')
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
									if(eval.find("GET_") == 0)
									{
										eval = eval.substr(4);
										if(GETMap->count(eval))
										{
											eval = GETMap->operator[](eval);
											arg.append(eval);
										}
									}
									else if(targetVars->count(eval))
									{
										eval = targetVars->operator[](eval);
										arg.append(eval);
									}
/*									else
									{
										arg.push_back('{');
										arg.append(eval);
										arg.push_back('}');
									}
*/									strictArg.push_back(lastPeek);
								}
/*								else
								{
									arg.push_back('{');
									arg.append(eval);
								}
*/							}
						}
						else if(argPeek == '=')
						{
							val = "";
							strictVal = "";
							pageStream.get();
							while(true)
							{
								char valPeek = pageStream.peek();
								if((valPeek >= 'A' && valPeek <= 'Z') || (valPeek >= 'a' && valPeek <= 'z') || (valPeek >= '0' && valPeek <= '9') || valPeek == '.' || valPeek == '/' || valPeek == '_')
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
											if((evalPeek >= 'A' && evalPeek <= 'Z') || (evalPeek >= 'a' && evalPeek <= 'z') || (evalPeek >= '0' && evalPeek <= '9') || evalPeek == '.' || evalPeek == '/' || evalPeek == ' ' || evalPeek == '_')
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
											
											if(eval.find("GET_") == 0)
											{
												eval = eval.substr(4);
												if(GETMap->count(eval))
												{
													eval = GETMap->operator[](eval);
													val.append(eval);
												}
											}
											else if(targetVars->count(eval))
											{
												eval = targetVars->operator[](eval);
												val.append(eval);
											}
/*											else
											{
												val.push_back('{');
												val.append(eval);
												val.push_back('}');
											}
*/											strictVal.push_back(lastPeek);
										}
/*										else
										{
											val.push_back('{');
											val.append(eval);
										}
*/									}
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
					ifStack.push_back(pair<unsigned char, string>(0, arg));
			}
			else if(!varChunk.compare("endif"))
			{
				if(doLoop)
					loopStream << "$" << varChunk << ":" << arg;
				else
					ifStack.erase(find(ifStack.begin(), ifStack.end(), pair<unsigned char, string>(0, arg)));
			}
			else if(!varChunk.compare("ifz"))
			{
				if(doLoop)
					loopStream << "$" << varChunk << ":" << arg;
				else
					ifStack.push_back(pair<unsigned char, string>('z', arg));
			}
			else if(!varChunk.compare("endifz"))
			{
				if(doLoop)
					loopStream << "$" << varChunk << ":" << arg;
				else
					ifStack.erase(find(ifStack.begin(), ifStack.end(), pair<unsigned char, string>('z', arg)));
			}
			else if(!varChunk.compare("ifnz"))
			{
				if(doLoop)
					loopStream << "$" << varChunk << ":" << arg;
				else
					ifStack.push_back(pair<unsigned char, string>('n', arg));
			}
			else if(!varChunk.compare("endifnz"))
			{
				if(doLoop)
					loopStream << "$" << varChunk << ":" << arg;
				else
					ifStack.erase(find(ifStack.begin(), ifStack.end(), pair<unsigned char, string>('n', arg)));
			}
			else if(!varChunk.compare("ifu"))
			{
				if(doLoop)
					loopStream << "$" << varChunk << ":" << arg;
				else
					ifStack.push_back(pair<unsigned char, string>('u', arg));
			}
			else if(!varChunk.compare("endifu"))
			{
				if(doLoop)
					loopStream << "$" << varChunk << ":" << arg;
				else
					ifStack.erase(find(ifStack.begin(), ifStack.end(), pair<unsigned char, string>('u', arg)));
			}
			else if(!varChunk.compare("ifd"))
			{
				if(doLoop)
					loopStream << "$" << varChunk << ":" << arg;
				else
					ifStack.push_back(pair<unsigned char, string>('d', arg));
			}
			else if(!varChunk.compare("endifd"))
			{
				if(doLoop)
					loopStream << "$" << varChunk << ":" << arg;
				else
					ifStack.erase(find(ifStack.begin(), ifStack.end(), pair<unsigned char, string>('d', arg)));
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
								vector<Submission *>::iterator submissionIt;
								vector<Submission *> *submissions = 0;
								vector<Team *> *teamVec = 0;
								vector<Team *>::iterator teamVecIt;
								unsigned short num = 0;
								if(loopTarget.size() > 1)
									num = atoi(&loopTarget.c_str()[1]);
								string numIdx;
								if(num)
									numIdx.push_back(loopTarget[0]);

								bool done = false;
								if(!loopTarget.compare("teams"))
									teamIt = teamsByID.begin();
								else if(!loopTarget.compare("teamsByScore"))
								{
									teamVec = Team::GetTeamsByScore();
									teamVecIt = teamVec->begin();
								}
								else if(!loopTarget.compare("teamsByName"))
								{
									teamVec = Team::GetTeamsByName();
									teamVecIt = teamVec->begin();
								}
								else if(!loopTarget.compare("problems"))
									problemIt = problemsByID.begin();
								else if(!loopTarget.compare("mySubmissions"))
								{
									if(team)
									{
										submissions = team->GetSubmissions();
										if(submissions->size())
											submissionIt = submissions->begin();
										else
											done = true;
									}
									else
										done = true;
								}
								else if(!loopTarget.compare("pendingSubmissions"))
								{
									if(team)
									{
										if(team->IsJudge())
										{
											submissions = Submission::GetPendingSubmissions();
											if(submissions->size())
												submissionIt = submissions->begin();
											else
												done = true;
										}
										else
											done = true;
									}
									else
										done = true;
								}
								else if(!num)
									done = true;
								Page *embPage = Page::CreateFromHTML(loopStream.str());
//								cout << "LoopStr: " << loopStream.str() << endl;
								unsigned short idx = 0;
								while(!done)
								{
									idx++;
									char str[16];
									memset(str, 0, 16);
									SPRINTF(str, "%d", idx);
									targetVars->operator[]("idx") = string(str);
									if(num)
										targetVars->operator[](numIdx) = string(str);
									if(!loopTarget.compare("teams"))
									{
										if(teamIt->first == 0)
											teamIt++;
										if(teamIt != teamsByID.end())
										{
											Team *tTeam = teamIt->second;
											targetVars->operator[]("teamName") = tTeam->GetName();
											memset(str, 0, 16);
											SPRINTF(str, "%d", teamIt->first);
											targetVars->operator[]("teamID") = string(str);
											memset(str, 0, 16);
											SPRINTF(str, "%d", idx);
											targetVars->operator[]("teamIdx") = string(str);
											memset(str, 0, 16);
											SPRINTF(str, "%0.2f", tTeam->GetTotalScore());
											targetVars->operator[]("teamTotalScore") = string(str);
											memset(str, 0, 16);
											SPRINTF(str, "%d", tTeam->GetTotalPenalties());
											targetVars->operator[]("teamTotalPenalties") = string(str);
											memset(str, 0, 16);
											SPRINTF(str, "%d", tTeam->GetNumSolutions());
											targetVars->operator[]("teamNumSolutions") = string(str);
											teamIt++;
										}
										if(teamIt == teamsByID.end())
											done = true;
									}
									else if(!loopTarget.compare("teamsByScore") || !loopTarget.compare("teamsByName"))
									{
										if(teamVecIt != teamVec->end())
										{
											Team *tTeam = *teamVecIt;
											targetVars->operator[]("teamName") = tTeam->GetName();
											memset(str, 0, 16);
											SPRINTF(str, "%d", tTeam->GetID());
											targetVars->operator[]("teamID") = string(str);
											memset(str, 0, 16);
											SPRINTF(str, "%d", idx);
											targetVars->operator[]("teamIdx") = string(str);
											memset(str, 0, 16);
											SPRINTF(str, "%0.2f", tTeam->GetTotalScore());
											targetVars->operator[]("teamTotalScore") = string(str);
											memset(str, 0, 16);
											SPRINTF(str, "%d", tTeam->GetTotalPenalties());
											targetVars->operator[]("teamTotalPenalties") = string(str);
											memset(str, 0, 16);
											SPRINTF(str, "%d", tTeam->GetNumSolutions());
											targetVars->operator[]("teamNumSolutions") = string(str);
											teamVecIt++;
										}
										if(teamVecIt == teamVec->end())
											done = true;
									}
									else if(!loopTarget.compare("problems"))
									{
										targetVars->operator[]("problemName") = problemIt->second->GetName();
										memset(str, 0, 16);
										SPRINTF(str, "%d", problemIt->first);
										targetVars->operator[]("problemID") = string(str);
										memset(str, 0, 16);
										SPRINTF(str, "%d", idx);
										targetVars->operator[]("problemIdx") = string(str);
										problemIt++;
										if(problemIt == problemsByID.end())
											done = true;
									}
									else if((!loopTarget.compare("mySubmissions") || !loopTarget.compare("pendingSubmissions")) && team)
									{
										Submission *submission = *submissionIt;
										Problem *problem = submission->GetProblem();
										Team *subTeam = submission->GetTeam();
										memset(str, 0, 16);
										SPRINTF(str, "%d", idx);
										string idxStr(str);
										unsigned short sid = submission->GetID();
										memset(str, 0, 16);
										SPRINTF(str, "%d", sid);
										targetVars->operator[]("submissionID") = string(str);
										memset(str, 0, 16);
										SPRINTF(str, "%lld", submission->GetTimeMS());
										targetVars->operator[]("submissionTime") = string(str);
										memset(str, 0, 16);
										SPRINTF(str, "%lld", submission->GetTimeMS());
										targetVars->operator[]("submissionProblemName") = problem->GetName();
										memset(str, 0, 16);
										SPRINTF(str, "%d", problem->GetID());
										targetVars->operator[]("submissionProblemID") = string(str);
										targetVars->operator[]("submissionTeamName") = subTeam->GetName();
										memset(str, 0, 16);
										SPRINTF(str, "%d", subTeam->GetID());
										targetVars->operator[]("submissionTeamID") = string(str);
										targetVars->operator[]("submissionStatus") = Submission::GetStatusText(submission->GetStatus());
										targetVars->operator[]("submissionCodeType") = submission->GetCodeTypeText();
										submissionIt++;
										if(submissionIt == submissions->end())
											done = true;
									}
									else if(num)
										if(idx >= num)
											done = true;
									embPage->AddToStream(stream, client, session, GETMap, targetVars);
								}
								delete embPage;
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
//						else
//							stream << "$" << varChunk << ":" << arg;
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
//						else
//							stream << "$" << varChunk << ":" << arg;
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
