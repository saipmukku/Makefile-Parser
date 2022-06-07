#include "fakefile.h"
#include "split.h"
#include "safe-fork.h"
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

/*
  TerpConnect login ID: saipmukk
  University ID Number: 118058573
  Discussion Section: 0102

  I pledge on my honor that I have not given or received
  unauthorized assistance on this assignment/examination.

  Block comment: This program manipulates the 
  information in files that are supposed to 
  contain information in a very similar format 
  to a normal Makefile. It contains functions 
  necessary for any program that would read a
  file, such as testing for whether or not it exists,
  and the age of the file. Then, it contains a very
  important function that reads examples of "fakefiles",
  and inputs information from the fakefile into a 
  dynamically allocated data structure. The program
  then contains various functions that print specific
  rules, actions in a rule, dependencies in a rule, 
  etc. The program also contains a method for doing
  the action that a fakefile rule has, as well as a 
  function that clears all the allocated memory in 
  the dynamically allocated data structure.
*/

/*
  Uses the system function "stat()" to determine
  whether or not the name of the file exists.
*/
int exists(const char filename[]) {

  /*
    Declares a stat structure for the stat()
    function.
  */

  struct stat file;
  errno = 0;

  /*
    Based on certain values returned by stat(), 
    the function determines whether or not the
    file exists.
  */

  if((stat(filename, &file) == -1 && errno == ENOENT)
     || filename == NULL) {

    return 0;

  }

  return 1;

}

/*
  Similar to the previous function, uses the stat()
  function to determine whether or not the first
  existing file is older than the second existing
  file.
*/
int is_older(const char filename1[], const char filename2[]) {

  /*
    Declares two stat structures and pointers to
    those two structures.
  */

  struct stat file1;
  struct stat file2;
  struct stat *file1_p = &file1;
  struct stat *file2_p = &file2;

  stat(filename1, &file1);
  stat(filename2, &file2);

  /*
    Sees if the files exist, and then uses
    information put into the stat structures
    to determine whether or not the first file
    is older than the second file. If it is,
    1 is returned, otherwise 0 is returned.
  */

  if(exists(filename1) && exists(filename2)
     && file1_p->st_mtime < file2_p->st_mtime) {

    return 1;

  } else if(filename1 == NULL 
	    || filename2 == NULL) {

    return 0;

  }

  return 0;

}

/*
  This function reads a file with the name of
  the parameter, and returns a pointer to a
  Fakefile datastructure that contains all of
  the information in the file.
*/
Fakefile *read_fakefile(const char filename[]) {

  /*
    Declares various variables, including a file
    variable, pointer to a new Fakefile data
    structure, character array that stores each
    line as it is read, and an integer that is
    incremented to keep track of what number rule
    was just inputted.
  */

  FILE *file = NULL;
  Fakefile *rf_pointer;
  char current_line[1001];
  int num_rule = 0;

  /*
    If the file does not exist, or the filename 
    is NULL, returns NULL. Otherwise, it opens
    the file.
  */

  if(!(exists(filename)) || filename == NULL) 
    
    return NULL;

  else file = fopen(filename, "r");

  /*
    Allocates memory for the Fakefile data
    structure.
  */
 
  rf_pointer = malloc(sizeof(struct fake));
  rf_pointer->rules = NULL;

  /*
    While loop that loops through every line
    in the file, and inputs it in the necessary
    spot in the datastructure. The while loop
    runs until the fgets() function returns
    NULL, indicating the end of the file.
  */

  while(fgets(current_line, 1001, file)) {

    /*
      Creates a new rule, and allocated memory to it.
      Then sees if the next line is a new line. If not,
      splits the current line and adds it to the data
      structure. It then does the same for the next line.
    */

    Fakefile_Rule *new_rule = 
      malloc(sizeof(struct rule));

    if(current_line[0] != '\n' && current_line != NULL) {

      new_rule->target_and_deps = split(current_line);

      fgets(current_line, 1001, file);
      new_rule->commands = split(current_line);

      fgets(current_line, 1001, file);

      /*
	This if statement adds to the linked list
	of rules, and if the head of the linked
	list is NULL, allocates memory for it.
      */

      if(rf_pointer->rules != NULL) {

	/*
	  If the head of the linked list has already
	  been initialized, a while loop iterates to
	  the end of the linked list, and allocates
	  memory for a new rule at the end of the 
	  linked list.
	*/

	Fakefile_Rule *last_rule = rf_pointer->rules;
      
	while(last_rule->next_rule != NULL) {

	  last_rule = last_rule->next_rule;

	}

	last_rule->next_rule = malloc(sizeof(struct rule));
	last_rule->next_rule = new_rule;
	last_rule->next_rule->num_rule = num_rule;

      } else {

	/*
	  If the head of the linked list is NULL, 
	  memory is allocated for it, and then the
	  new rule is added as the head of the linked
	  list.
	*/

	rf_pointer->rules = new_rule;
	rf_pointer->rules->next_rule = NULL;
	rf_pointer->rules->num_rule = num_rule;

      }

      /* Iterates num_rule variable */

      num_rule++;

    }

  }

  /*
    Sets the number of rules to the current
    number of the num_rule variable,
    indicating 
  */

  rf_pointer->num_of_rules = num_rule;

  fclose(file);

  return rf_pointer;

}

