#include <string>
#include <sstream>
#include <iostream>
#include <cstring>
#include <cstdlib>
#include "Interpreter.h"
#include "Executor.h"
#include "exceptions.h"
#include "Command.h"

/*************************************************************
This file implements all the methods in the Interpreter class.
*************************************************************/

/* Function:		parseOneLine
 * @Param:			std::string str : one line string from the user input
 * @Return:			void 
 * @Exception:  This function may throw EXIT_SHELL and COMMAND_ERROR exceptions,
 *              when there are errors or exit in the input command.
 *
 * Description: This fuction is to parse one signle line of command and 
 *              separate the arguments and store them into the member variables.
 */	
void Interpreter::parseOneLine(std::string line) {
	//empty command
	if (line == "") {
		throw COMMAND_ERROR("");
	}
	
	//reset the member variables of interpreter
	resetAllArguments();
	pipesArgs.clear();

	//parse and fill the arguments
	parseArguments(line); 
	//load the first argument into commandName
	if (arguments.size() > 0) {
		commandName = arguments[0];
	} else {
		throw COMMAND_ERROR("");
	}

	//exit command
	if (commandName == EXIT_COMMAND) {
		throw EXIT_SHELL(true);
	}
}

/* Function:		expandVariables
 * @Param:			std::string str : the string to be expanded
 * @Return:			(std::string)
 * @Exception:  This function won't throw any exceptions. (No Throw guaranteed) 
 *
 * Description: This fuction is to expand the $variables inside a string, which follows
 *							the rules in the requirements file and TESTING.txt. Please notice that,
 *							'$' can be escaped by a '\'.
 */	
std::string Interpreter::expandVariables(std::string str) {
	std::string res = "";

	//parse the string
	for (size_t i = 0; i < str.size();) {
		//meet a '\', the next '$' may be escaped.
		if (str[i] == '\\' && i+1 < str.size() && str[i+1] == '$') {
			i++;
			res += str[i++];
		}

		//meet a '$', start to expand the $var
		if (str[i] == '$') {
			i++;
			std::string variableName = "";

			//find the longest possible $var name
			while (i < str.size() && isValidVariableNameChar(str[i])) {
				variableName += str[i++];
			}

			//find the value of the variable
			res += executor.lookupVariable(variableName);

		//normal characters
		} else {
			res += str[i++];
		}
	}
	return res;
}

/* Function:		parseArguments
 * @Param:			std::string str : one string waiting for parsing
 * @Return:			void
 * @Exception:  This function may throw COMMAND_ERROR exceptions, 
 *              when there are only one non-escaped opened quotation " withoud another.
 *
 * Description: This fuction is to parse the arguments in the input, and check some
 *              invalid command during the parsing. When finishing the parsing, the arguement
 *              will be set into the member arguments variable.
 */	
