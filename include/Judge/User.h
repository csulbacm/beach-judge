#ifndef _JUDGE_USER_H_
#define _JUDGE_USER_H_

#include <stdio.h>
#include <string>
#include <string.h>
#include <openssl/sha.h>

namespace judge {

	typedef struct User
	{
		User()
		{
		}

		User(const char *name, const char *pw) :
			username(name)
		{
			SetPassword(pw);
		}

		~User()
		{
		}


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

		//TODO: Add salt and secret key or implement HMAC

	
	} User;


}

#endif
