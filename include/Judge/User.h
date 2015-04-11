#ifndef _JUDGE_USER_H_
#define _JUDGE_USER_H_

#include <stdio.h>
#include <string.h>
#include <openssl/sha.h>

namespace judge {

	typedef struct User
	{
		User() :
			password(0)
		{
		}

		User(const char *pw) :
			password(0)
		{
			SetPassword(pw);
		}

		~User()
		{
			delete [] password;
		}


		//----------------------------------------------
		//----------------- Password -------------------

		char *password;

		void SetPassword(const char *pw)
		{
			if (password != 0)
				delete [] password;
			password = new char[65];
			unsigned char hash[SHA256_DIGEST_LENGTH];
			SHA256_CTX sha256;
			SHA256_Init(&sha256);
			SHA256_Update(&sha256, pw, strlen(pw));
			SHA256_Final(hash, &sha256);
			for (int a = 0; a < SHA256_DIGEST_LENGTH; a++)
				sprintf(password + (a << 1), "%02x", hash[a]);
			password[64] = 0;
		}

		//TODO: Add salt and secret key

	
	} User;


}

#endif
