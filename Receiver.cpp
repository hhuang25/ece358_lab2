#include "Receiver.h"

Receiver::Receiver()
{
    next_expected_frame = 0;
    //tcs = 0;
    //received_frames = 0;
}

Event Receiver::receive(double time, int SN, Event::error_flag flag)
{
    Event event;
    //tcs = time;
    
    extern int H;
    extern int C;
    extern int received_frames;
    extern int error_frames;
    extern int lost_frames;
    extern int N;
    
    if(flag == Event::errorFree){
        next_expected_frame = (next_expected_frame+1)%(N+1);
        //RN = next_expected_frame;
        //std::cout<<"next expected frame: "<< next_expected_frame<<std::endl;
        time += ((double)H)/C;
        event.time = time;
        event.RN = next_expected_frame;
        event.eventType = Event::ACK;
        received_frames++;
        if(received_frames >= 10000)
        {
            //exit(0) or return to main
        }
        event.eventType = Event::ACK;
    }else if(flag == Event::hasError){
        //only for ABP
        time += ((double)H)/C;
        event.time = time;
        event.RN = next_expected_frame;
        event.eventType = Event::ACK;
        error_frames++;
    }else{
        //if flag says lost, do nothing
        lost_frames++;
    }
    
    return event;
}
