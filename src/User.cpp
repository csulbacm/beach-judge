// beachJudge
#include <Judge/Config.h>
#include <Judge/Base.h>
#include <Judge/User.h>

using namespace std;

namespace judge {

map<u16, UserGroup *> UserGroup::s_byID = map<u16, UserGroup *>();
map<string, UserGroup *> UserGroup::s_byName = map<string, UserGroup *>();

map<string, User *> User::s_byName = map<string, User *>();
map<u16, User *> User::s_byID = map<u16, User *>();


}
