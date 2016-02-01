// beachJudge
#include <Judge/Config.h>
#include <Judge/Base.h>
#include <Judge/User.h>

using namespace std;

namespace judge {

map<u16, UserGroup *> UserGroup::s_groupsByID = map<u16, UserGroup *>();
map<string, UserGroup *> UserGroup::s_groupsByName = map<string, UserGroup *>();

map<string, User *> User::s_usersByName = map<string, User *>();
map<u16, User *> User::s_usersByID = map<u16, User *>();


}
