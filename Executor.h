#ifndef __EXECUTOR_H_
#define __EXECUTOR_H_

#include <vector>
#include <string>
#include <map>
#include "Command.h"

/*************************************************************
This file defines the Executor class.
*************************************************************/

//Executor class is mainly for executing all kinds of command,
//and it stores the environment variables of current shell.
//It calls some C libarary functions to execute the program,
//get the environment variables and create child process.
//Besides, it use the child process to execute the executable file.
class Executor {
private:
	//store the ECE551PATH for searching executable program
	std::vector<std::string> ECE551PATH;
	//store the $vars in current shell
	std::map<std::string, std::string> shellVariables;
	//These three variables are for calling C libraries.
	char* CprogramName;
	char** Carguments;

//The private functions are to deal with internal logics.
private:
	/* Function:		initialVariables
	 * @Param:			void
   * @Return:			void
   * @Exception:  This function won't throw any exceptions. (No Throw guaranteed)
   *
   * Description: This fuction is to initial ECE551PATH from the system 'PATH',
	 *							This would only run once during the process of construction
   */	
	void initialVariables();

	/* Function:		initialPATH
	 * @Param:			void
   * @Return:			void
   * @Exception:  This function won't throw any exceptions. (No Throw guaranteed)
   *
   * Description: This fuction is to initial ECE551PATH from the system 'PATH',
	 *							This would only run once during the process of construction
   */	
	void initialPATH();

	/* Function:		searchPath
	 * @Param:			std::string programName : the program to be executed
   * @Return:			(std::string) The existing path of the current programName
   * @Exception:  This function may throw COMMAND_ERROR exceptions,
	 *							when all the path in ECE551PATH doesn't contain the program.
   *
   * Description: This fuction is to search the program in the ECE551PATH. When
	 *							the program exists in one path, it will return the fullpath
	 *							of the program, or if it can't find the program, then it will
	 *							throw a COMMAND_ERROR exception.
   */	
	std::string searchPath(std::string programName);

	/* Function:		packageExecArgs
	 * @Param:		  std::string & programName : the program name in C++ string
	 *							std::vector<std::string> & arguments : the arguments in C++ container.
   * @Return:			void
   * @Exception:  This function will not throw any exceptions.(No Throw guaranteed)
   *
   * Description: This fuction is to package the C++ style arguments into C-style strings,
	 *              which would use new to allocate memory for every characters. The results
	 *							are stored in the member variables.
   */	
	void packageExecArgs(std::string & programName, 
											 std::vector<std::string> & arguments); 

	/* Function:		freeExecArgs
	 * @Param:		  size_t argumentsNum : current argument number
   * @Return:			void
   * @Exception:  This function will not throw any exceptions.(No Throw guaranteed)
   *
   * Description: This fuction is to free all C-style strings memory, when errors happen in
	 *							the child process, which would delete the memory and finally set all the
	 *							pointers to NULL.
   */	
	void freeExecArgs(size_t argumentsNum);

	/* Function:		redirInAndOut
	 * @Param:		  std::string redirInput : the file name of redirect input, when "" no redirect
	 *							std::string redirOutput : the file name of redirect output, when "" no redirect
	 *							std::string redirError : the file name of redirect stderr, when "" no redirect
   * @Return:			void
   * @Exception:  This function may throw EXIT_SHELL exceptions when the system calls inside
	 *              the redirect process meet with errors.
   *
   * Description: This fuction is to redirect the standard input, standard output, and standard 
	 *              error when the arguments are not empty, and check the redirect status. If 
	 *              redirection fails, it will thrwo EXIT_SHELL exceptions.
	 *              This function only would be called in the child process when executing programs.
   */	
	void redirInAndOut(std::string redirInput, std::string redirOutput, std::string redirError);

	/* Function:		execOneProgram
	 * @Param:		  std::vector<std::string> arguments : the arguments in C++ style
	 *							std::string redirInput : the file name of redirect input, when "" no redirect
	 *							std::string redirOutput : the file name of redirect output, when "" no redirect
	 *							std::string redirError : the file name of redirect stderr, when "" no redirect
	 *							int inputFd : the default input file descriptor in the pipe
	 *							int outputFd : the default output file descriptor in the pipe
   * @Return:			pid_t : pid of the child process to execute the program
   * @Exception:  This function may throw COMMAND_ERROR exceptions when the program doesn't
	 *							exist in any paths, and may throw EXIT_SHELL exceptions when the child
	 *							process cannot execute the program successfully.
   *
   * Description: This fuction is to execute a program with arguments and environments variables.
	 *							It will first search the program through ECE551PATH, and then package all the
	 *							arguments, create child process to execute the program, this private method
	 *              is just for the pipe mode programs.
   */	
	pid_t execOneProgram(std::vector<std::string> arguments, std::string redirInput, 
											 std::string redirOutput, std::string redirError, int inputFd,
											 int outputFd);

