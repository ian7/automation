#include "Utils.h"

#include <string>
#include <sstream>
#include <vector>
#include <iterator>
using namespace std;


namespace patch
{
    template < typename T > string to_string( const T& n )
    {
        ostringstream stm ;
        stm << n ;
        return stm.str() ;
    }
}
using namespace patch;

class MessageHander {
    private:
    const string projectName = "ceiling";
    template<typename Out> void split(const string &s, char delim, Out result);
    Utils * utils;

    public:
    MessageHander(Utils &utils);
    ~MessageHander();
    bool matchesTopic(string topic, string testee);
    void messageReceived(string topic, string payload);
    string getProjectName();
    vector<string> split(const string &s, char delim);
};