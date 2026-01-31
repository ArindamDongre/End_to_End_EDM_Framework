#pragma once
#include <string>
#include "command.hpp"

using namespace std;

bool parse_command(const string &line, Command &out, string &error); // parse the command line into Command structure
// returns true if parsing is successful, false otherwise
// in case of failure, error contains the error message