	/* Function:		printExitStatus
	 * @Param:		  int status : exit status of current process
   * @Return:			void
   * @Exception:  This function will not throw any exceptions.(No Throw guaranteed)
   *
   * Description: This fuction is to print exit status of the current process.
   */	
	void printExitStatus(int status);

public:
	//default constructor
	Executor(): ECE551PATH(0),
							shellVariables(),
							CprogramName(NULL), 
							Carguments(NULL) {initialVariables();initialPATH();}
	//default destructor
	~Executor() {}

	/* Function:		executeProgram
	 * @Param:		  std::string programName : the program name to be executed
	 *							std::vector<std::string> arguments : the arguments in C++ style
	 *							std::string redirInput : the file name of redirect input, when "" no redirect
	 *							std::string redirOutput : the file name of redirect output, when "" no redirect
	 *							std::string redirError : the file name of redirect stderr, when "" no redirect
   * @Return:			void
   * @Exception:  This function may throw COMMAND_ERROR exceptions when the program doesn't
	 *							exist in any paths, and may throw EXIT_SHELL exceptions when the child
	 *							process cannot execute the program successfully.
   *
   * Description: This fuction is to execute a program with arguments and environments variables.
	 *							It will first search the program through ECE551PATH, and then package all the
	 *							arguments, create child process to execute the program.
   */	
	void executeProgram(std::string programName, std::vector<std::string> arguments,
											std::string redirInput, std::string redirOutput, std::string redirError);

	/* Function:		changeDir
	 * @Param:		  const char * dirName : the directory name to be changed into
   * @Return:			void
   * @Exception:  This function will not throw any exceptions.(No Throw guaranteed)
   *
   * Description: This fuction is to change the current directory of the shell.
	 *							If error happens, it will print the error message, and nothing changes.
   */	
	void changeDir(const char * dirName = getenv("HOME"));

	/* Function:		setVariables
	 * @Param:		  std::string variableName : the name of the shell variable to be set
	 *							std::string value : the value of the shell variable to be set
   * @Return:			void
   * @Exception:  This function will not throw any exceptions.(No Throw guaranteed)
   *
   * Description: This fuction is to set a new variable or change the value of
	 *							the existing variable. If the $var doesn't exist, it will create a
	 *              new one. If $var exists, it will set the new vaule. Besides, if the
	 *              $var is ECE551PATH, it will update the paths to look for programs.
   */	
	void setVariables(std::string variableName, std::string value);

	/* Function:		lookupVariable
	 * @Param:		  std::string variableName : the name of $var to look up
   * @Return:			(std::string) the value of the $var
   * @Exception:  This function will not throw any exceptions.(No Throw guaranteed)
   *
   * Description: This fuction is to lookup the value of $var in the current shell
	 *							If the value exists, then it will be returned. If the value does
	 *							not exist, then it will return "".
   */	
	std::string lookupVariable(std::string variableName);

	/* Function:		exportVariable
	 * @Param:		  std::string variableName : the name of $var to be exported
   * @Return:			void
   * @Exception:  This function may throw COMMAND_ERROR exceptions, when
	 *							the $var does not exist.
   *
   * Description: This fuction is to export the variable into the environment variables.
	 *							Then when the following programs executed, the environment variables will
	 *							be used to pass as part of parameters.Besides, if the $var does not exist
	 *							it will throw COMMAND_ERROR exceptions.
   */	
	void exportVariable(std::string variableName);

	/* Function:		reverseVariable
	 * @Param:		  std::string variableName : the name of $var to be reversed
   * @Return:			void
   * @Exception:  This function may throw COMMAND_ERROR exceptions, when
	 *							the $var does not exist.
   *
   * Description: This fuction is to reverse the variable in the current shell.
	 *							If the ECE551PATH be used, then it will reinitialize the paths
	 *							for searching the programs.
   */	
	void reverseVariable(std::string variableName);

	/* Function:		execPipeCommands
	 * @Param:		  std::vector<CommandsArguments> & commandsList : the commands to be executed
   * @Return:			void
   * @Exception:  This function may throw COMMAND_ERROR exceptions, when
	 *							there are errors in the parameters and may throw SHELL_EXIT exceptions,
	 *							when there are errors in the child process to execute the program.
   *
   * Description: This fuction is to execute the programs in the pipe mode, and may throw
	 *							exceptions in many error situations. If succeeding, it will print the
	 *							output of the last program and the exit status of the last program.
   */	
	void execPipeCommands(std::vector<CommandsArguments> & commandsList);
};

#endif
