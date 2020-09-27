#ifndef TRAFFICLIGHT_H
#define TRAFFICLIGHT_H

#include <mutex>
#include <deque>
#include <condition_variable>
#include "TrafficObject.h"

// forward declarations to avoid include cycle
class Vehicle;
enum TrafficLightPhase
{
    red,
    green,
};

template <class T>
class MessageQueue
{
public:
    void send(T &&currentPhase);
    T receive();
private:
    std::deque<T> _queue;
    std::mutex _mtx;
    std::condition_variable _condition;
};


class TrafficLight : TrafficObject
{
public:
    // constructor / desctructor
    TrafficLight();
    ~TrafficLight(){}

    // getters / setters
    TrafficLightPhase getCurrentPhase();

    // typical behaviour methods
    void waitForGreen();
    void simulate();
private:
    // typical behaviour methods
    void cycleThroughPhases();
    
    std::shared_ptr<MessageQueue<TrafficLightPhase>> _messageQueue;
    std::vector<std::thread> _threads;
    std::condition_variable _condition;
    std::mutex _mutex;
    TrafficLightPhase _currentPhase;
};

#endif