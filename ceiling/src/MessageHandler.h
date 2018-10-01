#include "Utils.h"

#include <string>
using namespace std;

#include <sstream>

namespace patch
{
    template < typename T > std::string to_string( const T& n )
    {
        std::ostringstream stm ;
        stm << n ;
        return stm.str() ;
    }
}
using namespace patch;

class MessageHander {
    private:
    const string projectName = "ceiling";
    Utils * utils;
    public:
    bool matchesTopic(string topic);
    MessageHander(Utils &utils);
    ~MessageHander();
    void messageReceived(string topic, string payload);
};