#include <iostream>
#include <vector>

#include "parser2.h"

namespace human_data {

using std::ostream;
using std::vector;

struct Event_Printer
{
    const Parser2::Event & e;
    Event_Printer(const Parser2::Event & e): e(e) {}
};

struct Event_List_Printer
{
    const vector<Parser2::Event> & events;
    Event_List_Printer(const vector<Parser2::Event> & e): events(e) {}
};

ostream & operator<< (ostream & o, const Event_Printer & e);
ostream & operator<< (ostream & o, const Event_List_Printer & e);

}