void Interpreter::parseArguments(std::string str) {
	//first of all, expand $var
	str = expandVariables(str);
	//current status of whether or not inside a quotation
	bool insideQuotation = false;
	//current status of whether or not to accept redirect file from next argument
	//identify next argument is a redirect file name or normal arguments
	// 0 - not to accept redirect file
	// 1 - to accept redirect stdin
	// 2 - to accept redirect stdout
	// 3 - to accept redirect stderr
	int redirectStatus = 0;
	//current status of whether or not to expect a pipe argument group
	bool expectedPipeArg = false;
	//temp variable to accept next argument
	std::string nextArg = "";

	//parse the arguments
	for (size_t i = 0; i < str.size();) {
		//all the characters after '\' should be escaped when outside the quotations
		if (insideQuotation == false &&  //when outside the quotation
				str[i] == '\\'					 &&  //current one is '\'
				i+1 < str.size()) {		 			 //next char still exists
			//record the next char and ignore the current '\'
			nextArg += str[i+1];
			i += 2;
		//some characters after '\' should still be escaped when inside the quotations
		} else if (insideQuotation == true &&  //when inside the quotations
							 str[i] == '\\'				 	 &&  //current one is '\'
							 i+1 < str.size()				 &&	 //next char still exists
							 (str[i+1] == '\"'			 ||  //next char is '"'
							  str[i+1] == '\\')) { 			 //next char is '\'
			nextArg += str[i+1];
			i += 2;
		//meet a ' ' or '|' outside quotation, the end of an argument or a group of arguments.
		} else if ((str[i] == ' ' || str[i] == '|') && insideQuotation == false) {
			//check if the argument is a redirect file name
			if (redirectStatus != 0) {
				loadRedirectFileName(nextArg, redirectStatus);
			//only when nextArg is not empty, push it into arguments
			} else if (nextArg != "") {
				arguments.push_back(nextArg);
			}
			//check the pipe sign
			if (str[i] == '|') {
				//check argument group size
				if (arguments.size() == 0) {
					throw COMMAND_ERROR("ERROR: empty arguments for pipe\n");
				}
				//save the current group
				pipesArgs.push_back(CommandsArguments(
														redirInput,
														redirOutput,
														redirError,
														arguments));
				resetAllArguments();
				//there should be a pipe argument group after '|'
				expectedPipeArg = true;
			}
			//reset nextArg
			nextArg = "";
			//reset redirect file status
			redirectStatus = 0;
			//ignore the consecutive whitespace
			while (i < str.size() && str[++i] == ' ');
		//meet a '"', start or end of the quotation status
		} else if (str[i] == '\"') {
			insideQuotation = !insideQuotation;
			i++;
		//meet a '>' and outside the quotation, then it is a redirect input file
		} else if (str[i] == '>' && insideQuotation == false) {
			if (redirectStatus != 0) {
				throw COMMAND_ERROR("ERROR: unexpected '>' in the parameters\n");
			}
			redirectStatus = 2;
			//ignore the consecutive whitespace
			while (i < str.size() && str[++i] == ' ');
		//meet a '<' and outside the quotation, then it is a redirect output file
		} else if (str[i] == '<' && insideQuotation == false) {			
			if (redirectStatus != 0) {
				throw COMMAND_ERROR("ERROR: unexpected '<' in the parameters\n");
			}
			redirectStatus = 1;
			//ignore the consecutive whitespace
			while (i < str.size() && str[++i] == ' ');
		//meet a '2>' and outside the quotation, then it is a redirect stderr file
		} else if (str[i] == '2' && i+1 < str.size() && str[i+1] == '>' && insideQuotation == false) {
			if (redirectStatus != 0) {
				throw COMMAND_ERROR("ERROR: unexpected '2>' in the parameters\n");
			}
			redirectStatus = 3;
			i++;
			//ignore the consecutive whitespace
			while (i < str.size() && str[++i] == ' ');
		//meet a '|' and use pipes to execute the programs
		} else if (str[i] == '|' && insideQuotation == false) {
		//normal characters
		} else {
			nextArg += str[i++];
		}
	}
	//at the end of line, still inside the quotation, it's an invalid command
	if (insideQuotation == true) {
		throw COMMAND_ERROR("ERROR: wrong command parameters\n");
	}
	//fill the last argument into arguments
	if (nextArg != "") {
		//check if the argument is a redirect file name
		if (redirectStatus != 0) {
			loadRedirectFileName(nextArg, redirectStatus);
		//normal arguements
		} else {
			arguments.push_back(nextArg);
		}
		redirectStatus = 0;
	}
	//check the redirect status
	if (redirectStatus != 0) {
		throw COMMAND_ERROR("ERROR: wrong redirect file name!\n");
	}
	//maybe last argument group of pipe
	if (expectedPipeArg == true) {
		//check argument group size
		if (arguments.size() == 0) {
			throw COMMAND_ERROR("ERROR: empty arguments for pipe\n");
		}
		//save the current group
		pipesArgs.push_back(CommandsArguments(
												redirInput,
												redirOutput,
												redirError,
												arguments));
	}
	
	//`set` command needs to leave the value argument not escaped
	if (arguments.size() >= 3 && arguments[0] == "set") {
		std::string varName = arguments[1];
		checkValidVarName(varName);
		
		//find the $var name field
		size_t varNamePos = str.find(arguments[1]);
		varNamePos += arguments[1].size();
		while (str[varNamePos] == ' ') {
			varNamePos++;
		}

		//the rest of line is value field, start from nonspace.
		arguments.clear();
		arguments.push_back("set");
		arguments.push_back(varName);
		arguments.push_back(str.substr(varNamePos));
	}

	//check the valid $var name for export and rev command
	if (arguments.size() >= 2 && (arguments[0] == "export" || arguments[0] == "rev")) {
		checkValidVarName(arguments[1]);
	}
}

/* Function:		execCommand
 * @Param:			void
 * @Return:			void
 * @Exception:  This function may throw COMMAND_ERROR exceptions,
 *              when there are errors in the executing process of the command.
 *							This function may also throw EXIT_SHELL exceptions when the 
 *						  child process failed to execute one program.
 *
 * Description: This fuction is to execute different commands. It calls different
 *              methods in executor object to execute the commands and checks the
 *              number of the arguments.
 */	
void Interpreter::execCommand() {
	//in pipe mode, call different functions
	if (pipesArgs.size() > 0) {
		executor.execPipeCommands(pipesArgs);
	//'cd' command
	} else if (commandName == "cd") {
		if (arguments.size() == 2) {
			executor.changeDir(arguments[1].c_str());
		} else if (arguments.size() == 1) {
			executor.changeDir();
		} else {
			checkArgumentsSize("cd", arguments.size(), 2);
		}

	//'set' command
	} else if (commandName == "set") {
		checkArgumentsSize("set", arguments.size(), 3);
		executor.setVariables(arguments[1], arguments[2]);
	
	//'export' command
	} else if (commandName == "export") {
		checkArgumentsSize("export", arguments.size(), 2);
		executor.exportVariable(arguments[1]);
	
	//'rev' command
	} else if (commandName == "rev") {
		checkArgumentsSize("rev", arguments.size(), 2);
		executor.reverseVariable(arguments[1]);

	//when the line only contains spaces, do nothing
	} else if (commandName == "") {
		throw COMMAND_ERROR("");

	//'other' executable programs
	} else {
		executor.executeProgram(commandName, arguments, redirInput, redirOutput, redirError);
	}
}

