#include <sstream>

// beachJudge
#include <Judge/WS_Judge.h>

using namespace std;

namespace judge {

//-----------------------------------------
//------------- Message Map ---------------

void msg_userCreate(libwebsocket *w, psd_judge *p, char *m);
void msg_userDelete(libwebsocket *w, psd_judge *p, char *m);
void msg_userInfo(libwebsocket *w, psd_judge *p, char *m);
void msg_userUpdate(libwebsocket *w, psd_judge *p, char *m);
void msg_userGroupCreate(libwebsocket *w, psd_judge *p, char *m);
void msg_userGroupDelete(libwebsocket *w, psd_judge *p, char *m);
void msg_userGroupInfo(libwebsocket *w, psd_judge *p, char *m);
void msg_userGroupUpdate(libwebsocket *w, psd_judge *p, char *m);
void msg_populate(libwebsocket *w, psd_judge *p, char *m);
void msg_userList(libwebsocket *w, psd_judge *p, char *m);
void msg_userGroupList(libwebsocket *w, psd_judge *p, char *m);

map<string, func_judge> createMsgMap()
{
	map<string, func_judge> m = map<string, func_judge>();
	m["POP"] = msg_populate;
	m["UL"] = msg_userList;
	m["UC"] = msg_userCreate;
	m["UD"] = msg_userDelete;
	m["UI"] = msg_userInfo;
	m["UU"] = msg_userUpdate;
	m["UGL"] = msg_userGroupList;
	m["UGC"] = msg_userGroupCreate;
	m["UGD"] = msg_userGroupDelete;
	m["UGI"] = msg_userGroupInfo;
	m["UGU"] = msg_userGroupUpdate;
	return m;
}
map<string, func_judge> g_msgMap =
	createMsgMap();


//----------------------------------------------
//------------- Message Handlers ---------------

void msg_populate(libwebsocket *w, psd_judge *p, char *m)
{
	// Populate User Session Data
	sprintf(p->msg, ""
		"\"msg\":\"POP\","
		"\"name\":\"%s\"",
		p->user->name.c_str());
}

void msg_userList(libwebsocket *w, psd_judge *p, char *m)
{
	char idStr[16];
	i16 r = sscanf(m, "i=%[0-9]", idStr);
	if (r != 1) {
		sprintf(p->msg, ""
			"\"msg\":\"UL\","
			"\"err\":\"I\"");
		return;
	}

	u16 id = atoi(idStr);
	if (UserGroup::s_groupsByID.count(id) == 0) {
		sprintf(p->msg, ""
			"\"msg\":\"UL\","
			"\"err\":\"U\"");
		return;
	}
	
	UserGroup *group = UserGroup::s_groupsByID[id];

	// Populate User Data
	stringstream str;
	vector<User *>::iterator it = group->users.begin();
	vector<User *>::iterator end = group->users.end();
	char entry[64];
	User *u;
	while (it != end) {
		u = *it;
		sprintf(entry, "{\"i\":\"%d\",\"n\":\"%s\"}",
			u->id, u->name.c_str());
		str << entry;
		++it;
		if (it != end)
			str << ",";
	}
	sprintf(p->msg, ""
		"\"msg\":\"UL\","
		"\"users\":[%s]",
		str.str().c_str());
}

void msg_userCreate(libwebsocket *w, psd_judge *p, char *m)
{
	// Restrict action to judge
	if (p->user->level < User::Admin) {
		sprintf(p->msg, "\"msg\": \"ERR\"");
		return;
	}

	//TODO: Define name/password length requirements
	//TODO: Allow for spaces and some style characters in display
	char groupIDStr[16], name[16], p1[16], p2[16], display[16];
	i16 r = sscanf(m, "g=%[0-9]&n=%[a-zA-Z0-9]&p1=%[a-zA-Z0-9]&p2=%[a-zA-Z0-9]&d=%[a-zA-Z0-9]", groupIDStr, name, p1, p2, display);
	if (r != 5) {
		sprintf(p->msg, ""
			"\"msg\":\"UC\","
			"\"err\":\"I\"");
		return;
	}

	u16 gid = atoi(groupIDStr);
	if (UserGroup::s_groupsByID.count(gid) == 0) {
		sprintf(p->msg, ""
			"\"msg\":\"UC\","
			"\"err\":\"U\"");
		return;
	}

	if (strcmp(p1, p2) != 0) {
		//TODO: memcpy prebuilt errors
		sprintf(p->msg, ""
			"\"msg\":\"UC\","
			"\"err\":\"P\"");
		return;
	}

	UserGroup *group = UserGroup::s_groupsByID[gid];

	//TODO: Check names for the group
	if (User::s_usersByName.count(name) != 0) {
		sprintf(p->msg, ""
			"\"msg\":\"UC\","
			"\"err\":\"N\"");
		return;
	}

	// Set name to lower case
	{
		i16 l = strlen(name);
		for (i16 a = 0; a < l; ++a)
			name[a] = tolower(name[a]);
	}

	User *user = new User(name, p1, display, User::Default, gid);
	user->SQL_Insert();
	sprintf(p->msg, ""
		"\"msg\":\"UC\","
		"\"i\":\"%d\","
		"\"n\":\"%s\"",
		user->id,
		user->name.c_str());
}

void msg_userDelete(libwebsocket *w, psd_judge *p, char *m)
{
	// Restrict action to judge
	if (p->user->level < User::Admin) {
		sprintf(p->msg, "\"msg\": \"ERR\"");
		return;
	}

	char idStr[16];
	i16 r = sscanf(m, "i=%[0-9]", idStr);
	if (r != 1) {
		sprintf(p->msg, ""
			"\"msg\":\"UD\","
			"\"err\":\"I\"");
		return;
	}

	u32 id = atoi(idStr);
	if (id == 0) {
		sprintf(p->msg, ""
			"\"msg\":\"UD\","
			"\"err\":\"A\"");
		return;
	}

	if (User::s_usersByID.count(id) == 0) {
		sprintf(p->msg, ""
			"\"msg\":\"UD\","
			"\"err\":\"U\"");
		return;
	}

	User *user = User::s_usersByID[id];
	user->SQL_Delete();
	sprintf(p->msg, ""
		"\"msg\":\"UD\","
		"\"i\":\"%d\"",
		id);
	user->Purge();
	delete user;
}

void msg_userInfo(libwebsocket *w, psd_judge *p, char *m)
{
	// Restrict action to judge
	if (p->user->level < User::Admin) {
		sprintf(p->msg, "\"msg\": \"ERR\"");
		return;
	}

	char idStr[16];
	i16 r = sscanf(m, "i=%[0-9]", idStr);
	if (r != 1) {
		sprintf(p->msg, ""
			"\"msg\":\"UI\","
			"\"err\":\"I\"");
		return;
	}

	u32 id = atoi(idStr);
	if (User::s_usersByID.count(id) == 0) {
		sprintf(p->msg, ""
			"\"msg\":\"UI\","
			"\"err\":\"U\"");
		return;
	}

	User *user = User::s_usersByID[id];
	sprintf(p->msg, ""
		"\"msg\":\"UI\","
		"\"g\":\"%d\","
		"\"i\":\"%d\","
		"\"n\":\"%s\","
		"\"d\":\"%s\"",
		user->groupID,
		id,
		user->name.c_str(),
		user->display.c_str());
}

void msg_userUpdate(libwebsocket *w, psd_judge *p, char *m)
{
	// Restrict action to judge
	if (p->user->level < User::Admin) {
		sprintf(p->msg, "\"msg\": \"ERR\"");
		return;
	}

	//TODO: Define name/password length requirements
	//TODO: Allow for spaces and some style characters in display
	char groupIDStr[16], idStr[16], name[16], p1[16], p2[16], display[16];
	i16 r = sscanf(m, "g=%[0-9]&i=%[0-9]&n=%[a-zA-Z0-9]&p1=%[a-zA-Z0-9]&p2=%[a-zA-Z0-9]&d=%[a-zA-Z0-9]", groupIDStr, idStr, name, p1, p2, display);
	bool noPW = false;
	if (r != 6) {
		r = sscanf(m, "g=%[0-9]&i=%[0-9]&n=%[a-zA-Z0-9]&p1=&p2=&d=%[a-zA-Z0-9]", groupIDStr, idStr, name, display);
		if (r != 4) {
			sprintf(p->msg, ""
				"\"msg\":\"UU\","
				"\"err\":\"I\"");
			return;
		}
		noPW = true;
	}

	u16 id = atoi(idStr);
	if (User::s_usersByID.count(id) == 0) {
		sprintf(p->msg, ""
			"\"msg\":\"UU\","
			"\"err\":\"U\"");
		return;
	}

	u16 gid = atoi(groupIDStr);
	if (UserGroup::s_groupsByID.count(gid) == 0) {
		sprintf(p->msg, ""
			"\"msg\":\"UU\","
			"\"err\":\"G\"");
		return;
	}

	if (noPW == false)
		if (strcmp(p1, p2) != 0) {
			//TODO: memcpy prebuilt errors
			sprintf(p->msg, ""
				"\"msg\":\"UU\","
				"\"err\":\"P\"");
			return;
		}

	User *user = User::s_usersByID[id];
	UserGroup *group = UserGroup::s_groupsByID[gid];

	//TODO: Check names for the group

	// Set name to lower case
	{
		i16 l = strlen(name);
		for (i16 a = 0; a < l; ++a)
			name[a] = tolower(name[a]);
	}

	i16 changes = 0;
	if (user->name.compare(name) != 0) {
		User::s_usersByName.erase(user->name);
		User::s_usersByName[name] = user;
		user->name = name;
		++changes;
	}

	if (noPW == false)
		if (user->password.compare(p1) != 0) {
			user->SetPassword(p1);
			++changes;
		}
	if (user->display.compare(display) != 0) {
		user->display = display;
		++changes;
	}
	if (changes == 0) {
		sprintf(p->msg, ""
			"\"msg\":\"UU\","
			"\"err\":\"S\"");
		return;
	}
	user->SQL_Sync();
	sprintf(p->msg, ""
		"\"msg\":\"UU\","
		"\"i\":\"%d\","
		"\"n\":\"%s\"",
		user->id,
		user->name.c_str());
}

void msg_userGroupList(libwebsocket *w, psd_judge *p, char *m)
{
	// Populate User Group Data
	stringstream str;
	map<u16, UserGroup *>::iterator it = UserGroup::s_groupsByID.begin();
	map<u16, UserGroup *>::iterator end = UserGroup::s_groupsByID.end();
	char entry[64];
	while (it != end) {
		sprintf(entry, "{\"i\":\"%d\",\"n\":\"%s\",\"a\":\"%d\"}",
			it->second->id, it->second->name.c_str(), it->second->isActive);
		str << entry;
		++it;
		if (it != end)
			str << ",";
	}
	sprintf(p->msg, ""
		"\"msg\":\"UGL\","
		"\"usergroups\":[%s]",
		str.str().c_str());
}

void msg_userGroupCreate(libwebsocket *w, psd_judge *p, char *m)
{
	// Restrict action to judge
	if (p->user->level < User::Admin) {
		sprintf(p->msg, "\"msg\": \"ERR\"");
		return;
	}

	//TODO: Define name length requirement
	bool isActive = false;
	char name[16], active[16];
	i16 r = sscanf(m, "n=%[a-zA-Z0-9]&a=%[a-zA-Z0-9]", name, active);
	if (r != 2) {
		r = sscanf(m, "n=%[a-zA-Z0-9]", name);
		if (r != 1) {
			sprintf(p->msg, ""
				"\"msg\":\"UGC\","
				"\"err\":\"I\"");
			return;
		}
	} else
		isActive = strcmp(active, "on") == 0;

	// Set name to lower case
	{
		i16 l = strlen(name);
		for (i16 a = 0; a < l; ++a)
			name[a] = tolower(name[a]);
	}

	if (UserGroup::s_groupsByName.count(name) != 0) {
		sprintf(p->msg, ""
			"\"msg\":\"UGC\","
			"\"err\":\"N\"");
		return;
	}

	//TODO: Debug text
	UserGroup *group = new UserGroup(name, 0xFFFF, isActive);
	group->SQL_Insert();
	sprintf(p->msg, ""
		"\"msg\":\"UGC\","
		"\"i\":\"%d\","
		"\"n\":\"%s\","
		"\"a\":\"%d\"",
		group->id,
		group->name.c_str(),
		group->isActive);
}

void msg_userGroupDelete(libwebsocket *w, psd_judge *p, char *m)
{
	// Restrict action to judge
	if (p->user->level < User::Admin) {
		sprintf(p->msg, "\"msg\": \"ERR\"");
		return;
	}

	char idStr[16];
	i16 r = sscanf(m, "i=%[0-9]", idStr);
	if (r != 1) {
		sprintf(p->msg, ""
			"\"msg\":\"UGD\","
			"\"err\":\"I\"");
		return;
	}

	u16 id = atoi(idStr);
	if (id == 0) {
		sprintf(p->msg, ""
			"\"msg\":\"UGD\","
			"\"err\":\"G\"");
		return;
	}

	if (UserGroup::s_groupsByID.count(id) == 0) {
		sprintf(p->msg, ""
			"\"msg\":\"UGD\","
			"\"err\":\"U\"");
		return;
	}

	//TODO: Debug text
	UserGroup *group = UserGroup::s_groupsByID[id];
	group->SQL_Delete();
	sprintf(p->msg, ""
		"\"msg\":\"UGD\","
		"\"i\":\"%d\"",
		id);
	group->Purge();
	delete group;
}

void msg_userGroupInfo(libwebsocket *w, psd_judge *p, char *m)
{
	// Restrict action to judge
	if (p->user->level < User::Admin) {
		sprintf(p->msg, "\"msg\": \"ERR\"");
		return;
	}

	char idStr[16];
	i16 r = sscanf(m, "i=%[0-9]", idStr);
	if (r != 1) {
		sprintf(p->msg, ""
			"\"msg\":\"UGI\","
			"\"err\":\"I\"");
		return;
	}

	u16 id = atoi(idStr);
	if (UserGroup::s_groupsByID.count(id) == 0) {
		sprintf(p->msg, ""
			"\"msg\":\"UGI\","
			"\"err\":\"U\"");
		return;
	}

	//TODO: Debug text
	UserGroup *group = UserGroup::s_groupsByID[id];
	sprintf(p->msg, ""
		"\"msg\":\"UGI\","
		"\"i\":\"%d\","
		"\"n\":\"%s\","
		"\"a\":\"%d\"",
		id,
		group->name.c_str(),
		group->isActive);
}

void msg_userGroupUpdate(libwebsocket *w, psd_judge *p, char *m)
{
	// Restrict action to judge
	if (p->user->level < User::Admin) {
		sprintf(p->msg, "\"msg\": \"ERR\"");
		return;
	}

	//TODO: Define name length requirement
	bool isActive = false;
	char name[16], active[16], idStr[16];
	i16 r = sscanf(m, "i=%[0-9]&n=%[a-zA-Z0-9]&a=%[a-zA-Z0-9]", idStr, name, active);
	if (r != 3) {
		r = sscanf(m, "i=%[0-9]&n=%[a-zA-Z0-9]", idStr, name);
		if (r != 2) {
			sprintf(p->msg, ""
				"\"msg\":\"UGU\","
				"\"err\":\"I\"");
			return;
		}
	} else
		isActive = strcmp(active, "on") == 0;

	u16 id = atoi(idStr);
	if (id == 0) {
		sprintf(p->msg, ""
			"\"msg\":\"UGU\","
			"\"err\":\"G\"");
		return;
	}

	if (UserGroup::s_groupsByID.count(id) == 0) {
		sprintf(p->msg, ""
			"\"msg\":\"UGU\","
			"\"err\":\"U\"");
		return;
	}

	UserGroup *group = UserGroup::s_groupsByID[id];

	// Set name to lower case
	{
		i16 l = strlen(name);
		for (i16 a = 0; a < l; ++a)
			name[a] = tolower(name[a]);
	}

	if (UserGroup::s_groupsByName.count(name) != 0)
		if (UserGroup::s_groupsByName[name] != group) { //TODO: Handle 0 case?
			sprintf(p->msg, ""
				"\"msg\":\"UGU\","
				"\"err\":\"N\"");
			return;
		}

	i16 changes = 0;
	if (group->name.compare(name) != 0) {
		UserGroup::s_groupsByName.erase(group->name);
		UserGroup::s_groupsByName[name] = group;
		group->name = name;
		++changes;
	}
	if (group->isActive != isActive) {
		group->isActive = isActive;
		++changes;
	}
	if (changes == 0) {
		sprintf(p->msg, ""
			"\"msg\":\"UGU\","
			"\"err\":\"S\"");
		return;
	}
	group->SQL_Sync();
	//TODO: Debug text
	sprintf(p->msg, ""
		"\"msg\":\"UGU\","
		"\"i\":\"%d\","
		"\"n\":\"%s\","
		"\"a\":\"%d\"",
		id,
		group->name.c_str(),
		group->isActive);
}

//----------------------------------------------
//------------ WebSocket Protocol --------------

int ws_judge(libwebsocket_context *context,
	libwebsocket *wsi,
	libwebsocket_callback_reasons reason,
	void *user, void *in, size_t len)
{
	i16 n;
	psd_judge *pss = (psd_judge *)user;

	switch (reason) {

	case LWS_CALLBACK_FILTER_PROTOCOL_CONNECTION: {
		i16 n = 0;
		char buf[256];
		const unsigned char *c;
		do {
			c = lws_token_to_string((lws_token_indexes)n);
			if (!c) {
				n++;
				continue;
			}
	
			if (!lws_hdr_total_length(wsi, (lws_token_indexes)n)) {
				n++;
				continue;
			}
	
			lws_hdr_copy(wsi, buf, sizeof buf, (lws_token_indexes)n);
			if (memcmp(c, "cookie:", 7) == 0) {
				char sessID[65];
				sessID[64] = 0;
				if (sscanf(buf, "JUDGESESSID=%[0-9a-zA-Z]", sessID) == 1) {
					string sessionID = string(sessID);
					if (Session::s_sessionMap.count(sessionID) != 0) {
						pss->session = &Session::s_sessionMap[sessionID];
						pss->session->Reset();
						pss->session->SQL_Sync();
						pss->user = pss->session->user;
					}
				}
			}

			n++;
		} while (c);
		
		// Block connections without valid session
		if (pss->user == 0)
			return -1;

		break;
	}

	case LWS_CALLBACK_ESTABLISHED:
		lwsl_info("callback_judge: LWS_CALLBACK_ESTABLISHED\n");
		pss->ringbuffer_tail = pss->ringbuffer_head;
		pss->wsi = wsi;

		// Initialize Session
		pss->width = 0;
		pss->height = 0;
		pss->lastSendMS = 0;
		pss->msg = pss->buffer + LWS_SEND_BUFFER_PRE_PADDING;

		printf("%p: Connected\n", pss);
		break;

	case LWS_CALLBACK_CLOSED:
		if (pss)
			for (n = 0; n < (sizeof pss->ringbuffer / sizeof pss->ringbuffer[0]); ++n)
				if (pss->ringbuffer[n].payload)
					free(pss->ringbuffer[n].payload);
		printf("%p: Disconnected\n", pss);
		break;

	case LWS_CALLBACK_PROTOCOL_DESTROY:
		break;

	case LWS_CALLBACK_SERVER_WRITEABLE:
		while (pss->ringbuffer_tail != pss->ringbuffer_head) {
			// Input Processing
			char *msgIn = (char *)pss->ringbuffer[pss->ringbuffer_tail].payload + LWS_SEND_BUFFER_PRE_PADDING;
			char msgType[5];
			msgType[4] = 0;
			sscanf(msgIn, "%4[a-zA-Z]", msgType);
			memset(pss->msg, 0, JUDGE_MAX_RESPONSE);
			if (g_msgMap.count(msgType))
				(*g_msgMap[msgType])(wsi, pss, msgIn + strlen(msgType) + 1);
			else
				sprintf(pss->msg, "\"msg\": \"ERR\"");
			libwebsocket_write(wsi,
				(unsigned char *)pss->msg,
				strlen(pss->msg), LWS_WRITE_TEXT);

			if (pss->ringbuffer_tail == (JUDGE_MAX_MESSAGE_QUEUE - 1))
				pss->ringbuffer_tail = 0;
			else
				pss->ringbuffer_tail++;

			if (((pss->ringbuffer_head - pss->ringbuffer_tail) &
					(JUDGE_MAX_MESSAGE_QUEUE - 1)) == (JUDGE_MAX_MESSAGE_QUEUE - 15))
				libwebsocket_rx_flow_allow_all_protocol(
								 libwebsockets_get_protocol(wsi));

//			lwsl_debug("tx fifo %d\n", (pss->ringbuffer_head - pss->ringbuffer_tail) & (JUDGE_MAX_MESSAGE_QUEUE - 1));

			if (lws_partial_buffered(wsi) || lws_send_pipe_choked(wsi)) {
				libwebsocket_callback_on_writable(context, wsi);
				break;
			}
			/*
			 * for tests with chrome on same machine as client and
			 * server, this is needed to stop chrome choking
			 */
#ifdef _WIN32
			Sleep(1);
#else
			usleep(1);
#endif
		}

		break;

	case LWS_CALLBACK_RECEIVE:
		if (((pss->ringbuffer_head - pss->ringbuffer_tail) &
					(JUDGE_MAX_MESSAGE_QUEUE - 1)) == (JUDGE_MAX_MESSAGE_QUEUE - 1)) {
			lwsl_err("dropping!\n");
			goto choke;
		}

		if (pss->ringbuffer[pss->ringbuffer_head].payload)
			free(pss->ringbuffer[pss->ringbuffer_head].payload);

		pss->ringbuffer[pss->ringbuffer_head].payload =
				malloc(LWS_SEND_BUFFER_PRE_PADDING + len +
							LWS_SEND_BUFFER_POST_PADDING);
		pss->ringbuffer[pss->ringbuffer_head].len = len;
		memcpy((char *)pss->ringbuffer[pss->ringbuffer_head].payload +
						LWS_SEND_BUFFER_PRE_PADDING, in, len + 1);
		if (pss->ringbuffer_head == (JUDGE_MAX_MESSAGE_QUEUE - 1))
			pss->ringbuffer_head = 0;
		else
			pss->ringbuffer_head++;

		if (((pss->ringbuffer_head - pss->ringbuffer_tail) &
					(JUDGE_MAX_MESSAGE_QUEUE - 1)) != (JUDGE_MAX_MESSAGE_QUEUE - 2))
			goto done;

choke:
		lwsl_debug("LWS_CALLBACK_RECEIVE: throttling %p\n", wsi);
		libwebsocket_rx_flow_control(wsi, 0);

//		lwsl_debug("rx fifo %d\n", (pss->ringbuffer_head - pss->ringbuffer_tail) & (JUDGE_MAX_MESSAGE_QUEUE - 1));
done:
		libwebsocket_callback_on_writable_all_protocol(
								 libwebsockets_get_protocol(wsi));
		break;

	default:
		break;
	}

	return 0;
}


}
