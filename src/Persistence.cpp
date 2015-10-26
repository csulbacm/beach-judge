#include <stdio.h>
#include <fstream>

// beachJudge
#include <Judge/Session.h>
#include <Judge/User.h>
#include <Judge/Persistence.h>

namespace judge {

void loadJudgeData()
{
	// Default User Data
	if (UserGroup::s_groupsByID.count(0) == 0) {
		printf("Creating global user group...\n");
		new UserGroup("Global", 0);
	}
	if (User::s_usersByName.count("admin") == 0) {
		printf("Creating admin user...\n");
		new User("admin", "test", 0, true, 0);
	}

	printf("Loading user sessions...\n");
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
