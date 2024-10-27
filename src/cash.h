/**
 * @file cash.h
 * @brief header for cash
 * @Author Angine (me@angine.tech)
 * @date   October 27, 2024
 *
 * Contains declarations.
 */


#ifndef CASH_H
#define CASH_H

// Colors for terminal output
#define RESET   "\033[0m"
#define RED     "\033[31m"      /* Red */
#define MAGENTA "\033[35m"      /* Magenta */
#define CYAN    "\033[36m"      /* Cyan */
#define BOLD    "\033[1m"      /* Bold */

namespace cash
{
    /**
    * @brief Prints greeting message.
    *
    * @return an integer, exit status.
    */
    int greet();

    /**
    * @brief Continuously print prompt and ask for input, preventing the program from exiting.
    *
    * @return an integer, exit status.
    */
    int loop();

    /**
    * @brief Parse input commands.
    *
    * @param input User input.
    * @param delimiter Delimiter used for splitting input into arguments.
    * @return arguments, in a vector of strings.
    */
    std::vector<std::string> parse(const std::string& input, char delimiter);

    /**
    * @brief Stores history commands.
    */
    static std::vector<std::string> history_commands;

    /**
    * @brief Prints help message.
    *
    * @param args arguments.
    * @return an integer, exit status.
    */
    int help(const std::vector<std::string>& args);

    /**
    * @brief Change directory.
    *
    * @param args arguments.
    * @return an integer, exit status.
    */
    int cd(const std::vector<std::string>& args);

    /**
    * @brief Exits the program.
    *
    * @param args arguments.
    * @return an integer, exit status.
    */
    int exit(const std::vector<std::string>& args);

    /**
    * @brief Prints history commands.
    *
    * @param args arguments.
    * @return an integer, exit status.
    */
    int history(const std::vector<std::string>& args);

    /**
    * @brief Spawns new process.
    *
    * @param args arguments.
    * @return an integer, exit status.
    */
    int spawn(const std::vector<std::string>& args);

    /**
    * @brief Executes the command.
    *
    * @param args arguments.
    * @return an integer, exit status.
    */
    int execute(const std::vector<std::string>& args);

    /**
    * @brief Built-in Command.
    */
    struct BuiltinCommand
    {
        std::string name; //!< Name of the built-in command.
        int (*func)(const std::vector<std::string>& args); //!< Pointer to the built-in function.
        std::string description; //!< Description of the built-in command.
    };

    static const BuiltinCommand BuiltinCommands[] = {
        BuiltinCommand{"help", help, "shows this message."},
        BuiltinCommand{"cd", cd, "changes directory."},
        BuiltinCommand{"exit", exit, "exits the shell program."},
        BuiltinCommand{"history", history, "show history commands"}
    }; //!< Array for built-in commands.
}

#endif //CASH_H
