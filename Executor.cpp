#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <string>
#include <iostream>
#include <algorithm>
#include <sstream>
#include "Executor.h"
#include "exceptions.h"

/*************************************************************
This file implements all the methods in the Executor class
*************************************************************/

/* Function:		initialVariables
 * @Param:			void
 * @Return:			void
 * @Exception:  This function won't throw any exceptions. (No Throw guaranteed)
 *
 * Description: This fuction is to initial ECE551PATH from the system 'PATH',
 *							This would only run once during the process of construction
 */	
void Executor::initialVariables() {
	//get the "PATH" environment variable from the system
	char * envPATHPointer = getenv("PATH");
	setenv("ECE551PATH", envPATHPointer, 1);

	//set the PATH variable to ECE551PATH
	if (envPATHPointer) {
		std::string PATH(envPATHPointer);
		shellVariables["ECE551PATH"] = PATH;
	//in error case, leave ECE551PATH empty
	} else {
		shellVariables["ECE551PATH"] = "";
	}
}

/* Function:		initialPATH
 * @Param:			void
 * @Return:			void
 * @Exception:  This function won't throw any exceptions. (No Throw guaranteed)
 *
 * Description: This fuction is to initial ECE551PATH from the system 'PATH',
 *							This would only run once during the process of construction
 */	
void Executor::initialPATH() {
	//use stringstream to parse the ECE551PATH string
	std::istringstream streamifiedPATH(shellVariables.find("ECE551PATH")->second);
	std::string path;
	std::vector<std::string> tempECE551PATH;
	//to separate the ECE551PATH string with ':'
	while (std::getline(streamifiedPATH, path, ':')) {
		tempECE551PATH.push_back(path);
	}
	//temp and swap
	ECE551PATH = tempECE551PATH;
}

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
std::string Executor::searchPath(std::string programName) {
	//to accept the result of stat()
	struct stat statbuff;
	
	//if the programName contains '/', then will not search the ECE551PATH
	if (programName.find("/") != std::string::npos) {
		//check if the program exists in the path
		if (stat(programName.c_str(), &statbuff) == 0) {
			return programName;
		}
	//or search every paths in the ECE551PATH
	} else {
		for (size_t i = 0; i < ECE551PATH.size(); i++) {
			std::string fullPath = ECE551PATH[i] + "/" + programName;
			//if the program exists in the current path, return the fullpath
			if (stat(fullPath.c_str(), &statbuff) == 0) {
				return fullPath;
			}
		}
	}
	
	//no existing program found in all paths, throw the COMMAND_ERROR
	throw COMMAND_ERROR("Command " + programName + " not found\n");
	return "";
}

/* Function:		packageExecArgs
 * @Param:		  std::string & programName : the program name in C++ string
 *							std::vector<std::string> & arguments : the arguments in C++ container.
 *						  std::map<std::string, std::string> & enVariables : environment vars
 * @Return:			void
 * @Exception:  This function will not throw any exceptions.(No Throw guaranteed)
 *
 * Description: This fuction is to package the C++ style arguments into C-style strings,
 *              which would use new to allocate memory for every characters. The results
 *							are stored in the member variables.
 */	
void Executor::packageExecArgs(std::string & programName, std::vector<std::string> & arguments) {
	//allocate memory for C-style arguments
	CprogramName = strdup(programName.c_str());
	Carguments = new char*[arguments.size()+1];
	
	//put the arguments into the C arguments array
	for (size_t i = 0; i < arguments.size(); i++) {
		Carguments[i] = strdup(arguments[i].c_str());
	}
	//the last item should be NULL
	Carguments[arguments.size()] = NULL;
}

/* Function:		freeExecArgs
 * @Param:		  size_t argumentsNum
 * @Return:			void
 * @Exception:  This function will not throw any exceptions.(No Throw guaranteed)
 *
 * Description: This fuction is to free all C-style strings memory, when errors happen in
 *							the child process, which would delete the memory and finally set all the
 *							pointers to NULL.
 */	
