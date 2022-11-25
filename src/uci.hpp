#pragma once

#include <iostream>
#include <string>

namespace UCI {
extern bool quit;
extern bool stop;
void loop();
void parse(const std::string &command);
void parsePos(const std::string &command);
void parseGo(const std::string &command);
void parseParam(const std::string& cmdArgs, const std::string& cmdName, int& output);
void checkUp();
void printEngineInfo();
void printHelpInfo();
} // namespace UCI
