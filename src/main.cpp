#include <setjmp.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <fstream>
#include <sstream>
#include <map>

// sqlite
#include <sqlite3.h>

// lws
#include <libwebsockets.h>

// beachJudge
#include <Judge/Judge.h>
#include <Judge/WS_HTTP.h>
#include <Judge/WS_Judge.h>

using namespace judge;
using namespace std;

const char *judge::g_resourcePath = "../res";

map<string, Session> Session::s_sessionMap = map<string, Session>();

enum lws_protocols {
	PROTOCOL_HTTP = 0,
	PROTOCOL_JUDGE,
	LWS_PROTOCOL_COUNT
};

// Catch program termination
volatile i16 exiting = 0;
sigjmp_buf jmpenv;
void sigHandler(int signo, siginfo_t *info, void *context) {
	if (exiting++)
		_exit(1);
	siglongjmp(jmpenv, 1);
}


//-----------------------------------------
//------------ Entry Function -------------

volatile int force_exit = 0;
int main(int argc, char *argv[])
{
	srand(time(NULL));

	// Initialize LWS
	libwebsocket_context *context;
	libwebsocket_protocols protocols[] = {
		{ // HTTP Handler
			"http-only", //lname
			ws_http, // callback
			sizeof (psd_http), // per_session_data_size
			0, // max frame size / rx buffer
		},
		{ // beachJudge Protocol
			"judge-protocol",
			ws_judge,
			sizeof(psd_judge),
			128,
		},
		{ NULL, NULL, 0, 0 } // terminator
	};

	lws_set_log_level(0, NULL);
	lws_context_creation_info info;
	memset(&info, 0, sizeof info);
	{
		info.port = 8081;
		info.protocols = protocols;
		info.gid = -1;
		info.uid = -1;
#if JUDGE_SSL
		char cert_path[1024];
		char key_path[1024];
		u16 plen_check = strlen(g_resourcePath) + 32;
		if (plen_check > sizeof(cert_path) || plen_check > sizeof(key_path)) {
			lwsl_err("resource path too long\n");
			return -1;
		}
		sprintf(cert_path, JUDGE_SSL_CERT);
		sprintf(key_path, JUDGE_SSL_KEY);
		info.ssl_cert_filepath = cert_path;
		info.ssl_private_key_filepath = key_path;
		info.options = LWS_SERVER_OPTION_ALLOW_NON_SSL_ON_SSL_PORT;
#endif
		context = libwebsocket_create_context(&info);
	}
	if (context == NULL) {
		lwsl_err("libwebsocket init failed\n");
		return -1;
	}

	// Load beachJudge Data
	printf("Loading beachJudge data...\n");
	loadJudgeData();

	// SQLite Test
	{
		printf("Opening SQLIte Database...\n");
		sqlite3 *db;
		int c = sqlite3_open(".data", &db);
		printf("   C: %d\n", c);
		sqlite3_close(db);
	}

	// Start Server
	printf("Server is running.\n");
	if (!sigsetjmp(jmpenv, 1)) {
		// Clean up upon orderly shut down. Do _not_ cleanup if we die
		// unexpectedly, as we cannot guarantee if we are still in a valid
		// static. This means, we should never catch SIGABRT.
		static const i16 signals[] = { SIGHUP, SIGINT, SIGQUIT, SIGTERM };
		struct sigaction sa;
		memset(&sa, 0, sizeof(sa));
		sa.sa_sigaction = sigHandler;
		sa.sa_flags	 = SA_SIGINFO | SA_RESETHAND;
		for (u16 i = 0; i < sizeof(signals) / sizeof(*signals); ++i)
			sigaction(signals[i], &sa, NULL);

		// Main Loop
		i16 n = 0;
		while (n >= 0 && !force_exit)
			n = libwebsocket_service(context, 50);
	}

	// Cleanup
	//TODO: Debug seg fault on from this line
	printf("Server is shutting down...\n");
	libwebsocket_context_destroy(context);
	printf("Saving beachJudge data...\n");
	saveJudgeData();
	User::Cleanup();

	return 0;
}
