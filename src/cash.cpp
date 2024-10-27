/**
 * @file cash.cpp
 * @brief cash: Can't Afford a SHell
 * @Author Angine (me@angine.tech)
 * @date   October 27, 2024
 * @version 0.1
 *
 * The main cpp file of cash, a toy shell project.
 */

#include <sys/wait.h>
#include <sys/types.h>
#include <unistd.h>
#include <cstdlib>
#include <cstdio>
#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include "cash.h"

int cash::help(const std::vector<std::string>& args)
{
    std::cout << "cash: Can\'t Afford a SHell" << std::endl
        << "Version 0.1" << std::endl
        << "A toy shell project by Angine." << std::endl
        << "Usage: type the command and press Enter." << std::endl
        << "Built-in commands:" << std::endl;
    for (const auto& command : cash::BuiltinCommands)
    {
        std::cout << "    " << BOLD << MAGENTA << command.name << RESET << ": " << command.description << std::endl;
    }

    return 0;
}

int cash::cd(const std::vector<std::string>& args)
{
    if (args.size() == 1)
    {
        std::cout << "cd: too few arguments!" << std::endl
            << "Usage: cd dest_dir" << std::endl;
    }
    else if (
        args.size() >= 3)
    {
        std::cout << "cd: too many arguments!" << std::endl;
    }
    else
    {
        // Changes the directory
        if (chdir(args[1].c_str()) != 0)
        {
            std::cout << RED << "cd: " << strerror(errno) << RESET << std::endl;
        }
    }
    return 0;
}

int cash::exit(const std::vector<std::string>& args)
{
    std::cout << "cash: Exiting..." << std::endl;
    std::exit(EXIT_SUCCESS);
    return 0;
}

int cash::history(const std::vector<std::string>& args)
{
    for (int i = 0; i < history_commands.size(); ++i)
    {
        std::printf("%3d %s\n", i + 1, history_commands[i].c_str());
    }
    return 0;
}

int cash::greet()
{
    std::cout << "cash: Can\'t Afford a SHell by Angine, version 0.1" << std::endl
        << "type \"help\" for more information." << std::endl;
    return 0;
}

std::vector<std::string> cash::parse(const std::string& input, const char delimiter)
{
    std::vector<std::string> args;
    std::string arg;
    bool quoted = false;

    for (const char ch : input)
    {
        if (ch == '"')
        {
            // Set quoted status when encountering a double quote
            quoted = !quoted;
        }
        else if (ch == delimiter && !quoted)
        {
            // If not within quotes and at a delimiter, finalize the current argument
            if (!arg.empty())
            {
                args.push_back(arg);
                // Clear arg for the next argument
                arg.clear();
            }
        }
        else
        {
            // Otherwise, add character to the current argument
            arg += ch;
        }
    }

    // Add the last argument if it's not empty
    if (!arg.empty())
    {
        args.push_back(arg);
    }

    // If quotation marks do not appear in pairs
    if (quoted)
    {
        // Print error message.
        std::cout << "cash: Bad syntax. Unmatched quotation marks." << std::endl;
        // Clear the args as empty output should be given to a bad input
        args.clear();
    }

    return args;
}

int cash::execute(const std::vector<std::string>& args)
{
    // Empty input
    if (args.empty())
    {
        return 1;
    }

    // Check if in the built-in commands list
    for (const auto& builtin_command : cash::BuiltinCommands)
    {
        if (args[0] == builtin_command.name)
        {
            return builtin_command.func(args);
        }
    }

    // Processing pipes
    std::vector<std::string> command1, command2;
    bool found_pipe = false;

    // Splits the args into to sets of args
    for (const auto& arg : args)
    {
        // Sets flag if there is pipe
        if (arg == "|")
        {
            found_pipe = true;
        }
        // When the pipe has not been met, args goes into the first command
        else if (!found_pipe)
        {
            command1.push_back(arg);
        }
        // Everything after the pipe goes to the second command
        else
        {
            command2.push_back(arg);
        }
    }

    // when there is pipe
    if (found_pipe)
    {
        // Initialize pipe file descriptors
        int pipe_file[2];
        pipe(pipe_file);

        // Forks for the first command
        if (fork() == 0)
        {
            // Redirects standard output to the pipe file 1
            dup2(pipe_file[1], STDOUT_FILENO);
            close(pipe_file[0]);
            close(pipe_file[1]);
            // Spawn the first process
            spawn(command1);
            std::exit(0);
        }
        // Forks for the second command
        if (fork() == 0)
        {
            // Redirects the pipe file 0 to standard input
            dup2(pipe_file[0], STDIN_FILENO);
            close(pipe_file[1]);
            close(pipe_file[0]);
            spawn(command2);
            std::exit(0);
        }
        close(pipe_file[0]);
        close(pipe_file[1]);
        wait(nullptr);
        wait(nullptr);
    }
    else
    {
        spawn(args);
    }
    return 0;
}

int cash::spawn(const std::vector<std::string>& args)
{
    // Convert the vector of arguments to a format execvp can use (array of char pointers)
    std::vector<char*> c_args;
    c_args.reserve(args.size());
    for (const auto& arg : args)
    {
        c_args.push_back(const_cast<char*>(arg.c_str()));
    }
    c_args.push_back(nullptr); // Null-terminate the array

    // Fork a new process
    pid_t pid = fork();
    if (pid == -1)
    {
        // Fork failed
        std::cout << RED << "fork: " << strerror(errno) << RESET << std::endl;
        return -1;
    }
    if (pid == 0)
    {
        // Child process: execute the command
        if (execvp(c_args[0], c_args.data()) == -1)
        {
            // If execvp fails, print error and exit
            std::cout << RED << "execvp: " << strerror(errno) << RESET << std::endl;
            std::exit(EXIT_FAILURE);
        }
    }
    else
    {
        // Parent process: wait for the child to finish
        int status;
        if (waitpid(pid, &status, 0) == -1)
        {
            std::cout << RED << "waitpid: " << strerror(errno) << RESET << std::endl;
            return -1;
        }

        // Return the child's exit status
        if (WIFEXITED(status))
        {
            return WEXITSTATUS(status);
        }
        else
        {
            return -1; // Abnormal termination
        }
    }
    return 0;
}

int cash::loop()
{
    while (true)
    {
        std::string input;

        // Prints the prompt
        std::cout << BOLD << CYAN << "cash> " << RESET;

        if (!std::getline(std::cin, input))
        {
            break;
        }

        // Saves history
        history_commands.push_back(input);
        std::vector<std::string> args = parse(input, ' ');

        execute(args);
    }
    return 0;
}

int main()
{
    cash::greet();
    cash::loop();
    return EXIT_SUCCESS;
}
