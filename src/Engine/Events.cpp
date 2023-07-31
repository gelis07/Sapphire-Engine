#include "Events.h"

bool Events::OnEvent(bool&& Condition, std::function<void()>&& Event)
{
    if(Condition){
        Event();
    }
    return false;
}