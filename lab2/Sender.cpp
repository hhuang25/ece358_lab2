#include <iostream>
#include <ctime>
#include <stdlib.h> 
#include <math.h>
#include <map>
#include <queue>
#include <vector>
#include "Sender.h"

enum SimulationType {ABP, ABP_NAK, GBN};
SimulationType simulationType = ABP;
int simulationTypeNumber = 0;
double tau = 0.005; //2tau is 10 ms and also 500 ms.
double delta = 2.5*tau; //timeout, 2.5 tau, 5 tau, ... 12.5 tau
int H = 54*8; //header length
int l = 1500*8; //packet length
int L = H+l; //packet
int N = 4; //buffer size
std::deque<double> buffer;
std::deque<int>sequenceBuffer;
double BER = 0.00001;
int C = 5000000; //channel capacity
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
    //for GBN, time is the first(or last)? item in the buffer
    if(simulationType == GBN){
        tc = buffer.back() + tau;
    }else{
        tc += tau + (double)L/C;
    }
    
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
    
    //reverse channel
    tc = returnEvent.time;
    //tc += tau + (double)H/C;
    //returnEvent.time = tc;
    
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
        tc+=tau;
        returnEvent.flag = Event::hasError;
    }else if(error_bits == 0){
        tc+=tau;
        returnEvent.flag = Event::errorFree;
    }
    returnEvent.time = tc;
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
    next_expected_ack = (SN+1)%(N+1);
    tc = 0.0;
    
    event.time = tc + ((double)L)/C + delta;
    event.eventType = Event::TIME_OUT;
    ES.push(event);
    
    //std::cout<< "First timeout: "<< event.time<< std::endl;
    Event returnedEvent = Send();
    //std::cout<< "First ack time: "<< returnedEvent.time<< std::endl;
    if(returnedEvent.flag != Event::lost){
        ES.push(returnedEvent);
    }
    
    while(received_frames < (TOTAL_PACKETS-1)){
//        std::cout<<std::endl;
//        std::cout<< "current time: "<< tc<< std::endl;
//        std::cout<< "received frames: "<< received_frames<< std::endl;
        //std::cout<< "ES size is "<< ES.size()<< std::endl;
//        printES(ES);
        //std::cout<<"----Top item: "<<ES.top()<<std::endl;
        //std::cout<<"------ next expected ack: "<<next_expected_ack<<std::endl;
        //std::cout<< "ES top event type: "<< ES.top().eventType<< std::endl;
        if(ES.top().eventType == Event::ACK 
                && ES.top().flag == Event::errorFree
                && ES.top().RN == next_expected_ack)
        {
            received_frames++;
            tc = ES.top().time;
            //std::cout<< "top tc: "<< ES.top().time<< std::endl;
            ES.pop();
            ES.pop();
            SN = (SN+1)%(N+1);
            next_expected_ack = (SN+1)%(N+1);
            //tc += ;
            Event newEvent;
            newEvent.time = tc + ((double)L)/(double)C + delta;
            //std::cout<< "inserted timeout event time: "<< newEvent.time<< std::endl;
            newEvent.eventType = Event::TIME_OUT;
            ES.push(newEvent);
            returnedEvent = Send();
            //std::cout<< "returned event time: "<< returnedEvent.time<< std::endl;
            if(returnedEvent.flag != Event::lost){
                ES.push(returnedEvent);
            }
        }else if(ES.top().eventType == Event::TIME_OUT){
            tc = ES.top().time;
            //std::cout<< "timeout tc: "<< tc<< std::endl;
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
        }else if(ES.top().eventType == Event::ACK 
                    && ES.top().RN != next_expected_ack
                    && simulationType == ABP_NAK){
            tc = ES.top().time;
            //std::cout<< "timeout tc: "<< tc<< std::endl;
            ES.pop();
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
        }
        else{
            tc = ES.top().time;
            //std::cout<< "has error tc: "<< tc<< std::endl;
            ES.pop();
        }
        //break;
        //received_frames++;
    }
    
}

