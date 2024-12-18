#include <iostream>
#include <random>
#include "TrafficLight.h"


/* Implementation of class "MessageQueue" */
template <typename T>
T MessageQueue<T>::receive()
{
    std::unique_lock<std::mutex> ul(_mutex);
    _condition.wait(ul, [this]{return !_queue.empty();});

    T msg = std::move(_queue.front());
    _queue.pop_front();
    return std::move(msg);
}

template <typename T>
void MessageQueue<T>::send(T &&msg)
{
    std::lock_guard<std::mutex> lock(_mutex);
    _queue.emplace_back(msg);
    _condition.notify_one();
}


/* Implementation of class "TrafficLight" */
TrafficLight::TrafficLight() : _currentPhase(TrafficLightPhase::RED) {}

void TrafficLight::waitForGreen()
{
    while(true){
        TrafficLightPhase signal = _messages.receive();
        if(signal == TrafficLightPhase::GREEN)
            return;
    }
}

TrafficLightPhase TrafficLight::getCurrentPhase()
{
    return _currentPhase;
}

void TrafficLight::simulate()
{
    threads.emplace_back(std::thread(&TrafficLight::cycleThroughPhases, this));
}

void TrafficLight::cycleThroughPhases()
{
    std::random_device rd; 
    std::mt19937 eng(rd());
    std::uniform_int_distribution<> dist(4, 6);
    double cycle = dist(eng);
    std::chrono::system_clock::time_point startTime(std::chrono::system_clock::now());

    while(true){
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
        long passedTime = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now() - startTime).count();
        if(passedTime > cycle){
            // Toggle phase
            TrafficLightPhase _phase = (_currentPhase==TrafficLightPhase::RED ? TrafficLightPhase::GREEN : TrafficLightPhase::RED);
            _currentPhase = _phase;
            _messages.send(std::move(_phase));
            cycle = dist(eng);
            startTime = std::chrono::system_clock::now();
        }
    }
}