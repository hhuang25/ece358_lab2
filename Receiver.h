#ifndef RECEIVER_H
#define	RECEIVER_H
/* 
 * File:   Receiver.h
 * Author: huangster
 *
 * Created on March 13, 2015, 11:51 PM
 */

#include "Event.h"

class Receiver
{
    public:
        int next_expected_frame;
        //double tcs; // current time
        //int RN;
        int received_frames;
        
        
        Receiver();
        Event receive(double time, int SN, Event::error_flag flag);
};

#endif	/* RECEIVER_H */

