#ifndef NODE_H
#define NODE_H
#include "Event.h"
#include "Receiver.h"

class Sender
{
	public:
		int SN;
		int next_expected_ack;
                double tc; //current time
                bool first;
                Receiver receiver;
                //double delta; // timeout
                //double setDelta(double t){delta = t; return delta;}
		Sender();
                Event Send();
                void printES(std::priority_queue<Event,std::vector<Event>, OrderBySmallestTime> ES);
                void EventProcessor();
                void EventGBNProcessor();
};
#endif
