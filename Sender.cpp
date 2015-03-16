#include <iostream>
#include <ctime>
#include <stdlib.h> 
#include <math.h>
#include <map>
#include <queue>
#include <vector>
#include "Sender.h"

double tau = 0.005; //2tau is 10 ms and also 500 ms.
double delta = 2.5*tau; //timeout, 2.5 tau, 5 tau, ... 12.5 tau
int H = 54*8; //header length
int l = 1500*8; //packet length
int L = H+l; //packet
double BER = 0.0001;
int C = 5000; //channel capacity
int received_frames = 0;
int error_frames = 0;
int lost_frames = 0;
const int TOTAL_PACKETS = 10000;

Sender::Sender()
{
	SN = 0;
	next_expected_ack = 0;
        tc = 0;
        first = true;
}
Event Sender::Send()
{
    Event event1;
    double ranNum;
    int error_bits = 0;
    
    //forward channel
    tc+= tau;
    event1.time = tc;
    
    for(int i = 0; i < L; i++){
        ranNum = rand() / (double)(RAND_MAX);
        if(ranNum < BER){
            error_bits++;
        }
    }
    //std::cout<< "error bits in sent frame: "<< error_bits<< std::endl;
    if(error_bits >= 5){
        event1.flag = Event::lost;
        event1.eventType = Event::NIL;
        lost_frames++;
        return event1;
    }else if(error_bits > 0){
        event1.flag = Event::hasError;
    }else if(error_bits == 0){
        event1.flag = Event::errorFree;
    }
    
    //receiver
    Event returnEvent = receiver.receive(tc,SN,event1.flag);
    
    //why is this being done even if the sender packet got lost?
    //reverse channel
    tc = returnEvent.time;
    tc += (double)tau;
    returnEvent.time = tc;
    
    error_bits = 0;
    for(int i = 0; i < H; i++){
        ranNum = rand() / (double)(RAND_MAX);
        if(ranNum < BER){
            error_bits++;
        }
    }
    //std::cout<< "error bits in ACK: "<< error_bits<< std::endl;
    if(error_bits >= 5){
        returnEvent.flag = Event::lost;
        returnEvent.eventType = Event::NIL;
    }else if(error_bits > 0){
        returnEvent.flag = Event::hasError;
    }else if(error_bits == 0){
        returnEvent.flag = Event::errorFree;
    }
    
    return returnEvent;
    
}

void Sender::printES(std::priority_queue<Event,std::vector<Event>, OrderBySmallestTime> ES)
{
    while(!ES.empty()){
        std::cout<< ES.top()<<std::endl;
        ES.pop();
    }
}

void Sender::EventProcessor()
{
    std::priority_queue<Event,std::vector<Event>, OrderBySmallestTime> ES;
    Event event;
    next_expected_ack = (SN+1)%2;
    tc = 0.0;
    
    event.time = tc + ((double)L)/C + delta;
    event.eventType = Event::TIME_OUT;
    ES.push(event);
    
    Event returnedEvent = Send();
    if(returnedEvent.flag != Event::lost){
        ES.push(returnedEvent);
    }
    
    while(received_frames < TOTAL_PACKETS){
        //std::cout<<std::endl;
        //std::cout<< "received frames: "<< received_frames<< std::endl;
        //std::cout<< "current time: "<< tc<< std::endl;
        //std::cout<< "ES size is "<< ES.size()<< std::endl;
        //printES(ES);
        //std::cout<<"----Top item: "<<ES.top()<<std::endl;
        //std::cout<<"------ next expected ack: "<<next_expected_ack<<std::endl;
        //std::cout<< "ES top event type: "<< ES.top().eventType<< std::endl;
        if(ES.top().eventType == Event::ACK 
                && ES.top().flag == Event::errorFree
                && ES.top().RN == next_expected_ack)
        {
            ES.pop();
            ES.pop();
            SN++;
            next_expected_ack = (SN+1)%2;
            //tc += ;
            Event newEvent;
            newEvent.time = tc + ((double)L)/C + delta;
            newEvent.eventType = Event::TIME_OUT;
            ES.push(newEvent);
            returnedEvent = Send();
            if(returnedEvent.flag != Event::lost){
                ES.push(returnedEvent);
            }
        }else if(ES.top().eventType == Event::TIME_OUT){
            ES.pop();
            //tc += ;
            Event newEvent;
            newEvent.time = tc + ((double)L)/C + delta;
            newEvent.eventType = Event::TIME_OUT;
            ES.push(newEvent);
            returnedEvent = Send();
            if(returnedEvent.flag != Event::lost){
                ES.push(returnedEvent);
            }
        }else{
            ES.pop();
        }
        //break;
        //received_frames++;
    }
    
}

//transmission delay = L/C. Packet arrive at tc + L/C
//timeout is registered at tc + L/C + delta
int main(int argc, char* argv[])
{
    srand(time(0));
    double actual_tau[2] = {0.005, 0.25};
    double actual_ber[3] = {0.0, 0.00001, 0.0001};
    for(double i = 2.5; i < 12.51; i += 2.5)
    {
        //std::cout<<"delta: "<<std::endl;
        for(int j = 0; j < (sizeof(actual_tau)/sizeof(actual_tau[0])); j++){
            tau = actual_tau[j];
            for(int k = 0; k < (sizeof(actual_ber)/sizeof(actual_ber[0])); k++){
                BER = actual_ber[k];
                received_frames = 0;
                error_frames = 0;
                lost_frames = 0;
                delta = i*tau;

                Sender sender;
                sender.EventProcessor();
                std::cout<<((double)l)*TOTAL_PACKETS/sender.tc;
                if(j < (sizeof(actual_tau)/sizeof(actual_tau[0])-1)
                        || k < (sizeof(actual_ber)/sizeof(actual_ber[0])-1))
                {
                    std::cout<<", ";
                }

            }
        }
        std::cout<<std::endl;
    }
    //std::cout<< "throughput is: "<< ((double)l)*TOTAL_PACKETS/sender.tc<< std::endl;
    //std::cout<< "error frames/acks: "<< error_frames<<std::endl;
    //std::cout<< "lost frames: "<< lost_frames<<std::endl;

    return 0;
}
