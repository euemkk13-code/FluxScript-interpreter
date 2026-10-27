#pragma once
#include "value.h"
#include <string>
#include <vector>
namespace fluxgui {
Value invoke(const std::string& name, const std::vector<Value>& args);
void enter(int id);
void leave();
int current();
void onClick(int id);
std::vector<int> poll();
bool alive();
void shutdown();
}
