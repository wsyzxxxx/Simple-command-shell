#ifndef __INTERPRETER_H_
#define __INTERPRETER_H_

#include <string>
#include <vector>
#include "configuration.h"
#include "Executor.h"
#include "Command.h"

/*************************************************************
This file defines the Interpreter class.
*************************************************************/

//Interpreter class is mainly for accepting and parsing the
//command from input, and output the result or error message.
//This is the class for user interaction.
class Interpreter {
private:
	//command arguments for every line of user input.
	std::vector<std::string> arguments;
	//commandName is the command to execute. Usually it is the
	//first argument of user input.
	std::string commandName;
	//current absolute path, for output before the $
	char currDirName[4096];
	//The main object to execute every commands.
	Executor executor;
	//record the file to redirect the standard input to
	std::string redirInput;
	//record the file to redirect the standard output to
	std::string redirOutput;
	//record the file to redirect the standard error to
	std::string redirError;
	//only when using pipes, used to pass the arguments
	std::vector<CommandsArguments> pipesArgs;


//The private functions are to deal with internal logics.
private:
	/* Function:		parseOneLine
	 * @Param:			std::string str : one line string from the user input
   * @Return:			void
   * @Exception:  This function may throw EXIT_SHELL and COMMAND_ERROR exceptions,
	 *              when there are errors or exit in the input command.
   *
   * Description: This fuction is to parse one signle line of command and 
	 *              separate the arguments and store them into the member variables.
   */	
	void parseOneLine(std::string line);

	/* Function:		expandVariables
	 * @Param:			std::string str : the string to be expanded
   * @Return:			(std::string)
   * @Exception:  This function won't throw any exceptions. (No Throw guaranteed) 
   *
   * Description: This fuction is to expand the $variables inside a string, which follows
	 *							the rules in the requirements file and TESTING.txt. Please notice that,
	 *							'$' can be escaped by a '\'.
   */	
	std::string expandVariables(std::string str);

	/* Function:		parseArguments
	 * @Param:			std::string str : one string waiting for parsing
   * @Return:			void
   * @Exception:  This function may throw COMMAND_ERROR exceptions, 
	 *              when there are only one non-escaped opened quotation " withoud another.
   *
   * Description: This fuction is to parse the arguments in the input, and check some 
	 *              invalid command during the parsing. When finishing the parsing, the arguements
	 *              will be set into the member arguments variable.
   */	
	void parseArguments(std::string str);
	
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
	void execCommand();

	/* Function:		isValidVariableNameChar
	 * @Param:			char c : the character to be checked
   * @Return:			(bool) true or false. Whether or not the character is valid.
   * @Exception:  This function won't throw any exceptions. (No Throw guaranted) 
   *
   * Description: This fuction is a help function to check if one character is valid
	 *							as part of the $var name. This is a helper function of checkValidVarName
   */	
	bool isValidVariableNameChar(char c);

	/* Function:		checkValidVarName
	 * @Param:			std::string str : the $var name to be checked
   * @Return:			void
   * @Exception:  This function may throw COMMAND_ERROR exceptions,
	 *							when the given $var name is not valid.
   *
   * Description: This function is to check the whole $var name, which calls isValidVariableNameChar
	 *              to check every character in the $var name.
   */	
	void checkValidVarName(std::string str);

	/* Function:		checkArgumentsSize
	 * @Param:			std::string operation: the command name of the arguments.
	 *							size_t			size:	arguments size of the current command.
	 *							size_t      requirement: the expected arguments of the specific command.
   * @Return:			void
   * @Exception:  This function may throw COMMAND_ERROR exceptions,
	 *							when the arguments size doesn't match the expected one.
   *
   * Description: This function is to check the arguments size of the specific command.
	 *							If the size matches, it will do nothing, but if not matches,
	 *							it will throw COMMAND_ERROR exceptions.
   */	
	void checkArgumentsSize(std::string operation, size_t size, size_t requirement);
	
	/* Function:		resetAllArguments
	 * @Param:			void
   * @Return:			void
   * @Exception:  This function won't throw any exceptions. (No Throw guaranteed) 
   *
   * Description: This function is to reset all the member variables (arguments) for the 
	 *              interpreter to accept and parse the next line of commands.
   */	
	void resetAllArguments();

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
	void loadRedirectFileName(std::string fileName, int redirStatus);
	
public:
	//default constructor
	Interpreter(): arguments(0),
								 commandName(""),
								 currDirName(),
								 executor(Executor()),
								 redirInput(""),
								 redirOutput(""),
								 redirError(""),
								 pipesArgs() {}
	//default destructor
	~Interpreter() {}

	/* Function:		startShell
	 * @Param:		  void
   * @Return:			void
   * @Exception:  This function may throw EXIT_SHELL exceptions,
	 *							when the shell would be exited, or errors happen in the child process. 
   *
   * Description: This function is the start point of the shell, which repeatedly accept
	 *							the input from user, parse and execute the commands by different methods,
	 *							and also handles the COMMAND_ERROR exceptions, print proper message.
   */	
	void startShell();


};

#endif
