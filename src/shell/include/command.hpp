#pragma once
#include <string>
#include <vector>

using namespace std;

struct SimpleCommand{
    vector<string> argv; // command and arguments
    string input_redirection; // input redirection file
    string output_redirection; // output redirection file
};

struct Command{
    vector<SimpleCommand> commands; // pipeline commands (size >= 1)
    bool background=false; // whether the command should run in the background
    string original_line; // raw input for job control/debugging
};
