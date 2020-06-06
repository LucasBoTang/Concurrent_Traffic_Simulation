#include <iostream>
#include <random>
#include <future>
#include "TrafficLight.h"

/* Implementation of class "MessageQueue" */

template <typename T>
T MessageQueue<T>::receive()
{
    // FP.5a : The method receive should use std::unique_lock<std::mutex> and _condition.wait() 
    // to wait for and receive new messages and pull them from the queue using move semantics. 
    // The received object should then be returned by the receive function. 
}

template <typename T>
void MessageQueue<T>::send(T &&msg)
{
    std::lock_guard<std::mutex> lock(_mutex);
    _queue.push_back(std::move(msg));
    _condition.notify_one();
}

/* Implementation of class "TrafficLight" */
TrafficLight::TrafficLight()
{
    _currentPhase = TrafficLightPhase::red;
}

void TrafficLight::waitForGreen()
{
    // FP.5b : add the implementation of the method waitForGreen, in which an infinite while-loop 
    // runs and repeatedly calls the receive function on the message queue. 
    // Once it receives TrafficLightPhase::green, the method returns.
}

TrafficLightPhase TrafficLight::getCurrentPhase()
{
    return _currentPhase;
}

void TrafficLight::simulate()
{
    // started in a thread
    threads.emplace_back(std::thread(&TrafficLight::cycleThroughPhases, this));
}

// virtual function which is executed in a thread
void TrafficLight::cycleThroughPhases()
{     
    // random duration
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<int> dist(4000, 6000);
    int duration = dist(gen);
        
    // infinite loop
    auto tick = std::chrono::system_clock::now();
    while (true) {
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
        // time elpase
        auto tock = std::chrono::system_clock::now();
        int elapse = (tock - tick).count();
        if (elapse > duration) {
            // toggles current phase
            if (_currentPhase == TrafficLightPhase::red) {
                _currentPhase = TrafficLightPhase::green;
                std::cout << "Traffic light #" << _id << ": phase turns green." << std::endl;
            }
            else {
                _currentPhase = TrafficLightPhase::red;
                std::cout << "Traffic light #" << _id << ": phase turns red." << std::endl;
            }
            // send update
            TrafficLightPhase message = TrafficLight::getCurrentPhase();
            //_queue.send(std::move(message)); // may be blocked
            auto sendFuture = std::async(std::launch::async, 
                                         &MessageQueue<TrafficLightPhase>::send,
                                         &_queue,
                                         std::move(message));
            // reset
            duration = dist(gen);
            auto tick = std::chrono::system_clock::now();
        }
    }
}