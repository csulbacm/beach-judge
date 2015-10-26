#include <stdio.h>
#include <fstream>

// beachJudge
#include <Judge/Session.h>
#include <Judge/User.h>
#include <Judge/Persistence.h>
#include <Judge/SQL.h>

namespace judge {

void loadJudgeData()
{
	// Load Users & Groups
	printf("Loading user data...\n");
	UserGroup::SQL_LoadAll();
	User::SQL_LoadAll();

	// Default User Data
	if (UserGroup::s_groupsByID.count(0) == 0) {
		printf("Creating global user group...\n");
		(new UserGroup("Global", 0))->SQL_Insert();
	}
	if (User::s_usersByName.count("admin") == 0) {
		printf("Creating admin user...\n");
		(new User("admin", "test", "admin", User::Admin, 0, 0))->SQL_Insert();
	}

	printf("Restoring sessions...\n");
	Session::SQL_LoadAll();
	return;
	{
		ifstream file(".sessions");
		if (file.is_open()) {
			string sessID, name;
			u64 expireTimeMS;
			while (file >> sessID >> name >> expireTimeMS) {
				if (User::s_usersByName.count(name) == 0)
					continue;
				Session::s_sessionMap[sessID] = Session(sessID.c_str(), User::s_usersByName[name], expireTimeMS);
			}
			file.close();
		}
	}
}

void saveJudgeData()
{
	printf("Saving user sessions...\n");
	{
		ofstream file;
		file.open(".sessions");
		std::map<std::string, Session>::iterator it = Session::s_sessionMap.begin();
		std::map<std::string, Session>::iterator end = Session::s_sessionMap.end();
		while (it != end) {
			file << it->first
				<< ' ' << it->second.user->name.c_str() 
				<< ' ' << it->second.expireTimeMS
				<< '\n';
			++it;
		}
		file.close();
	}
}

}
