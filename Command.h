#ifndef __COMMAND_H_
#define __COMMAND_H_

#include <string>

/*************************************************************
This file defines the CommandsArguments class.
*************************************************************/

//This class is to pass arguments when implementing pipes
struct CommandsArguments {
	//record the file to redirect the standard input to
	std::string redirInput;
	//record the file to redirect the standard output to
	std::string redirOutput;
	//record the file to redirect the standard error to
	std::string redirError;
	//command arguments for groups of user input.
	std::vector<std::string> execArgs;

	//default constructor
	CommandsArguments(std::string _in, std::string _out, std::string _err, std::vector<std::string> & _args):
										redirInput(_in),
										redirOutput(_out),
										redirError(_err),
										execArgs(_args) {}
};

#endif
