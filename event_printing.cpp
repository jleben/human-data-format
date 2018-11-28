#include "event_printing.h"

using namespace std;

namespace human_data {

ostream & operator<< (ostream & out, const Event_Printer & p)
{
    switch(p.e.type)
    {
    case Parser2::Event::List_Start:
    {
        out << "[" << endl;
        break;
    }
    case Parser2::Event::List_End:
    {
        out << "]" << endl;
        break;
    }
    case Parser2::Event::Map_Start:
    {
        out << "{" << endl;
        break;
    }
    case Parser2::Event::Map_End:
    {
        out << "}" << endl;
        break;
    }
    case Parser2::Event::Map_Key:
    {
        out << "? " << p.e.value << endl;
        break;
    }
    case Parser2::Event::Scalar:
    {
        out << "= " << p.e.value << endl;
        break;
    }
    case Parser2::Event::Verbatim_Scalar:
    {
        out << ">" << endl << p.e.value << endl << '<' << endl;
        break;
    }
    default:
        out << "?" << endl;
    }

    return out;
}

ostream & operator<< (ostream & out, const Event_List_Printer & p)
{
    for (auto & event : p.events)
        out << event;

    return out;
}

}
