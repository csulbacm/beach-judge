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

	class Team;
	class Problem;

	class Submission
	{
		CodeType m_codeType;
		unsigned short m_id;
		Team *m_team;
		Problem *m_problem;
		unsigned long m_timeMS;
		std::string m_sourceFile;

		Submission();

	public:
		static Submission *Create(Team *team, Problem *problem, CodeType codeType, unsigned long timeMS, unsigned short id = 0);
		static std::map<unsigned short, Submission *> &GetSubmissionsByID();
		static Submission *LookupByID(unsigned short id);

		~Submission();

		Team *GetTeam() const;
		Problem *GetProblem() const;
		unsigned long GetTimeMS() const;
		unsigned short GetID() const;
		CodeType GetCodeType() const;
		std::string GetSourceFile() const;
	};
}

#endif
