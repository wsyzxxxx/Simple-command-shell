#ifndef __EXCEPTION_H_
#define __EXCEPTION_H_

#include <exception>
#include <string>

/*************************************************************
This file is to define all the exception classes in the shell.
*************************************************************/

//This class defines the exceptions which may cause the shell to exit.
class EXIT_SHELL : public std::exception {
private:
	bool exitStatus; //The exit status of the shell

public:
	//default constructor(must have a status)
	EXIT_SHELL(bool status): exitStatus(status) {}
	//get the exit status
	bool getStatus() {return exitStatus;}
	//inheritated error output
	const char * what() const throw() {
		return "EXIT SHELL";
	}
};

//This class defines the exceptions which may cause the shell to output an error message
class COMMAND_ERROR : public std::exception {
private:
	std::string errorString; //The error message of the command

public:
	//default constructor(must have a error message)
	COMMAND_ERROR(std::string errorString): errorString(errorString) {}
	//default distructor
	virtual ~COMMAND_ERROR() throw() {}
	//inheritated error output
	const char * what() const throw() {
		return errorString.c_str();
	}
	//error output in C++ String, for some special characters like '"'
	std::string whatInString() const throw() {
		return errorString;
	}
};

#endif
