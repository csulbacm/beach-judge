#include <stdio.h>
#include <fstream>

// beachJudge
#include <Judge/Session.h>
#include <Judge/User.h>
#include <Judge/Persistence.h>
#include <Judge/Problem.h>
#include <Judge/SQL.h>

namespace judge {

void loadJudgeData()
{
	// Load Problem Data
	printf("Loading problem data...\n");
	ProblemSet::SQL_LoadAll();
	Problem::SQL_LoadAll();

	// Load Users & Groups
	printf("Loading user data...\n");
	UserGroup::SQL_LoadAll();
	User::SQL_LoadAll();

	// Default User Data
	if (UserGroup::s_byID.count(0) == 0) {
		printf("Creating global user group...\n");
		(new UserGroup("global", 0))->SQL_Insert();
	}
	//TODO: Refer to 'global' group name map
	if (User::s_byName.count("admin") == 0) {
		printf("Creating admin user...\n");
		(new User("admin", "test", "admin", User::Admin, 0, 0))->SQL_Insert();
	}

	printf("Restoring sessions...\n");
	Session::SQL_LoadAll();
}

}
