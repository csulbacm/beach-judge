#ifndef _JUDGE_PROBLEM_H_
#define _JUDGE_PROBLEM_H_

#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <string.h>
#include <map>
#include <vector>
#include <algorithm>

// beachJudge
#include <Judge/SQL.h>
#include <Judge/Types.h>

namespace judge {

typedef struct Problem Problem;
typedef struct ProblemSet ProblemSet;
typedef struct Submission Submission;
typedef struct TestCase TestCase;
typedef struct Score Score;


//------------------------------------------------------
//-------------------- ProblemSet ----------------------

struct ProblemSet
{
	// ID
	// Name
	// StartTime
	// Duration
	// 
	// Problems
	// Messages

	static std::map<u16, ProblemSet *> s_byID;
	static std::map<std::string, ProblemSet *> s_byName;

	ProblemSet()
	{
	}

	ProblemSet(const char *name, u16 id = 0xFFFF) :
		name(name)
	{
		if (id == 0xFFFF) {
			id = 0;
			do ++id;
			while (s_byID.count(id));
		}
		this->id = id;
		s_byID[id] = this;
		s_byName[name] = this;
	}

	~ProblemSet()
	{
		//TODO: Verify this is necessary, might have to explicity remove
		if (s_byID.count(id))
			s_byID[id] = 0;
		if (s_byName.count(name))
			s_byName[name] = 0;
	}

	void Purge()
	{
		if (s_byID.count(id))
			s_byID.erase(id);
		if (s_byName.count(name))
			s_byName.erase(name);
	}


	//-------------------- ID ----------------------

	u16 id;


	//------------------- Name ---------------------

	std::string name;


	//------------------- SQL ----------------------

	inline void SQL_Insert()
	{
		sqlite3_bind_int(SQL::problemSet_insert,
			1, id);
		sqlite3_bind_text(SQL::problemSet_insert,
			2, name.c_str(), name.length(), 0);
		sqlite3_step(SQL::problemSet_insert);
		sqlite3_reset(SQL::problemSet_insert);
	}

	inline void SQL_Sync()
	{
		sqlite3_bind_text(SQL::problemSet_update,
			1, name.c_str(), name.length(), 0);
		sqlite3_bind_int(SQL::problemSet_update,
			2, id);
		sqlite3_step(SQL::problemSet_update);
		sqlite3_reset(SQL::problemSet_update);
	}

	inline void SQL_Delete()
	{
		//TODO: Delete all associated users
		sqlite3_bind_int(SQL::problemSet_delete,
			1, id);
		sqlite3_step(SQL::problemSet_delete);
		sqlite3_reset(SQL::problemSet_delete);
	}

	static inline void SQL_LoadAll()
	{
		const char *_name;
		u16 _id;
		while (sqlite3_step(SQL::problemSet_selectAll)
				!= SQLITE_DONE) {
			_id = sqlite3_column_int(SQL::problemSet_selectAll, 0);
			_name = (const char *)sqlite3_column_text(SQL::problemSet_selectAll, 1);

			new ProblemSet(_name, _id);
		}
		sqlite3_reset(SQL::problemSet_selectAll);
	}
};


//------------------------------------------------------
//--------------------- Problem ------------------------

struct Problem
{
	// ID
	// Name
	// Description
	// SampleTests
	// ProblemSet
	//
	// Submissions
	// Scores
	// TestCases

};


//------------------------------------------------------
//-------------------- Submission ----------------------

struct Submission
{

};


//------------------------------------------------------
//---------------------- Score -------------------------

struct Score
{

};


//------------------------------------------------------
//--------------------- TestCase -----------------------

struct TestCase
{

};


}

#endif
