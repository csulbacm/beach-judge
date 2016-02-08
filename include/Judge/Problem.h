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
	// Status
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

	ProblemSet(const char *name, u16 id, u16 status, u64 startTime, u16 duration) :
		name(name),
		status(status),
		startTime(startTime),
		duration(duration)
	{
		if (id == 0xFFFF) {
			id = 0;
			do ++id;
			while (s_byID.count(id));
		}
		this->id = id;
		s_byID[id] = this;
		s_byName[name] = this;
		//TODO: Add to pending queue if pending
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


	//------------------ Status --------------------

	typedef enum ProblemSetStatuses {
		Hidden,
		Pending,
		Closed
	} ProblemSetStatuses;

	u8 status;


	//------------------- Time ---------------------

	u64 startTime, endTime;

	u32 duration;


	//------------------- SQL ----------------------

	inline void SQL_Insert()
	{
		std::string startStr = TimeStampToDateTime(startTime);
		sqlite3_bind_int(SQL::problemSet_insert,
			1, id);
		sqlite3_bind_text(SQL::problemSet_insert,
			2, name.c_str(), name.length(), 0);
		sqlite3_bind_int(SQL::problemSet_insert,
			3, status);
		sqlite3_bind_text(SQL::problemSet_insert,
			4, startStr.c_str(), startStr.length(), 0);
		sqlite3_bind_int(SQL::problemSet_insert,
			5, duration);
		sqlite3_step(SQL::problemSet_insert);
		sqlite3_reset(SQL::problemSet_insert);
	}

	inline void SQL_Sync()
	{
		std::string startStr = TimeStampToDateTime(startTime);
		sqlite3_bind_text(SQL::problemSet_update,
			1, name.c_str(), name.length(), 0);
		sqlite3_bind_int(SQL::problemSet_update,
			2, status);
		sqlite3_bind_text(SQL::problemSet_update,
			3, startStr.c_str(), startStr.length(), 0);
		sqlite3_bind_int(SQL::problemSet_update,
			4, duration);
		sqlite3_bind_int(SQL::problemSet_update,
			5, id);
		sqlite3_step(SQL::problemSet_update);
		sqlite3_reset(SQL::problemSet_update);
	}

	inline void SQL_Delete()
	{
		//TODO: Delete all associated problems
		sqlite3_bind_int(SQL::problemSet_delete,
			1, id);
		sqlite3_step(SQL::problemSet_delete);
		sqlite3_reset(SQL::problemSet_delete);
	}

	static inline void SQL_LoadAll()
	{
		const char *_name, *_startTime;
		u16 _id, _status, _duration;
		while (sqlite3_step(SQL::problemSet_selectAll)
				!= SQLITE_DONE) {
			_id = sqlite3_column_int(SQL::problemSet_selectAll, 0);
			_name = (const char *)sqlite3_column_text(SQL::problemSet_selectAll, 1);
			_status = sqlite3_column_int(SQL::problemSet_selectAll, 2);
			_startTime = (const char *)sqlite3_column_text(SQL::problemSet_selectAll, 3);
			_duration = sqlite3_column_int(SQL::problemSet_selectAll, 4);

			new ProblemSet(_name, _id, _status, DateTimeToTimeStamp(_startTime), _duration);
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