/*
  This function returns the rule number of the
  found target, based on the value of the 
  target_name function.
*/
int lookup_target(Fakefile *const fakefile, 
		  const char target_name[]) {

  /*
    Declares a variable that iterates through the
    while loop.
  */

  Fakefile_Rule *current_rule;

  /*
    If either of the parameters are NULL, returns
    -1.
  */

  if(fakefile == NULL || 
     target_name == NULL) return -1;

  /*
    Sets current rule to the head of the fakefile.
  */

  current_rule = fakefile->rules;

  /*
    While loop loops through the linked list,
    and finds the rule with the target name to
    return it.
  */
  
  while(current_rule != NULL) {

    if(strcmp(current_rule->target_and_deps[0], 
	      target_name) == 0) {

      return current_rule->num_rule;

    }

    current_rule = current_rule->next_rule;

  }

  return -1;

}

/*
  Prints the specific action once the rule with 
  the rule number is found.
*/
void print_action(Fakefile *const fakefile, int rule_num) {

  /*
    Declares a variable that is used to iterate
    the linked list.
  */

  Fakefile_Rule *current_rule;

  if(fakefile == NULL || 
     rule_num > fakefile->num_of_rules) return;

  current_rule = fakefile->rules;

  /*
    Iterates through the linked list and finds
    the specific command that is being looked
    for.
  */

  while(current_rule != NULL) {

    if(current_rule->num_rule == rule_num) {

      int command_counter = 0;

      while(current_rule->commands[command_counter + 1] 
	    != NULL) {

	printf("%s ", 
	       current_rule->commands[command_counter]);
	command_counter++;

      }

      printf("%s\n", 
	     current_rule->commands[command_counter]);

    }
	  
    current_rule = current_rule->next_rule;

  }

}

/*
  Prints the fakefile according to standard
  makefile format.
*/
void print_fakefile(Fakefile *const fakefile) {

  /*
    Declares variables that help the function
    find the necessary components of the
    fakefile to print out.
  */
 
  Fakefile_Rule *current_rule = fakefile->rules;
  int target;
  int action;

  if(current_rule == NULL) return;

  /*
    Iterates through the linked list in the 
    data structure, and prints out the
    necessary components.
  */
    
  while(current_rule != NULL) {

    target = 1;
    action = 0;

    printf("%s: ", current_rule->target_and_deps[0]);

    /*
      Iterates through the double pointer holding
      the target and dependencies and prints out
      everything.
    */

    while(current_rule->target_and_deps[target + 1] 
	  != NULL) {

      printf("%s ", 
	     current_rule->target_and_deps[target]);
      target++;

    }

    printf("%s\n\t", 
	   current_rule->target_and_deps[target]);

    /*
      Iterates through the double pointer holding
      the commands and prints out everything.
    */

    while(current_rule->commands[action + 1] != NULL) {

      printf("%s ", current_rule->commands[action]);
      action++;

    }

    printf("%s\n", current_rule->commands[action]);

    if(current_rule->next_rule != NULL) printf("\n");

    current_rule = current_rule->next_rule;
    
  }

}

