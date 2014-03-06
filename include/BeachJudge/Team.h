#ifndef _BEACHJUDGE_TEAM_H_
#define _BEACHJUDGE_TEAM_H_

//- Standard Library -
#include <string>
#include <cstdlib>
#include <map>
#include <vector>

//- Beach Judge -
#include <BeachJudge/Socket.h>
#include <BeachJudge/Problem.h>

namespace beachjudge
{
	class Team
	{
		unsigned short m_id, m_totalPenalties, m_numActiveSubmissions, m_numSolutions;
		float m_totalScore;
		std::string m_name, m_password;
		bool m_isJudge;
		std::vector<Submission *> m_submissions;
		std::map<Problem *, float> m_scores;
		std::map<Problem *, unsigned short> m_penalties;

		Team();

	public:
		static Team *Create(std::string name, std::string password, unsigned short id = 0, bool isJudge = false);
		static Team *LookupByID(unsigned short id);
		static Team *LookupByName(std::string name);
		static void SetDatabase(std::string file);
		static void SaveToDatabase();
		static void LoadFromDatabase();
		static void DeleteAll();
		static std::map<unsigned short, Team *> &GetTeamsByID();
		static void SaveScores();
		static void LoadScores();

		~Team();

		void SetPassword(std::string password);
		bool TestPassword(std::string password);
		void SetName(std::string name);
		std::string GetName() const;
		unsigned short GetID() const;
		unsigned short GetTotalPenalties() const;
		unsigned short GetNumSolutions() const;
		float GetTotalScore() const;
		bool IsJudge() const;
		std::vector<Submission *> *GetSubmissions();
		void AddSubmission(Submission *submission);
		void RemoveSubmission(Submission *submission);
		unsigned short GetNumActiveSubmissions() const;
		float GetScore(Problem *problem);
		void AddScore(Problem *problem, float score);
		unsigned short GetPenalties(Problem *problem);
		void AddPenalty(Problem *problem);
	};
}

#endif
