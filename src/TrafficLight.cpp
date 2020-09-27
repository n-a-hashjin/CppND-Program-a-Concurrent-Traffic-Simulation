#include <iostream>
#include <random>
#include "TrafficLight.h"
#include <ctime>
#include <chrono>
#include <future>
#include <thread>

/* Implementation of class "MessageQueue" */


template <typename T>
T MessageQueue<T>::receive()
{
    // wait for and receive new messages and pull them from the queue
    // The received object returns by the receive function.
    std::unique_lock<std::mutex> ulck(_mtx);
    _condition.wait(ulck, [this]{return !_queue.empty();});
    T msg = std::move(_queue.back());
    _queue.pop_back();
    return msg;
}

template <typename T>
void MessageQueue<T>::send(T &&msg)
{
    std::lock_guard<std::mutex> glck(_mtx);
    _queue.emplace_back(std::move(msg));
    _condition.notify_one();
}


/* Implementation of class "TrafficLight" */

TrafficLight::TrafficLight()
{
    _currentPhase = TrafficLightPhase::red;
    _messageQueue = std::make_shared<MessageQueue<TrafficLightPhase>>();
}

void TrafficLight::waitForGreen()
{
    // repeatedly calls the receive function on the message queue. 
    // Once it receives TrafficLightPhase::green, the method returns.
    while (true)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
        if (_messageQueue->receive() == TrafficLightPhase::green) return;
    }
}

TrafficLightPhase TrafficLight::getCurrentPhase()
{
    return _currentPhase;
}

void TrafficLight::simulate()
{
    // started in a thread when the public method „simulate“ is called.
    _threads.push_back(std::thread(&TrafficLight::cycleThroughPhases, this));
}

// virtual function which is executed in a thread
void TrafficLight::cycleThroughPhases()
{
    // measuring the time between two loop cycles and toggles the current phase
    // of the traffic light between red and green and sends an update method 
    long beginTime = std::clock();
    long cycleDuration = (std::rand() % 21 + 40) / 10 * CLOCKS_PER_SEC;
    while (true)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
        if ((std::clock() - beginTime) >= cycleDuration)
        {
            if (_currentPhase == red) _currentPhase = green;
            else _currentPhase = red;

            // Send an update to the message queue and wait for it to be sent
            auto msg = _currentPhase;
            auto isSent = std::async(std::launch::async, &MessageQueue<TrafficLightPhase>::send, _messageQueue, std::move(msg));
            isSent.wait();
            
            beginTime = std::clock();
            cycleDuration = (std::rand() % 21 + 40) / 10 * CLOCKS_PER_SEC;
        }
    }
}