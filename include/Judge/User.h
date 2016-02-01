#ifndef _JUDGE_USER_H_
#define _JUDGE_USER_H_

#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <string.h>
#include <map>
#include <openssl/sha.h>
#include <vector>

// beachJudge
#include <Judge/Types.h>
#include <Judge/SQL.h>

namespace judge {

typedef struct User User;
typedef struct UserGroup UserGroup;


//------------------------------------------------------
//--------------------- UserGroup ----------------------

struct UserGroup
{
	// ID
	// Name
	// IsActive
	//
	// Users

	static std::map<u16, UserGroup *> s_groupsByID;
	static std::map<std::string, UserGroup *> s_groupsByName;

	UserGroup()
	{
	}

	UserGroup(const char *name, u16 id = 0xFFFF, bool isActive = true) :
		name(name),
		isActive(isActive),
		users(std::vector<User *>())
	{
		if (id == 0xFFFF) {
			id = 0;
			do ++id;
			while (s_groupsByID.count(id));
		}
		this->id = id;
		s_groupsByID[id] = this;
		s_groupsByName[name] = this;
	}

	~UserGroup()
	{
		//TODO: Verify this is necessary, might have to explicity remove
		s_groupsByID[id] = 0;
		s_groupsByName[name] = 0;
	}


	//-------------------- ID ----------------------

	u16 id;


	//------------------- Name ---------------------

	std::string name;


	//---------------- Activation ------------------
	// Inactive users cannot log into the system.

	bool isActive;


	//------------------ Users ---------------------

	std::vector<User *> users;


	//------------------- SQL ----------------------

	inline void SQL_Insert()
	{
		sqlite3_bind_int(SQL::userGroup_insert,
			1, id);
		sqlite3_bind_text(SQL::userGroup_insert,
			2, name.c_str(), name.length(), 0);
		sqlite3_bind_int(SQL::userGroup_insert,
			3, isActive);
		sqlite3_step(SQL::userGroup_insert);
		sqlite3_reset(SQL::userGroup_insert);
	}

	inline void SQL_Sync()
	{
		sqlite3_bind_text(SQL::userGroup_update,
			1, name.c_str(), name.length(), 0);
		sqlite3_bind_int(SQL::userGroup_update,
			2, isActive);
		sqlite3_step(SQL::userGroup_update);
		sqlite3_reset(SQL::userGroup_update);
	}

	inline void SQL_Delete()
	{
		sqlite3_bind_int(SQL::userGroup_delete,
			1, id);
		sqlite3_step(SQL::userGroup_delete);
		sqlite3_reset(SQL::userGroup_delete);
	}

	static inline void SQL_LoadAll()
	{
		const char *_name;
		u16 _id;
		bool _isActive;
		while (sqlite3_step(SQL::userGroup_selectAll)
				!= SQLITE_DONE) {
			_id = sqlite3_column_int(SQL::userGroup_selectAll, 0);
			_name = (const char *)sqlite3_column_text(SQL::userGroup_selectAll, 1);
			_isActive = sqlite3_column_int(SQL::userGroup_selectAll, 2);

			new UserGroup(_name, _id, _isActive);
		}
		sqlite3_reset(SQL::userGroup_selectAll);
	}
};


//------------------------------------------------------
//----------------------- User -------------------------

struct User
{
	// ID
	// Name
	// DisplayName
	// Level
	// Group
	//
	// Submissions
	// Scores
	// Messages

	static std::map<std::string, User *> s_usersByName;
	static std::map<u16, User *> s_usersByID;

	static inline void Cleanup()
	{
		std::map<std::string, User *>::iterator it = s_usersByName.begin();
		std::map<std::string, User *>::iterator end = s_usersByName.end();
		while (it != end) {
			delete it->second;
			++it;
		}
	}

	User()
	{
	}

	User(u32 id, const char *name, const char *pw, const char *display, u8 level, u16 groupID) :
		name(name),
		password(pw),
		display(display),
		level(level),
		groupID(groupID),
		id(id)
	{
		s_usersByName[name] = this;
		s_usersByID[id] = this;
		UserGroup::s_groupsByID[groupID]->users.push_back(this);
	}

	User(const char *name, const char *pw, const char *display, u8 level, u16 groupID, u32 id = 0xFFFFFFFF) :
		name(name),
		display(display),
		level(level),
		groupID(groupID)
	{
		SetPassword(pw);

		s_usersByName[name] = this;

		if (id == 0xFFFFFFFF) {
			do id = rand() % 0x100000000;
			while (s_usersByID.count(id));
		}
		this->id = id;
		s_usersByID[id] = this;
		UserGroup::s_groupsByID[groupID]->users.push_back(this);
	}

