#ifndef _BEACHJUDGE_SUBMISSION_H_
#define _BEACHJUDGE_SUBMISSION_H_

//- Standard Library -
#include <map>
#include <vector>
#include <string>

namespace beachjudge
{
	typedef enum CodeType
	{
		CodeType_Unknown,
		CodeType_C,
		CodeType_CPP,
		CodeType_Java
	} CodeType;

	typedef enum SubStatus
	{
		SubStatus_Pending,
		SubStatus_Accepted,
		SubStatus_NotExecutable,
		SubStatus_TimeLimitExceeded,
		SubStatus_WrongAnswer,
		SubStatus_PresentationError
	} SubStatus;

	class Team;
	class Problem;

	class Submission
	{
		CodeType m_codeType;
		SubStatus m_subStatus, m_autoTestStatus;
		unsigned short m_id;
		Team *m_team;
		Problem *m_problem;
		unsigned long long m_timeMS;
		std::string m_sourceFile, m_base;
		std::map<unsigned short, bool> m_autoTestVerdicts;

		Submission();

	public:
		static Submission *Create(Team *team, Problem *problem, CodeType codeType, unsigned long long timeMS, SubStatus subStatus = SubStatus_Pending, unsigned short id = 0);
		static std::map<unsigned short, Submission *> &GetSubmissionsByID();
		static std::vector<Submission *> *GetPendingSubmissions();
		static Submission *LookupByID(unsigned short id);
		static void Cleanup();
		static std::string GetStatusText(SubStatus status);

		~Submission();

		Team *GetTeam() const;
		Problem *GetProblem() const;
		unsigned long long GetTimeMS() const;
		unsigned short GetID() const;
		CodeType GetCodeType() const;
		std::string GetCodeTypeText() const;
		std::string GetSourceFile() const;
		std::string GetBase() const;
		SubStatus GetStatus() const;
		void SetStatus(SubStatus status);
		SubStatus AutoTest();
		SubStatus GetAutoTestStatus() const;
		std::map<unsigned short, bool> *GetAutoTestVerdicts();
	};
}

#endif
