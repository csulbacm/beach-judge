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

	UserGroup()
	{
	}

	UserGroup(const char *name, u16 id = 65535, bool isActive = true) :
		name(name),
		id(id),
		isActive(isActive)
	{
		if (id == 65535) {
			id = 0;
			do ++id;
			while (s_groupsByID.count(id));
		}
		s_groupsByID[id] = this;
	}

	~UserGroup()
	{
		s_groupsByID[id] = 0;
	}


	//-------------------- ID ----------------------

	u16 id;


	//------------------- Name ---------------------

	std::string name;


	//---------------- Activation ------------------
	// Inactive users cannot log into the system.

	bool isActive;


	//------------------ Users ---------------------

	std::vector<User> users;


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

	User(const char *name, const char *pw, u16 *groupID, bool judge, u16 _id = 65535) :
		name(name),
		groupID(groupID),
		isJudge(judge),
		id(_id)
	{
		SetPassword(pw);

		s_usersByName[name] = this;

		if (id == 65535) {
			do id = rand() % 65536;
			while (s_usersByID.count(id));
		}
		s_usersByID[id] = this;
	}

	~User()
	{
		// Keep an entry in the user map so we can reload the user later
		if (name.length())
			s_usersByName[name] = 0;
		s_usersByID[id] = 0;
	}

	bool isJudge;

	u16 id;


	//------------------- Name ---------------------

	std::string name;

	std::string displayName;


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


	//------------------- Group --------------------

	u16 *groupID;
};


}

#endif