/* Function:		isValidVariableNameChar
 * @Param:			char c : the character to be checked
 * @Return:			(bool) true or false. Whether or not the character is valid.
 * @Exception:  This function won't throw any exceptions. (No Throw guaranted) 
 *
 * Description: This fuction is a help function to check if one character is valid
 *							as part of the $var name. This is a helper function of checkValidVarName
 */	
bool Interpreter::isValidVariableNameChar(char c) {
	// a valid character should be 'a'-'z', 'A'-'Z', '0'-'9' or '_'
	return (c >= 'a' && c <= 'z') || 
				 (c >= 'A' && c <= 'Z') || 
				 (c >= '0' && c <= '9') ||
				 (c == '_');
}

/* Function:		checkValidVarName
 * @Param:			std::string str : the $var name to be checked
 * @Return:			void
 * @Exception:  This function may throw COMMAND_ERROR exceptions,
 *							when the given $var name is not valid.
 *
 * Description: This function is to check the whole $var name, which calls isValidVariableNameChar
 *              to check every character in the $var name.
 */	
void Interpreter::checkValidVarName(std::string str) {
	for (size_t i = 0; i < str.size(); i++) {
		if (!isValidVariableNameChar(str[i])) {
			throw COMMAND_ERROR("ERROR: wrong set parameters\n");
		}
	}
}

/* Function:		checkArgumentsSize
 * @Param:			std::string operation: the command name of the arguments.
 *							size_t			size:	arguments size of the current command.
 *							size_t      requirement: the expected arguments of the specific command.
 * @Return:			void
 * @Exception:  This function may throw COMMAND_ERROR exceptions
 *							when the arguments size doesn't match the expected one.
 *
 * Description: This function is to check the arguments size of the specific command.
 *							If the size matches, it will do nothing, but if not matches,
 *							it will throw COMMAND_ERROR exceptions.
 */	
void Interpreter::checkArgumentsSize(std::string operation, size_t size, size_t requirement) {
	if (size != requirement) {
		throw COMMAND_ERROR("ERROR: " + operation + ": wrong parameter numbers\n");
	}
}

/* Function:		resetAllArguments
 * @Param:			void
 * @Return:			void
 * @Exception:  This function won't throw any exceptions. (No Throw guaranteed) 
 *
 * Description: This function is to reset all the member variables (arguments) for the 
 *              interpreter to accept and parse the next line of commands.
 */	
void Interpreter::resetAllArguments() {
	arguments.clear();
	commandName = "";
	redirInput = "";
	redirOutput = "";
	redirError = "";
}

/* Function:		loadRedirectFileName
 * @Param:			std::string fileName: the parsed file name of the redirect file
 *							int					redirStatus: the current redirect status
 * @Return:			void
 * @Exception:  This function may throw COMMAND_ERROR exceptions,
 *              when the redirect file name is empty or redirect status is not correct.
 *
 * Description: This function is to load the current argument into the redirect file
 *              name, including redirInput, redirOutput, and redirError. But if the
 *              redirStatus is not as desired, it will throw COMMAND_ERROR exceptions.
 */	
void Interpreter::loadRedirectFileName(std::string fileName, int redirStatus) {
	//check the argument
	if (fileName == "") {
		throw COMMAND_ERROR("ERROR: wrong redirect file name!\n");
	}
	//set the redirect file
	switch (redirStatus) {
		case 1: //this is a redirect input
			redirInput = fileName;
			break;
		case 2: //this is a redirect output
			redirOutput = fileName;
			break;
		case 3: //this is a redirect stderr
			redirError = fileName;
			break;
		default: // unexpected situations
			throw COMMAND_ERROR("ERROR: wrong redirect status!\n");
	}
}

/* Function:		startShell
 * @Param:		  void
 * @Return:			void
 * @Exception:  This function may throw EXIT_SHELL exceptions,
 *							when the shell would be exited, or errors happen in the child process 
 *
 * Description: This function is the start point of the shell, which repeatedly accept
 *							the input from user, parse and execute the commands by different methods,
 *							and also handles the COMMAND_ERROR exceptions, print proper message.
 */	
void Interpreter::startShell() {
	std::string inputCommand = "";
	
	//print the shell information
	std::cout << SHELL_NAME << ":" << realpath("./", currDirName) << "$ ";
	//get one line
	while (std::getline(std::cin, inputCommand)) {
		try {
			//parse the line
			parseOneLine(inputCommand);
			//execute the line
			execCommand();
		//catch and print the command error
		} catch (COMMAND_ERROR & e) {
			std::cout << e.whatInString();
		}
		//print the shell information for next line
		std::cout << SHELL_NAME << ":" << realpath("./", currDirName) << "$ ";
	}
	//for exit with ctrl-D
	std::cout << std::endl;
}

