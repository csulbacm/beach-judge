#ifndef _JUDGE_USER_H_
#define _JUDGE_USER_H_

#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <string.h>
#include <map>
#include <openssl/sha.h>

namespace judge {

	typedef struct User
	{
		static std::map<std::string, User *> s_usersByName;
		static std::map<unsigned short, User *> s_usersById;

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

		User(const char *name, const char *pw, bool judge, unsigned short _id = 65535) :
			username(name),
			isJudge(judge),
			id(_id)
		{
			SetPassword(pw);

			s_usersByName[username] = this;

			if (id == 65535) {
				do id = rand() % 65536;
				while (s_usersById.count(id));
				s_usersById[id] = this;
			}
		}

		~User()
		{
			//- Keep an entry in the user map so we can reload the user later-
			if (username.length())
				s_usersByName[username] = 0;
			s_usersById[id] = 0;
		}

		bool isJudge;

		unsigned short id;


		//----------------------------------------------
		//----------------- Username -------------------

		std::string username;


		//----------------------------------------------
		//----------------- Password -------------------

		std::string password;

		void SetPassword(const char *pw)
		{
			unsigned int len = SHA256_DIGEST_LENGTH * 2;
			char buffer[len + 1];
			unsigned char hash[SHA256_DIGEST_LENGTH];
			SHA256_CTX sha256;
			SHA256_Init(&sha256);
			SHA256_Update(&sha256, pw, strlen(pw));
			SHA256_Final(hash, &sha256);
			for (int a = 0; a < SHA256_DIGEST_LENGTH; ++a)
				sprintf(buffer + (a << 1), "%02x", hash[a]);
			buffer[len] = 0;
			password = buffer;
		}

		bool TestPassword(const char *pw)
		{
			unsigned int len = SHA256_DIGEST_LENGTH * 2;
			char buffer[len + 1];
			unsigned char hash[SHA256_DIGEST_LENGTH];
			SHA256_CTX sha256;
			SHA256_Init(&sha256);
			SHA256_Update(&sha256, pw, strlen(pw));
			SHA256_Final(hash, &sha256);
			for (int a = 0; a < SHA256_DIGEST_LENGTH; ++a)
				sprintf(buffer + (a << 1), "%02x", hash[a]);
			buffer[len] = 0;

			return password.compare(buffer) == 0;
		}

		//TODO: Add salt and secret key or implement HMAC

	
	} User;


}

#endif