void Executor::freeExecArgs(size_t argumentsNum) {
	//delete name memory
	free(CprogramName);

	//delete arguments memory
	for (size_t i = 0; i <= argumentsNum; i++) {
		free(Carguments[i]);
	}
	delete[] Carguments;

	//set all the member pointers to be NULL
	CprogramName = NULL;
	Carguments = NULL;
}

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
void Executor::redirInAndOut(std::string redirInput, std::string redirOutput, std::string redirError) {
	//redirect standard input
	if (redirInput != "") {
		int fdInput;
		//open the redirect input file
		if ((fdInput = open(redirInput.c_str(), O_RDONLY)) == -1) {
			perror("ERROR: open() error");
			throw EXIT_SHELL(false);
		}
		if (dup2(fdInput, 0) == -1) {
			perror("ERROR: dup2() error");
			throw EXIT_SHELL(false);
		}
		close(fdInput);
	}

	//redirect standard output
	if (redirOutput != "") {
		int fdOutput;
		if ((fdOutput = open(redirOutput.c_str(), O_CREAT|O_WRONLY|O_TRUNC, 0644)) == -1) {
			perror("ERROR: open() error");
			throw EXIT_SHELL(false);
		}
		if (dup2(fdOutput, 1) < -1) {
			perror("ERROR: dup2() error");
			throw EXIT_SHELL(false);
		}
		close(fdOutput);
	}

	//redirect standard error
	if (redirError != "") {
		int fdError;
		if ((fdError = open(redirError.c_str(), O_CREAT|O_WRONLY|O_TRUNC, 0644)) == -1) {
			perror("ERROR: open() error");
			close(fdError);
			throw EXIT_SHELL(false);
		}
		if (dup2(fdError, 2) == -1) {
			perror("ERROR: dup2() error");
			close(fdError);
			throw EXIT_SHELL(false);
		}
		close(fdError);
	}
}

//import environment variables from system
extern char ** environ;

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
pid_t Executor::execOneProgram(std::vector<std::string> arguments, std::string redirInput, 
															std::string redirOutput, std::string redirError, int inputFd,
															int outputFd) {
	//pid of the child process
	pid_t pid;

	//check the argument number
	if (arguments.size() == 0) {
		throw COMMAND_ERROR("ERROR: wrong parameters!\n");
	}

	//search the full path for the program
	arguments[0] = searchPath(arguments[0]);

	if (arguments[0] != "") {
		pid = fork();
		//check fork error
		if (pid < 0) {
			perror("ERROR: fork() error");

		//in the child process
		} else if (pid == 0) {
			//redirect input
			if (inputFd != 0) {
				if (dup2(inputFd, 0) == -1) {
					perror("ERROR: dup2() error");
					throw COMMAND_ERROR("");
				}
				close(inputFd);
			}
			//redirect output
			if (outputFd != 1) {
				if (dup2(outputFd, 1) == -1) {
					perror("ERROR: dup2() error");
					throw COMMAND_ERROR("");
				}
				close(outputFd);
			}
			//redirect input and output
			redirInAndOut(redirInput, redirOutput, redirError);
			//package the C arguments
			packageExecArgs(arguments[0], arguments);
			//execute the program
			execve(CprogramName, Carguments, environ);
			//error happens
			perror("ERROR: execve() error");
			//delete all the memory
			freeExecArgs(arguments.size());
			//throw the exceptions when error happens
			throw EXIT_SHELL(false);
		}
	}
	
	//return the current pid
	return pid;
}

/* Function:		printExitStatus
 * @Param:		  int status : exit status of current process
 * @Return:			void
 * @Exception:  This function will not throw any exceptions.(No Throw guaranteed)
 *
 * Description: This fuction is to print exit status of the current process.
 */	
void Executor::printExitStatus(int status) {
	//check different finish status
	if (WIFEXITED(status)) {
		//successfully finish
		if (WEXITSTATUS(status) == 0) {
			std::cout << "Program was successful" << std::endl;
		//failed to finish
		} else {
			std::cout << "Program failed with code " << WEXITSTATUS(status) << std::endl;
		}
	//terminated by some signals
	} else if (WIFSIGNALED(status)) {
		std::cout << "Terminated by signal " << WTERMSIG(status) << std::endl;
	//enter other status - stop ,etc.
	} else {
		std::cout << "Program enter other status" << std::endl;
	}
}

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
void Executor::executeProgram(std::string programName, std::vector<std::string> arguments,
															std::string redirInput, std::string redirOutput, std::string redirError) {
	//search the program in the paths first, get the full paths of the program
	programName = searchPath(programName);
	
	//execute the program
	if (programName != "") {
		//create child process
		pid_t currPid, waitPid;
		currPid = fork();

		//fail to create child process
		if (currPid < 0) {
			perror("ERROR: fork() error");

		//in the paraent process, wait for child process to finish
		} else if (currPid > 0) {
			int status;
			//wait
			waitPid = waitpid(currPid, &status, 0);
			
			//check the return status
			if (waitPid == -1) {
				perror("ERROR: waitpid() error");
				return;
			}

			//print the exit status
			printExitStatus(status);

		//in the child process - execute the program
		} else {			
			//redirect input and output
			redirInAndOut(redirInput, redirOutput, redirError);
			//package the C arguments
			packageExecArgs(programName, arguments);
			//execute the program
			execve(CprogramName, Carguments, environ);
			//error happens
			perror("ERROR: execve() error");
			//delete all the memory
			freeExecArgs(arguments.size());
			//throw the exceptions when error happens
			throw EXIT_SHELL(false);
		}
	}
}

