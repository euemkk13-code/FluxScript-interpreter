
#pragma once

#include "value.h"

#include <string>
#include <vector>

namespace fluxgui {

struct Event {
    int id = 0;
    std::string kind;
    Value value;
};

// Native GUI functions
Value invoke(
    const std::string& name,
    const std::vector<Value>& args
);

// Window context
void enter(int id);
void leave();
int current();

void onClick(int id);
std::vector<int> poll();
std::vector<Event> pollEvents();

Value property(
    int id,
    const std::string& name
);
void setProperty(
    int id,
    const std::string& name,
    const Value& value
);
// timer()
int scheduleTimer(double seconds);
void cancelTimer(int id);
// GUI lifecycle
bool alive();
void shutdown();
}
