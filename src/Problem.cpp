// beachJudge
#include <Judge/Config.h>
#include <Judge/Base.h>
#include <Judge/Problem.h>

using namespace std;

namespace judge {

map<u16, ProblemSet *> ProblemSet::s_byID = map<u16, ProblemSet *>();
map<string, ProblemSet *> ProblemSet::s_byName = map<string, ProblemSet *>();


}
