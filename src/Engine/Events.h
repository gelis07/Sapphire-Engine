#pragma once
#include "Utilities.hpp"


enum EventType{
    ApplicationEvent, MouseInput,
    KeyInput
};

class Events{
    public:
        static bool OnEvent(bool&& Condition, std::function<void()>&& Event);
};  