/*
  Prints out the number of the dependencies.
*/
int num_dependencies(Fakefile *const fakefile, 
		     int rule_num) {

  Fakefile_Rule *current_rule = fakefile->rules;

  if(fakefile == NULL || 
     rule_num > fakefile->num_of_rules) return -1;

  /*
    Iterates through the linked list and
    increments an integer variable for
    every dependency in the indicated
    rule.
  */

  while(current_rule != NULL) {

    if(current_rule->num_rule == rule_num) {

      int num_dependencies = 0;

      while(current_rule->target_and_deps[num_dependencies] 
	    != NULL) {

	num_dependencies++;

      }

      return num_dependencies - 1;

    }

    current_rule = current_rule->next_rule;

  }

  return -1;

}

/*
  Returns the string value of the dependency
  after finding the specific rule and
  dependency number.
*/
char *get_dependency(Fakefile *const fakefile, 
		     int rule_num, int dependency_num) {

  Fakefile_Rule *current_rule = fakefile->rules;

  if(fakefile == NULL || 
     rule_num > fakefile->num_of_rules 
     || dependency_num < 0) return NULL;

  /*
    Iterates through the linked list and returns
    the proper dependency after the correct rule
    is found.
  */

  while(current_rule != NULL) {

    if(current_rule->num_rule == rule_num) {

      return 
	current_rule->target_and_deps[dependency_num + 1];

    }

    current_rule = current_rule->next_rule;

  }

  return NULL;

}

/*
  Does the action using an exec function.
*/
int do_action(Fakefile *const fakefile, int rule_num) {

  Fakefile_Rule *found_rule;
  pid_t child_pid;
  int exit_status;
  
  if(fakefile == NULL || 
     rule_num > fakefile->num_of_rules) return -1;

  found_rule = fakefile->rules;

  while(found_rule != NULL && 
	found_rule->num_rule != rule_num) {

    found_rule = found_rule->next_rule;

  }

  if(found_rule == NULL) return -1;

  /* Forks child */

  child_pid = safe_fork();

  /*
    Parent process waits for child to exit,
    then returns the exit status of the 
    child for the make_target() function.
  */

  if(child_pid > 0) {

    wait(&exit_status);

    if(WIFEXITED(exit_status)
       && !(WEXITSTATUS(exit_status))) {

      return exit_status;

    }

    /*
      Child process exec's a program.
    */

  } else if(child_pid == 0) {

    if(found_rule != NULL) 

      execvp(found_rule->commands[0], 
	     found_rule->commands);

  }

  return -1;

}

/*
  Recursive function clears all the rules from
  the data structure.
*/
void clear_rules(Fakefile_Rule *rule);

void clear_rules(Fakefile_Rule *rule) {

  int i;

  /* Recursive call */

  if(rule->next_rule != NULL) {

    clear_rules(rule->next_rule);

  }

  i = 0;

  /*
    While loop loops through target and
    dependencies and clears all of the 
    individual char pointer components.
  */

  while(rule->target_and_deps[i] != NULL) {

    free(rule->target_and_deps[i]);
    i++;

  }

  free(rule->target_and_deps);
  i = 0;

  /*
    While loop loops through commands
    and clears all of the individual
    char pointer components.
  */

  while(rule->commands[i] != NULL) {

    free(rule->commands[i]);
    i++;

  }

  /*
    Frees the commands and then the rule
    itself.
  */
  
  free(rule->commands);
  free(rule);

}

void clear_fakefile(Fakefile **const fakefile) {

  /*
    Declares a variable to access the 
    fakefile from the double pointer.
  */

  Fakefile *fake = *fakefile;

  if(fakefile == NULL || *fakefile == NULL) return;

  /*
    Calls recursive function, then frees
    the fakefile and makes it NULL,
  */

  clear_rules(fake->rules);
  free(*fakefile);
  *fakefile = NULL;
}