/* Function:		changeDir
 * @Param:		  const char * dirName : the directory name to be changed into
 * @Return:			void
 * @Exception:  This function will not throw any exceptions.(No Throw guaranteed)
 *
 * Description: This fuction is to change the current directory of the shell.
 *							If error happens, it will print the error message, and nothing changes.
 */	
void Executor::changeDir(const char * dirName) {
	if (dirName) {
		if (chdir(dirName) == -1) {
			perror("ERROR: chdir() error");
		}
	}
}

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
void Executor::setVariables(std::string variableName, std::string value) {
	//look for the variable
	std::map<std::string, std::string>::iterator it = shellVariables.find(variableName);
	//update value
	if (it != shellVariables.end()) {
		it->second = value;
	//create new item
	} else {
		shellVariables[variableName] = value;
	}
	//reinitilize the searching paths, when updating ECE551PATH
	if (variableName == "ECE551PATH") {
		initialPATH();
	}
}

/* Function:		lookupVariable
 * @Param:		  std::string variableName : the name of $var to look up
 * @Return:			(std::string) the value of the $var
 * @Exception:  This function will not throw any exceptions.(No Throw guaranteed)
 *
 * Description: This fuction is to lookup the value of $var in the current shell
 *							If the value exists, then it will be returned. If the value does
 *							not exist, then it will return "".
 */	
std::string Executor::lookupVariable(std::string variableName) {
	//look for the variable
	std::map<std::string, std::string>::iterator it = shellVariables.find(variableName);
	if (it != shellVariables.end()) { //found
		return it->second;
	}
	//not found
	return ""; 
}

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
void Executor::exportVariable(std::string variableName) {
	//look for the variable
	std::map<std::string, std::string>::iterator it = shellVariables.find(variableName);
	if (it != shellVariables.end()) { // found, set it into environment
		if (setenv(variableName.c_str(), it->second.c_str(), 1) == -1) {
			perror("ERROR: setenv() error");
		}
	} else { // not found, throw exception
		throw COMMAND_ERROR("ERROR: export: var " + variableName + " does not exist\n");
	}
}

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
void Executor::reverseVariable(std::string variableName) {
	//look for the variable
	std::map<std::string, std::string>::iterator it = shellVariables.find(variableName);
	if (it != shellVariables.end()) { //found, reverse the value
		std::reverse(it->second.begin(), it->second.end());
	} else { //not found, throw exception
		throw COMMAND_ERROR("ERROR: rev: var " + variableName + " does not exist\n");
	}
	//reinitilize the searching paths, when updating ECE551PATH
	if (variableName == "ECE551PATH") {
		initialPATH();
	}
}

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
void Executor::execPipeCommands(std::vector<CommandsArguments> & commandsList) {
	//pid of the child process
	pid_t childPid;
	//lastOutput File Descriptor, start from the standard input
	int lastOutput = 0;
	//array to store the file descriptor in the pipe
	int pipeFd[2];
	//store the commandsList size for readability
	size_t n = commandsList.size();

	//iteratively create pipe and execute the programs
	for (size_t i = 0; i < n - 1; i++) {
		//create a new pipe for current programs
		if (pipe(pipeFd) == -1) {
			perror("ERROR: pipe() ");
			throw COMMAND_ERROR("");
		}

		//exec current program
		execOneProgram(commandsList[i].execArgs,
									 commandsList[i].redirInput,
									 commandsList[i].redirOutput,
									 commandsList[i].redirError,
									 lastOutput,
									 pipeFd[1]);
		//close the write end of the pipe
		close(pipeFd[1]);
		//set the read end of the pipe
		lastOutput = pipeFd[0];
	}
	//execute the last program without a new pipe
	childPid = execOneProgram(commandsList[n-1].execArgs,
								 commandsList[n-1].redirInput,
								 commandsList[n-1].redirOutput,
								 commandsList[n-1].redirError,
								 lastOutput,
								 1); // standard output
	
	int status;
	//wait for the child process to finish
	pid_t waitPid = waitpid(childPid, &status, 0);
			
	//check the return status
	if (waitPid == -1) {
		perror("ERROR: waitpid() error");
		return;
	}
	
	//print the exit status
	printExitStatus(status);
}
