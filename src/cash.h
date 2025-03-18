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

#include <map>
#include <termios.h>  // 用于终端控制

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
    * @brief Stores alias mappings.
    */
    static std::map<std::string, std::string> aliases;

    /**
    * @brief Background job structure.
    */
    struct Job {
        pid_t pid;         //!< Process ID of the job
        pid_t pgid;        //!< Process group ID
        std::string cmd;   //!< Command string
        enum Status {
            RUNNING,
            STOPPED,
            DONE
        } status;          //!< Job status
    };

    /**
    * @brief Stores background jobs.
    */
    static std::vector<Job> background_jobs;

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
    int spawn(const std::vector<std::string>& args, int in_fd = STDIN_FILENO, int out_fd = STDOUT_FILENO);

    /**
    * @brief Executes the command.
    *
    * @param args arguments.
    * @return an integer, exit status.
    */
    int execute(const std::vector<std::string>& args);

    /**
    * @brief Echoes text to console.
    *
    * @param args arguments.
    * @return an integer, exit status.
    */
    int echo(const std::vector<std::string>& args);

    /**
    * @brief Clears the terminal screen.
    *
    * @param args arguments.
    * @return an integer, exit status.
    */
    int clear(const std::vector<std::string>& args);

    /**
    * @brief Creates, displays or removes aliases.
    *
    * @param args arguments.
    * @return an integer, exit status.
    */
    int alias(const std::vector<std::string>& args);

    /**
    * @brief Display status of background jobs.
    *
    * @param args arguments.
    * @return an integer, exit status.
    */
    int jobs(const std::vector<std::string>& args);

    /**
    * @brief Sets or displays environment variables.
    *
    * @param args arguments.
    * @return an integer, exit status.
    */
    int export_var(const std::vector<std::string>& args);

    /**
    * @brief Brings a background job to the foreground.
    *
    * @param args arguments.
    * @return an integer, exit status.
    */
    int fg(const std::vector<std::string>& args);

    /**
    * @brief Continues a stopped job in the background.
    *
    * @param args arguments.
    * @return an integer, exit status.
    */
    int bg(const std::vector<std::string>& args);

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
        BuiltinCommand{"history", history, "shows history commands"},
        BuiltinCommand{"echo", echo, "displays text"},
        BuiltinCommand{"clear", clear, "clears the terminal screen"},
        BuiltinCommand{"alias", alias, "creates, lists or removes aliases"},
        BuiltinCommand{"jobs", jobs, "lists background jobs"},
        BuiltinCommand{"export", export_var, "sets environment variables"},
        BuiltinCommand{"fg", fg, "brings job to foreground"},
        BuiltinCommand{"bg", bg, "continues job in background"}
    }; //!< Array for built-in commands.

    /**
    * @brief 保存原始终端属性以便恢复
    */
    static struct termios orig_termios;
    
    /**
    * @brief 当前编辑的命令行
    */
    static std::string current_line;
    
    /**
    * @brief 当前光标位置
    */
    static int cursor_position;
    
    /**
    * @brief 当前历史命令索引
    */
    static int history_index;
    
    /**
    * @brief 设置终端为原始模式
    *
    * @return 成功返回0，失败返回-1
    */
    int enable_raw_mode();
    
    /**
    * @brief 恢复终端到原始状态
    */
    void disable_raw_mode();
    
    /**
    * @brief 读取一个字符
    *
    * @return 读取的字符
    */
    int read_key();
    
    /**
    * @brief 处理特殊键（如箭头键）
    *
    * @param key 读取的键值
    * @return 继续读取返回true，命令完成返回false
    */
    bool process_keypress(int key);
    
    /**
    * @brief 重新绘制当前命令行
    */
    void refresh_line();
    
    /**
    * @brief 读取一行输入，支持命令历史和行编辑
    *
    * @return 输入的命令字符串
    */
    std::string read_line();
}

#endif //CASH_H
