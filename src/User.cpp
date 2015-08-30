// beachJudge
#include <Judge/Config.h>
#include <Judge/User.h>

using namespace std;

namespace judge {

map<string, User *> User::s_usersByName = map<string, User *>();
map<unsigned short, User *> User::s_usersById = map<unsigned short, User *>();

}
