#include <iostream>
#include <random>
#include <future>
#include "TrafficLight.h"

/* Implementation of class "MessageQueue" */

template <typename T>
T MessageQueue<T>::receive()
{
    std::unique_lock<std::mutex> ulock(_mutex);
    _condition.wait(ulock, [this]{return !_queue.empty();});
    T message = std::move(_queue.back());
    _queue.pop_back();
  
    return message;
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
    // infinite loop
    while (true) {
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
        // until receive green
        if (_queue.receive() == TrafficLightPhase::green) {return;}
    }
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
    std::chrono::time_point<std::chrono::system_clock> tick, tock;
    tick = std::chrono::system_clock::now();
    while (true) {
        // avoid CPU overclocking
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
        // time elpase
        tock = std::chrono::system_clock::now();
        int elapse = std::chrono::duration_cast<std::chrono::milliseconds>(tock - tick).count();
        // std::cout << elapse << std::endl;
        if (elapse > duration) {
            // toggles current phase
            if (_currentPhase == TrafficLightPhase::red) {
                _currentPhase = TrafficLightPhase::green;
                //std::cout << "Traffic light #" << _id << ": phase turns green." << std::endl;
            }
            else {
                _currentPhase = TrafficLightPhase::red;
                //std::cout << "Traffic light #" << _id << ": phase turns red." << std::endl;
            }
            // send update
            TrafficLightPhase message = TrafficLight::getCurrentPhase();
            _queue.send(std::move(message)); // may be blocked
            /*
            auto sendFuture = std::async(std::launch::async, 
                                         &MessageQueue<TrafficLightPhase>::send,
                                         &_queue,
                                         std::move(message));
            */
            // reset
            duration = dist(gen);
            tick = std::chrono::system_clock::now();
        }
    }
}