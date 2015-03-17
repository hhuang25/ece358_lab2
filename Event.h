#ifndef EVENT_H
#define EVENT_H
#include <iostream>

class Event
{
	public:
		enum type{TIME_OUT = 0, ACK = 1, NIL = 2};
		enum error_flag{lost = 0, errorFree = 1, hasError = 2};
                
                type eventType;
                error_flag flag;
		double time;
		int RN; //sequence number
                
                //Event::type getType(){return Event::eventType;}
                double setTime(double t){time = t; return time;}
                friend std::ostream&
                operator<<(std::ostream& os, const Event& ev) {
                    return os << "time: " << ev.time 
                    << " type: " << ev.eventType
                    << " flag: " << ev.flag
                    << " RN: "<< ev.RN;
                }
                //Event(){flag = lost;}
	private:
		
};
struct LessThanByTime
{
  bool operator()(const Event& lhs, const Event& rhs) const
  {
    return lhs.time < rhs.time;
  }
};

struct OrderBySmallestTime
{
  bool operator()(const Event& lhs, const Event& rhs) const
  {
    return lhs.time > rhs.time;
  }
};

#endif