void Sender::EventGBNProcessor()
{
    std::priority_queue<Event,std::vector<Event>, OrderBySmallestTime> ES;
    Event timeout;
    //double bufferArray[N];
    
    int P = (SN)%(N+1); // current ack
    tc = 0.0;
    timeout.eventType = Event::TIME_OUT;
    timeout.time = tc + ((double)L)/C + delta;
    
    //create buffer, fill it with items.
    tc+=((double)L)/C;
    buffer.push_back(tc);
    sequenceBuffer.push_back(SN);
    Event returnedEvent = Send();
    //returned result stored in ES
    if(returnedEvent.flag != Event::lost){
        ES.push(returnedEvent);
    }
    //repeat for all N items
    while(buffer.size()<N){
        tc+=((double)L)/C;
        buffer.push_back(buffer.back() + ((double) L)/C);
        SN = (SN+1)%(N+1);
        sequenceBuffer.push_back(SN);
        Event returnedEvent = Send();
        if(returnedEvent.flag != Event::lost){
            ES.push(returnedEvent);
        }
    }
    
    while(received_frames < (TOTAL_PACKETS)){
//        std::cout<<std::endl;
//        std::cout<< "current time: "<< tc<< std::endl;
//        std::cout<< "Buffer has "<<buffer.size()<<" items: ";
//        for(int i = 0; i < buffer.size(); i++){
//            std::cout<<buffer.at(i)<< " ";
//        }
//        std::cout<<std::endl;
//        std::cout<< "timeout at "<< timeout.time<< std::endl;
//        std::cout<< "received frames: "<< received_frames<< std::endl;
//        std::cout<< "ES size is "<< ES.size()<< std::endl;
//        if(!ES.empty()) printES(ES);
//        std::cout<<"----Top item: "<<ES.top()<<std::endl;
        
//        std::cout<<"------ current P: "<<P<<std::endl;
//        std::cout<< "ES top event type: "<< ES.top().eventType<< std::endl;
        
        //if top item is ack, not timeout
        if((ES.size()>0) && ES.top().time < timeout.time
                && ES.top().eventType == Event::ACK
                && ES.top().flag == Event::errorFree
                && ES.top().RN != P)
        {
            int topRN = ES.top().RN;
	    tc = ES.top().time;
	    ES.pop();
            if(tc < buffer.back()){
                //tc = buffer.back();
            }
//            std::cout<<"~~Success, top RN: "<<topRN<<std::endl;
            //this implies you received all previous frames
            //therefore keep popping
            while(P != topRN){
//                std::cout<<"----removing ack: "<<ES.top()<<std::endl;
//                std::cout<<"------ current P: "<<P<<std::endl;
                received_frames++;
                //SN=(SN+1)%(N+1);
                P = (P+1)%(N+1);
                //if the ack comes back before the buffer's last item's been sent
                //I can't send the next one until after the buffer's last item
                if(received_frames >= (TOTAL_PACKETS)){
//                    std::cout<< "exiting at: "<< tc<< std::endl;
                    break;
                }
                if(tc > buffer.back()){
                    buffer.push_back(tc + ((double) L)/C);
                }else{
                    buffer.push_back(buffer.back() + ((double) L)/C);
                }
                SN = (SN+1)%(N+1);
                sequenceBuffer.push_back(SN);
                buffer.pop_front();
                sequenceBuffer.pop_front();
                timeout.time = buffer.front() + ((double)L)/C + delta;
                Event returnedEvent = Send();
                if(returnedEvent.flag != Event::lost){
                    ES.push(returnedEvent);
                }
            }
        //top item is a timeout
        }else if((ES.size()>0) 
                && ES.top().time > timeout.time
                && ES.top().eventType == Event::ACK){
            //empty ES
            //resend every item in buffer
            tc = timeout.time;
//            std::cout<<"~~timeout "<<timeout.time << " < ES " <<ES.top()<<std::endl;
//            std::cout<<"------ current P: "<<P<<std::endl;
            while(!ES.empty()){
                ES.pop();
            }
            buffer.clear();
            SN = sequenceBuffer.front();
            sequenceBuffer.clear();
            buffer.push_back(tc + ((double) L)/C);
            sequenceBuffer.push_back(SN);
            Event returnedEvent = Send();
            if(returnedEvent.flag != Event::lost){
                ES.push(returnedEvent);
            }
            while(buffer.size()<N){
                buffer.push_back(buffer.back() + ((double) L)/C);
                SN = (SN+1)%(N+1);
                sequenceBuffer.push_back(SN);
                Event returnedEvent = Send();
                if(returnedEvent.flag != Event::lost){
                    ES.push(returnedEvent);
                }
            }
            timeout.time = buffer.front() + ((double)L)/C + delta;
        }else{
            //ignore errors, but you should repopulate buffer if needed
            //received_frames++;
//            std::cout<<"~~Neither good ack nor timeout."<<std::endl;
            if(!ES.empty()){
//                std::cout<<"top item: " <<ES.top()<<std::endl;
                //tc = ES.top().time;
                ES.pop();
            }else{
                //repopulate
                buffer.clear();
                SN = sequenceBuffer.front();
                sequenceBuffer.clear();
                tc = timeout.time;
                buffer.push_back(tc + ((double) L)/C);
                sequenceBuffer.push_back(SN);
                Event returnedEvent = Send();
                if(returnedEvent.flag != Event::lost){
                    ES.push(returnedEvent);
                }
                while(buffer.size()<N){
                    tc+=((double)L)/C;
                    buffer.push_back(buffer.back() + ((double) L)/C);
                    SN = (SN+1)%(N+1);
                    sequenceBuffer.push_back(SN);
                    Event returnedEvent = Send();
                    if(returnedEvent.flag != Event::lost){
                        ES.push(returnedEvent);
                    }
                }
                timeout.time = buffer.front() + ((double)L)/C + delta;
            }
        }
    }
}

