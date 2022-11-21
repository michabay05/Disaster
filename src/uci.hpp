#pragma once

#include <iostream>
#include <string>

namespace UCI {
void loop();
//void parse(const std::string command);
void parse(char *command);
void parsePos(const char *command);
//void parseGo(const std::string command);
void parseGo(const char *command);
void printEngineInfo();
void printHelpInfo();
} // namespace UCI
