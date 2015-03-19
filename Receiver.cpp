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
    //extern int simulationTypeNumber;
    
    if(flag == Event::errorFree){
        next_expected_frame = (next_expected_frame+1)%(N+1);
        //RN = next_expected_frame;
        //std::cout<<"next expected frame: "<< next_expected_frame<<std::endl;
        time += ((double)H)/C;
        event.time = time;
        event.RN = next_expected_frame;
        event.eventType = Event::ACK;
        received_frames++;
		//exit(0) after 10000 frames
        event.eventType = Event::ACK;
    }else if(flag == Event::hasError){
        //only for ABP, do nothing for GBN?
        //if(simulationTypeNumber != 2){
        time += ((double)H)/C;
        event.time = time;
        event.RN = next_expected_frame;
        event.eventType = Event::ACK;
        //}
        error_frames++;
    }else{
        //if flag says lost, do nothing
        lost_frames++;
    }
    
    return event;
}