	~User()
	{
		// Keep an entry in the user map so we can reload the user later
		if (name.length())
			s_usersByName[name] = 0;
		s_usersByID[id] = 0;
		//TODO: Remove user from userGroup
	}

	u32 id;


	//------------------- Name ---------------------

	std::string name;

	std::string display;


	//----------------- Password -------------------
	//TODO: Add salt and secret key or implement HMAC

	std::string password;

	void SetPassword(const char *pw)
	{
		u16 len = SHA256_DIGEST_LENGTH * 2;
		char buffer[len + 1];
		unsigned char hash[SHA256_DIGEST_LENGTH];
		SHA256_CTX sha256;
		SHA256_Init(&sha256);
		SHA256_Update(&sha256, pw, strlen(pw));
		SHA256_Final(hash, &sha256);
		for (u16 a = 0; a < SHA256_DIGEST_LENGTH; ++a)
			sprintf(buffer + (a << 1), "%02x", hash[a]);
		buffer[len] = 0;
		password = buffer;
	}

	bool TestPassword(const char *pw)
	{
		u16 len = SHA256_DIGEST_LENGTH * 2;
		char buffer[len + 1];
		unsigned char hash[SHA256_DIGEST_LENGTH];
		SHA256_CTX sha256;
		SHA256_Init(&sha256);
		SHA256_Update(&sha256, pw, strlen(pw));
		SHA256_Final(hash, &sha256);
		for (u16 a = 0; a < SHA256_DIGEST_LENGTH; ++a)
			sprintf(buffer + (a << 1), "%02x", hash[a]);
		buffer[len] = 0;
		return password.compare(buffer) == 0;
	}


	//------------------- Level --------------------

	typedef enum UserLevels {
		Default,
		Judge,
		Admin
	} UserLevels;

	u8 level;


	//------------------- Group --------------------

	u16 groupID;


	//------------------- SQL ----------------------

	inline void SQL_Insert()
	{
		sqlite3_bind_int(SQL::user_insert,
			1, id);
		sqlite3_bind_text(SQL::user_insert,
			2, name.c_str(), name.length(), 0);
		sqlite3_bind_text(SQL::user_insert,
			3, password.c_str(), password.length(), 0);
		sqlite3_bind_text(SQL::user_insert,
			4, display.c_str(), display.length(), 0);
		sqlite3_bind_int(SQL::user_insert,
			5, level);
		sqlite3_bind_int(SQL::user_insert,
			6, groupID);
		sqlite3_step(SQL::user_insert);
		sqlite3_reset(SQL::user_insert);
	}

	inline void SQL_Sync()
	{
		sqlite3_bind_text(SQL::user_update,
			1, name.c_str(), name.length(), 0);
		sqlite3_bind_text(SQL::user_update,
			2, password.c_str(), password.length(), 0);
		sqlite3_bind_text(SQL::user_update,
			3, display.c_str(), display.length(), 0);
		sqlite3_bind_int(SQL::user_update,
			4, level);
		sqlite3_bind_int(SQL::user_update,
			5, groupID);
		sqlite3_step(SQL::user_update);
		sqlite3_reset(SQL::user_update);
	}

	inline void SQL_Delete()
	{
		sqlite3_bind_int(SQL::user_delete,
			1, id);
		sqlite3_step(SQL::user_delete);
		sqlite3_reset(SQL::user_delete);
	}

	static inline void SQL_LoadAll()
	{
		const char *_name, *_pw, *_display;
		u16 _id, _groupID;
		u8 _level;
		while (sqlite3_step(SQL::user_selectAll)
				!= SQLITE_DONE) {
			_id = sqlite3_column_int(SQL::user_selectAll, 0);
			_name = (const char *)sqlite3_column_text(SQL::user_selectAll, 1);
			_pw = (const char *)sqlite3_column_text(SQL::user_selectAll, 2);
			_display = (const char *)sqlite3_column_text(SQL::user_selectAll, 3);
			_level = sqlite3_column_int(SQL::user_selectAll, 4);
			_groupID = sqlite3_column_int(SQL::user_selectAll, 5);

			new User(_id, _name, _pw, _display, _level, _groupID);
		}
		sqlite3_reset(SQL::user_selectAll);
	}
};


}

#endif