//transmission delay = L/C. Packet arrive at tc + L/C
//timeout is registered at tc + L/C + delta
int main(int argc, char* argv[])
{
    srand(time(0));
    if(argc == 2){
        std::string argv1 = argv[1];
        if(argv1=="ABP"){
            //std::cout<<"begin ABP"<<std::endl;
            simulationType = ABP;
            simulationTypeNumber = 0;
            N = 1;
        }else if(argv1=="ABP_NAK"){
            //std::cout<<"begin ABP_NAK"<<std::endl;
            simulationType = ABP_NAK;
            simulationTypeNumber = 1;
            N = 1;
        }else if(argv1=="GBN"){
            //std::cout<<"begin GBN"<<std::endl;
            simulationType = GBN;
            simulationTypeNumber = 2;
            N = 4;
        }else{
            if(argv1=="-h"){
                std::cout<<"should be GBN tau delta BER N"<<std::endl;
            }else{
                std::cout<<"incorrect parameters: "<<argv[1]<<std::endl;
            }
            return 1;
        }
    }else if(argc == 6){
        std::string argv1 = argv[1];
        tau = atof(argv[2]);
        delta = atof(argv[3]);
        BER = atof(argv[4]);
        N = atoi(argv[5]);
        if(argv1=="ABP"){
            //std::cout<<"begin ABP"<<std::endl;
            simulationType = ABP;
            simulationTypeNumber = 0;
            N = 1;
        }else if(argv1=="ABP_NAK"){
            //std::cout<<"begin ABP_NAK"<<std::endl;
            simulationType = ABP_NAK;
            simulationTypeNumber = 1;
            N = 1;
        }else if(argv1=="GBN"){
            //std::cout<<"begin GBN"<<std::endl;
            simulationType = GBN;
            simulationTypeNumber = 2;
            //N = 4;
        }else{
            std::cout<<"incorrect parameters: "<<argv[1]<<std::endl;
            std::cout<<"should be GBN tau delta BER N"<<std::endl;
            return 1;
        }
        Sender sender;
        if(simulationType != GBN){
            sender.EventProcessor();
        }else{
            //remember to clear buffer for GBN
            buffer.clear();
            sequenceBuffer.clear();
            sender.EventGBNProcessor();
        }
        std::cout<<((double)l)*TOTAL_PACKETS/sender.tc<<std::endl;
        return 0;
    }
    
//    Sender sender;
//    simulationType = GBN;
//    simulationTypeNumber = 2;
//    if(simulationType != GBN){
//                sender.EventProcessor();
//    }else{
//            buffer.clear(); //remember to clear buffer for GBN
//            sequenceBuffer.clear();
//            sender.EventGBNProcessor();
//    }
//    std::cout<< "sender tc is: "<< (double)sender.tc<< std::endl;
//    std::cout<< "throughput is: "<< ((double)l)*TOTAL_PACKETS/sender.tc<< std::endl;
//    std::cout<< "error frames/acks: "<< error_frames<<std::endl;
//    std::cout<< "lost frames: "<< lost_frames<<std::endl;
//    return 0;
    
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

                Sender *sender = new Sender();
                sender->tc = 0.0;
                if(simulationType != GBN){
                    sender->EventProcessor();
                }else{
                    //remember to clear buffer for GBN
                    buffer.clear();
                    sequenceBuffer.clear();
                    sender->EventGBNProcessor();
                }
                std::cout<<((double)l)*TOTAL_PACKETS/sender->tc;
                if(j < (sizeof(actual_tau)/sizeof(actual_tau[0])-1)
                        || k < (sizeof(actual_ber)/sizeof(actual_ber[0])-1))
                {
                    std::cout<<", ";
                }
                delete sender;
            }
        }
        std::cout<<std::endl;
    }
    

    return 0;
}
