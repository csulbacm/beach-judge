// beachJudge
#include <Judge/Config.h>
#include <Judge/Base.h>
#include <Judge/User.h>

using namespace std;

namespace judge {

map<string, User *> User::s_usersByName = map<string, User *>();
map<u16, User *> User::s_usersById = map<u16, User *>();


}
