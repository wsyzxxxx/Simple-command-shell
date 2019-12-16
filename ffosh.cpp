#include <cstdlib>
#include <iostream>
#include "Interpreter.h"
#include "exceptions.h"

/*************************************************************
This file is the entrance of the ECE551 shell program.
*************************************************************/


/* Function:		main
 * @Param:			void
 * @Return:			(int) EXIT_SUCCESS or EXIT_FAILURE, 
 *							which identifies the exit status of the current program.
 * @Exception:  This function is 'No Throw' guaranted and treats 
 *							all the catched exceptions as failed exit.
 *
 * Description: This is the entrance of the shell program.
 *              It initializes and holds an Interpreter object, 
 *							which deals with all the interaction with the user.
 */
int main(void) {
	//The main object to deal with the interactions.
	Interpreter interpreter;

	// User interactions are handled in the interpreter object.
	try {
		interpreter.startShell();
	} catch (EXIT_SHELL & e) { // Normal exit the shell or error happened in exection of the child process.
		if (e.getStatus() == false) {
			return EXIT_FAILURE;
		}
	} catch (std::exception & e) { // In case of special exception happened.
		std::cout << "ERROR: main" << e.what() << std::endl;
		return EXIT_FAILURE;
	}

	//shell exit
	return EXIT_SUCCESS;
}
