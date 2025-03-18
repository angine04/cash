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
#include <cstring>  // 用于strlen函数
#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include <fstream>   // 用于检查进程状态文件
#include <signal.h>   // 用于信号处理
#include <termios.h>  // 用于终端控制
#include "cash.h"

#ifdef _MSC_VER
#include <windows.h>  // Windows环境变量函数
#endif

// 键值定义
enum Key {
    KEY_NULL = 0,
    CTRL_A = 1,
    CTRL_B = 2,
    CTRL_C = 3,
    CTRL_D = 4,
    CTRL_E = 5,
    CTRL_F = 6,
    CTRL_H = 8,
    TAB = 9,
    CTRL_K = 11,
    CTRL_L = 12,
    ENTER = 13,
    CTRL_N = 14,
    CTRL_P = 16,
    CTRL_T = 20,
    CTRL_U = 21,
    CTRL_W = 23,
    ESC = 27,
    BACKSPACE = 127,
    // 以下是多字节序列的第一个字节后的代码
    ARROW_UP = 1000,
    ARROW_DOWN,
    ARROW_RIGHT,
    ARROW_LEFT,
    HOME_KEY,
    END_KEY,
    DEL_KEY
};

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
    else if (args.size() >= 3)
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
    bool single_quoted = false;  // 添加单引号状态跟踪

    for (size_t i = 0; i < input.length(); i++)
    {
        char ch = input[i];
        
        // 处理引号
        if (ch == '"' && !single_quoted)
        {
            quoted = !quoted;
            continue;  // 跳过引号本身
        }
        else if (ch == '\'' && !quoted)
        {
            single_quoted = !single_quoted;
            continue;  // 跳过引号本身
        }
        // 如果在引号内或遇到分隔符
        else if ((ch == delimiter) && !quoted && !single_quoted)
        {
            // 如果当前参数不为空，加入到参数列表
            if (!arg.empty())
            {
                args.push_back(arg);
                arg.clear();
            }
        }
        else
        {
            // 其他情况，添加字符到当前参数
            arg += ch;
        }
    }

    // 添加最后一个参数
    if (!arg.empty())
    {
        args.push_back(arg);
    }

    // 检查引号是否匹配
    if (quoted || single_quoted)
    {
        std::cout << "cash: Bad syntax. Unmatched quotation marks." << std::endl;
        args.clear();
    }

    return args;
}

int cash::spawn(const std::vector<std::string>& args, int in_fd, int out_fd)
{
    // Convert the vector of arguments to a format execvp can use
    std::vector<char*> c_args;
    c_args.reserve(args.size());
    for (const auto& arg : args)
    {
        c_args.push_back(const_cast<char*>(arg.c_str()));
    }
    c_args.push_back(nullptr);

    pid_t pid = fork();
    if (pid == -1)
    {
        std::cout << RED << "fork: " << strerror(errno) << RESET << std::endl;
        return -1;
    }
    
    if (pid == 0)
    {
        // 创建新的进程组（子进程）
        setpgid(0, 0);
        
        // 重定向输入输出（如果需要）
        if (in_fd != STDIN_FILENO) {
            dup2(in_fd, STDIN_FILENO);
        }
        if (out_fd != STDOUT_FILENO) {
            dup2(out_fd, STDOUT_FILENO);
        }
        
        // 关闭所有管道文件描述符
        if (in_fd != STDIN_FILENO) close(in_fd);
        if (out_fd != STDOUT_FILENO) close(out_fd);

        // 执行命令
        if (execvp(c_args[0], c_args.data()) == -1)
        {
            std::cout << RED << "execvp: " << strerror(errno) << RESET << std::endl;
            std::exit(EXIT_FAILURE);
        }
    }
    else
    {
        // 确保子进程在不同的进程组（父进程）
        setpgid(pid, pid);
    }

    return pid;
}

int cash::alias(const std::vector<std::string>& args)
{
    // 不带参数时显示所有别名
    if (args.size() == 1)
    {
        if (aliases.empty())
        {
            std::cout << "No aliases defined" << std::endl;
        }
        else
        {
            for (const auto& alias_pair : aliases)
            {
                std::cout << "alias " << alias_pair.first << "='" << alias_pair.second << "'" << std::endl;
            }
        }
        return 0;
    }

    // 处理带等号的别名设置，例如 alias ll='ls -l'
    // 首先检查原始输入中是否包含等号
    std::string original_input;
    for (size_t i = 1; i < args.size(); ++i) {
        original_input += args[i];
        if (i < args.size() - 1) {
            original_input += " ";
        }
    }

    // 检查是否是取消别名操作 (alias -r name)
    if (args.size() >= 3 && args[1] == "-r")
    {
        std::string name = args[2];
        if (aliases.find(name) != aliases.end())
        {
            aliases.erase(name);
            std::cout << "Alias '" << name << "' removed" << std::endl;
        }
        else
        {
            std::cout << "No such alias: " << name << std::endl;
        }
        return 0;
    }
    
    // 查找赋值符号=
    size_t equal_pos = original_input.find('=');
    if (equal_pos != std::string::npos && equal_pos > 0)
    {
        std::string name = original_input.substr(0, equal_pos);
        std::string value = original_input.substr(equal_pos + 1);
        
        // 如果值用引号包围，去掉引号
        if ((value.front() == '\'' && value.back() == '\'') ||
            (value.front() == '"' && value.back() == '"'))
        {
            value = value.substr(1, value.length() - 2);
        }
        
        aliases[name] = value;
    }
    else
    {
        std::cout << "Invalid alias syntax: " << original_input << std::endl;
        std::cout << "Usage: alias name=value" << std::endl;
    }
    
    return 0;
}

int cash::jobs(const std::vector<std::string>& args)
{
    // 检查每个后台任务的状态并更新
    for (auto& job : background_jobs)
    {
        if (job.status == Job::RUNNING)
        {
            int status;
            pid_t result = waitpid(job.pid, &status, WNOHANG);
            
            if (result == job.pid)
            {
                // 进程已完成
                job.status = Job::DONE;
            }
            else if (result == -1)
            {
                // 错误
                job.status = Job::DONE;
            }
            else if (result == 0)
            {
                // 进程仍在运行，检查是否已停止
                if (kill(job.pid, 0) == 0)
                {
                    // 进程存在，检查是否已暂停
                    if (kill(job.pid, 0) == 0)
                    {
                        // 检查/proc/[pid]/stat文件以确定进程状态
                        std::string procfile = "/proc/" + std::to_string(job.pid) + "/stat";
                        std::ifstream statfile(procfile);
                        if (statfile.is_open())
                        {
                            std::string line;
                            std::getline(statfile, line);
                            std::istringstream iss(line);
                            std::string status_str;
                            
                            // 跳过前两个字段
                            iss >> status_str >> status_str;
                            iss >> status_str; // 第三个字段是状态
                            
                            if (status_str == "T")
                            {
                                job.status = Job::STOPPED;
                            }
                        }
                    }
                }
            }
        }
    }
    
    // 显示后台任务列表
    bool has_jobs = false;
    for (size_t i = 0; i < background_jobs.size(); ++i)
    {
        const auto& job = background_jobs[i];
        
        // 跳过已完成的作业
        if (job.status == Job::DONE)
            continue;
            
        has_jobs = true;
        
        if (job.status == Job::RUNNING)
        {
            std::cout << "[" << i + 1 << "] Running    " << job.pid << " " << job.cmd << std::endl;
        }
        else if (job.status == Job::STOPPED)
        {
            std::cout << "[" << i + 1 << "] Stopped    " << job.pid << " " << job.cmd << std::endl;
        }
    }
    
    if (!has_jobs)
    {
        std::cout << "No active jobs" << std::endl;
    }
    
    return 0;
}

int cash::execute(const std::vector<std::string>& args)
{
    // Empty input
    if (args.empty())
    {
        return 1;
    }
    
    // 检查命令是否是别名
    std::vector<std::string> expanded_args = args;
    if (aliases.find(args[0]) != aliases.end())
    {
        // 解析别名值
        std::string expanded_cmd = aliases[args[0]];
        std::vector<std::string> alias_args = parse(expanded_cmd, ' ');
        
        // 替换第一个参数，保留原始参数中的其他参数
        expanded_args[0] = alias_args[0];
        alias_args.erase(alias_args.begin());
        
        // 插入别名的其余参数
        expanded_args.insert(expanded_args.begin() + 1, alias_args.begin(), alias_args.end());
    }
    
    // 在此处展开环境变量（应用到所有命令，不仅是echo）
    for (auto& arg : expanded_args)
    {
        if (!arg.empty() && arg[0] == '$' && arg.length() > 1)
        {
            std::string var_name = arg.substr(1);
            const char* value = getenv(var_name.c_str());
            if (value != nullptr)
            {
                arg = value;
            }
            else
            {
                arg = ""; // 如果变量不存在，设为空字符串
            }
        }
    }

    // Check if in the built-in commands list
    for (const auto& builtin_command : cash::BuiltinCommands)
    {
        if (expanded_args[0] == builtin_command.name)
        {
            return builtin_command.func(expanded_args);
        }
    }

    // 检查是否要在后台运行
    bool run_in_background = false;
    if (!expanded_args.empty() && expanded_args.back() == "&")
    {
        run_in_background = true;
        expanded_args.pop_back();  // 移除&符号
    }

    // 设置SIGINT处理器，确保shell不会被Ctrl+C杀死
    struct sigaction sa_int, old_sa_int;
    sa_int.sa_handler = SIG_IGN;  // 忽略SIGINT
    sigemptyset(&sa_int.sa_mask);
    sa_int.sa_flags = 0;
    sigaction(SIGINT, &sa_int, &old_sa_int);

    // Processing pipes
    std::vector<std::string> command1, command2;
    bool found_pipe = false;

    // Splits the args into to sets of args
    for (const auto& arg : expanded_args)
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
        int pipe_file[2];
        if (pipe(pipe_file) == -1) {
            std::cout << RED << "pipe: " << strerror(errno) << RESET << std::endl;
            sigaction(SIGINT, &old_sa_int, NULL);  // 恢复SIGINT处理
            return -1;
        }

        // 执行第一个命令
        pid_t pid1 = spawn(command1, STDIN_FILENO, pipe_file[1]);
        if (pid1 == -1) {
            close(pipe_file[0]);
            close(pipe_file[1]);
            sigaction(SIGINT, &old_sa_int, NULL);  // 恢复SIGINT处理
            return -1;
        }
        close(pipe_file[1]);  // 立即关闭写端

        // 执行第二个命令
        pid_t pid2 = spawn(command2, pipe_file[0], STDOUT_FILENO);
        if (pid2 == -1) {
            close(pipe_file[0]);
            sigaction(SIGINT, &old_sa_int, NULL);  // 恢复SIGINT处理
            return -1;
        }
        close(pipe_file[0]);  // 立即关闭读端

        // 等待两个子进程完成
        int status1, status2;
        waitpid(pid1, &status1, 0);
        waitpid(pid2, &status2, 0);

        // 恢复SIGINT处理
        sigaction(SIGINT, &old_sa_int, NULL);

        if (!WIFEXITED(status1) || !WIFEXITED(status2)) {
            return -1;
        }
        return WEXITSTATUS(status2);
    }
    else
    {
        // 对于普通命令，使用默认的标准输入输出
        pid_t pid = spawn(expanded_args);
        
        // 如果是后台运行，则添加到任务列表
        if (run_in_background)
        {
            // 构造命令字符串
            std::string cmd_str;
            for (const auto& arg : expanded_args)
            {
                cmd_str += arg + " ";
            }
            
            // 添加到后台任务列表
            Job job;
            job.pid = pid;
            job.pgid = pid;  // 进程ID与进程组ID相同
            job.cmd = cmd_str;
            job.status = Job::RUNNING;
            background_jobs.push_back(job);
            
            std::cout << "[" << background_jobs.size() << "] " << pid << std::endl;
            
            // 恢复SIGINT处理
            sigaction(SIGINT, &old_sa_int, NULL);
            return 0;
        }
        
        // 否则等待前台命令完成
        int status;
        waitpid(pid, &status, WUNTRACED);  // 添加WUNTRACED标志检测子进程是否被暂停
        
        // 恢复SIGINT处理
        sigaction(SIGINT, &old_sa_int, NULL);
        
        // 检查进程是否被暂停
        if (WIFSTOPPED(status)) {
            // 构造命令字符串
            std::string cmd_str;
            for (const auto& arg : expanded_args)
            {
                cmd_str += arg + " ";
            }
            
            // 如果子进程被暂停，将其添加到后台任务
            Job job;
            job.pid = pid;
            job.pgid = pid;
            job.cmd = cmd_str;
            job.status = Job::STOPPED;
            background_jobs.push_back(job);
            
            std::cout << "[" << background_jobs.size() << "] Stopped: " << cmd_str << std::endl;
        }
        else if (WIFEXITED(status)) {
            return WEXITSTATUS(status);
        }
        else {
            return -1;
        }
    }
    return 0;
}

// 设置终端为原始模式，以便可以按字符处理输入
int cash::enable_raw_mode()
{
    if (tcgetattr(STDIN_FILENO, &orig_termios) == -1)
        return -1;
        
    struct termios raw = orig_termios;
    
    // 设置输入标志: 禁用ECHO和规范模式，但保留信号处理
    raw.c_lflag &= ~(ECHO | ICANON | IEXTEN);
    // 保留ISIG，允许Ctrl+C产生SIGINT信号
    
    // 禁用 Ctrl-S, Ctrl-Q, Ctrl-V
    raw.c_iflag &= ~(IXON | ICRNL);
    
    // 禁用输出处理
    raw.c_oflag &= ~(OPOST);
    
    // 设置字符大小为8位
    raw.c_cflag |= (CS8);
    
    // 设置读取超时
    raw.c_cc[VMIN] = 1;  // 最少读取1个字符
    raw.c_cc[VTIME] = 0; // 不设置超时，阻塞读取
    
    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw) == -1)
        return -1;
        
    return 0;
}

// 恢复终端到原始状态
void cash::disable_raw_mode()
{
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios);
}

// 读取一个按键
int cash::read_key()
{
    char c;
    int nread = read(STDIN_FILENO, &c, 1);
    if (nread <= 0) 
        return KEY_NULL;
        
    // 处理转义序列
    if (c == ESC) {
        char seq[3];
        
        if (read(STDIN_FILENO, &seq[0], 1) != 1) return ESC;
        if (read(STDIN_FILENO, &seq[1], 1) != 1) return ESC;
        
        if (seq[0] == '[') {
            switch (seq[1]) {
                case 'A': return ARROW_UP;
                case 'B': return ARROW_DOWN;
                case 'C': return ARROW_RIGHT;
                case 'D': return ARROW_LEFT;
                case 'H': return HOME_KEY;
                case 'F': return END_KEY;
                case '3': 
                    if (read(STDIN_FILENO, &seq[2], 1) != 1) return ESC;
                    if (seq[2] == '~') return DEL_KEY;
                    break;
            }
        }
        return ESC;
    }
    
    return c;
}

// 刷新显示行
void cash::refresh_line()
{
    // 清除当前行
    printf("\r\033[K"); // 从光标位置到行末清除
    
    // 打印提示符
    printf("%s%s%s", BOLD, CYAN, "cash> ");
    printf("%s", RESET);
    
    // 打印当前行内容
    printf("%s", current_line.c_str());
    
    // 将光标移动到当前位置
    printf("\r\033[%dC", 6 + cursor_position); // 6是 "cash> " 的长度
    
    fflush(stdout);
}

// 处理按键
bool cash::process_keypress(int key)
{
    switch (key) {
        case ENTER:
            return false; // 结束输入
        
        case CTRL_C:
            printf("^C\n");
            current_line.clear();
            cursor_position = 0;
            refresh_line();  // 刷新显示空命令行
            return true;    // 继续输入，不退出，只取消当前命令
            
        case CTRL_D:  // EOF
            if (current_line.empty()) {  // 只有在命令行为空时才退出
                printf("exit\n");
                cash::disable_raw_mode();
                std::exit(EXIT_SUCCESS);
            }
            break;
            
        case BACKSPACE:
        case CTRL_H:
            if (cursor_position > 0) {
                current_line.erase(cursor_position - 1, 1);
                cursor_position--;
            }
            break;
            
        case DEL_KEY:
            if (cursor_position < current_line.length()) {
                current_line.erase(cursor_position, 1);
            }
            break;
            
        case ARROW_LEFT:
            if (cursor_position > 0) {
                cursor_position--;
            }
            break;
            
        case ARROW_RIGHT:
            if (cursor_position < current_line.length()) {
                cursor_position++;
            }
            break;
            
        case ARROW_UP:
            if (!history_commands.empty() && history_index > 0) {
                history_index--;
                current_line = history_commands[history_index];
                cursor_position = current_line.length();
            }
            break;
            
        case ARROW_DOWN:
            if (!history_commands.empty() && history_index < history_commands.size() - 1) {
                history_index++;
                current_line = history_commands[history_index];
                cursor_position = current_line.length();
            } else if (history_index == history_commands.size() - 1) {
                // 如果已经到达最新的历史命令，下一个是空行
                history_index++;
                current_line.clear();
                cursor_position = 0;
            }
            break;
            
        case TAB:
            // 简单的命令补全功能
            {
                if (current_line.empty()) break;
                
                // 获取要补全的部分（当前光标前的单词）
                std::string to_complete;
                int word_start = cursor_position;
                while (word_start > 0 && current_line[word_start - 1] != ' ') {
                    word_start--;
                }
                to_complete = current_line.substr(word_start, cursor_position - word_start);
                
                if (to_complete.empty()) break;
                
                // 构建候选列表（从内置命令中查找）
                std::vector<std::string> candidates;
                
                // 首先检查内置命令
                for (const auto& cmd : BuiltinCommands) {
                    if (cmd.name.find(to_complete) == 0) {
                        candidates.push_back(cmd.name);
                    }
                }
                
                // 然后检查别名
                for (const auto& alias_pair : aliases) {
                    if (alias_pair.first.find(to_complete) == 0) {
                        candidates.push_back(alias_pair.first);
                    }
                }
                
                // 如果只有一个匹配项，直接补全
                if (candidates.size() == 1) {
                    std::string completion = candidates[0].substr(to_complete.length());
                    current_line.insert(cursor_position, completion);
                    cursor_position += completion.length();
                }
                // 如果有多个匹配项，显示所有可能性
                else if (candidates.size() > 1) {
                    printf("\n");
                    for (const auto& candidate : candidates) {
                        printf("%s%s%s  ", BOLD, MAGENTA, candidate.c_str());
                        printf("%s", RESET);
                    }
                    printf("\n");
                    refresh_line();
                }
            }
            break;
            
        case HOME_KEY:
            cursor_position = 0;
            break;
            
        case END_KEY:
            cursor_position = current_line.length();
            break;
            
        case CTRL_L: // 清屏
            system("clear");
            refresh_line();
            break;
            
        default:
            if (key >= 32 && key < 127) { // 可打印字符
                current_line.insert(cursor_position, 1, (char)key);
                cursor_position++;
            }
            break;
    }
    
    return true; // 继续输入
}

// 读取一行输入
std::string cash::read_line()
{
    // 初始化行编辑状态
    current_line.clear();
    cursor_position = 0;
    history_index = history_commands.size();
    
    // 进入原始模式
    if (enable_raw_mode() == -1) {
        std::cerr << "Error: Unable to set terminal to raw mode" << std::endl;
        return "";
    }
    
    // 打印提示符
    refresh_line();
    
    // 设置Ctrl+C处理函数
    struct sigaction sa;
    struct sigaction old_sa;
    sa.sa_handler = [](int sig) {
        // 清空当前行并重新显示提示符
        write(STDOUT_FILENO, "\n", 1);
        cash::current_line.clear();
        cash::cursor_position = 0;
        cash::refresh_line();
    };
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sigaction(SIGINT, &sa, &old_sa);
    
    // 读取输入
    bool reading = true;
    while (reading) {
        int key = read_key();
        reading = process_keypress(key);
        if (reading) {
            refresh_line();
        }
    }
    
    // 恢复原来的信号处理
    sigaction(SIGINT, &old_sa, NULL);
    
    // 恢复终端
    disable_raw_mode();
    printf("\n"); // 命令结束后换行
    
    return current_line;
}

// 修改loop函数使用新的行输入方法
int cash::loop()
{
    while (true)
    {
        std::string input = read_line();

        // 空输入则继续
        if (input.empty()) {
            continue;
        }

        // 保存到历史记录
        history_commands.push_back(input);
        
        // 特殊处理alias命令
        if (input.substr(0, 6) == "alias " && input.find('=') != std::string::npos) {
            // 将alias命令拆分为两部分：alias和后面的部分
            std::vector<std::string> alias_args;
            alias_args.push_back("alias");
            
            // 获取等号前后的部分
            std::string rest = input.substr(6); // 去掉"alias "
            alias_args.push_back(rest);
            
            execute(alias_args);
        }
        else {
            // 正常解析命令
            std::vector<std::string> args = parse(input, ' ');
            execute(args);
        }
    }
    return 0;
}

int cash::echo(const std::vector<std::string>& args)
{
    for (size_t i = 1; i < args.size(); ++i)
    {
        std::cout << args[i];
        if (i < args.size() - 1)
        {
            std::cout << " ";
        }
    }
    std::cout << std::endl;
    return 0;
}

int cash::clear(const std::vector<std::string>& args)
{
    // ANSI转义序列用于清屏
    std::cout << "\033[2J\033[1;1H";
    return 0;
}

int cash::export_var(const std::vector<std::string>& args)
{
    // 无参数时显示当前环境变量
    if (args.size() == 1)
    {
        // 使用系统命令来显示环境变量
        return system("env");
    }
    
    // 处理变量设置
    for (size_t i = 1; i < args.size(); ++i)
    {
        std::string var_def = args[i];
        
        // 查找赋值符号=
        size_t equal_pos = var_def.find('=');
        if (equal_pos != std::string::npos && equal_pos > 0)
        {
            std::string name = var_def.substr(0, equal_pos);
            std::string value = var_def.substr(equal_pos + 1);
            
            // 如果值用引号包围，去掉引号
            if ((value.front() == '\'' && value.back() == '\'') ||
                (value.front() == '"' && value.back() == '"'))
            {
                value = value.substr(1, value.length() - 2);
            }
            
            // 设置环境变量
            if (setenv(name.c_str(), value.c_str(), 1) != 0)
            {
                std::cout << RED << "export: " << strerror(errno) << RESET << std::endl;
                return 1;
            }
        }
        else
        {
            std::cout << "Invalid export syntax: " << var_def << std::endl;
            std::cout << "Usage: export NAME=VALUE" << std::endl;
            return 1;
        }
    }
    
    return 0;
}

// 用于信号处理的全局函数
void handle_signal(int sig)
{
    // 对不同信号进行不同处理
    switch (sig) {
        case SIGINT:   // Ctrl+C
            // 不做任何处理，让shell自己处理Ctrl+C
            break;
        case SIGTSTP:  // Ctrl+Z
            // 不做任何处理，让当前前台进程自己处理Ctrl+Z
            break;
        case SIGTERM:  // 终止信号
        case SIGQUIT:  // Ctrl+\
        default:
            // 其他信号恢复终端并退出
            cash::disable_raw_mode();
            exit(1);
            break;
    }
}

int cash::fg(const std::vector<std::string>& args)
{
    int job_id = 1;  // 默认是1号作业
    
    // 解析作业号参数
    if (args.size() > 1) {
        if (args[1][0] == '%') {
            try {
                job_id = std::stoi(args[1].substr(1));
            } catch (const std::exception& e) {
                std::cout << "fg: " << args[1] << ": no such job" << std::endl;
                return 1;
            }
        } else {
            try {
                job_id = std::stoi(args[1]);
            } catch (const std::exception& e) {
                std::cout << "fg: " << args[1] << ": no such job" << std::endl;
                return 1;
            }
        }
    }
    
    // 检查作业ID是否有效
    if (job_id <= 0 || job_id > background_jobs.size()) {
        std::cout << "fg: no such job" << std::endl;
        return 1;
    }
    
    Job& job = background_jobs[job_id-1];
    if (job.status == Job::DONE) {
        std::cout << "fg: job has terminated" << std::endl;
        return 1;
    }
    
    std::cout << job.cmd << std::endl;
    
    // 保存当前终端进程组
    pid_t shell_pgid = tcgetpgrp(STDIN_FILENO);
    
    // 将作业进程组放到前台
    if (tcsetpgrp(STDIN_FILENO, job.pgid) == -1) {
        std::cout << RED << "tcsetpgrp: " << strerror(errno) << RESET << std::endl;
        return 1;
    }
    
    // 如果作业已暂停，发送继续信号
    if (job.status == Job::STOPPED) {
        if (kill(-job.pgid, SIGCONT) < 0) {
            std::cout << RED << "kill: " << strerror(errno) << RESET << std::endl;
            return 1;
        }
    }
    
    // 等待作业完成或暂停
    int status;
    if (waitpid(job.pid, &status, WUNTRACED) == -1) {
        std::cout << RED << "waitpid: " << strerror(errno) << RESET << std::endl;
        return 1;
    }
    
    // 更新作业状态
    if (WIFSTOPPED(status)) {
        job.status = Job::STOPPED;
        std::cout << std::endl << "Stopped: " << job.cmd << std::endl;
    } 
    else if (WIFEXITED(status) || WIFSIGNALED(status)) {
        job.status = Job::DONE;
    }
    
    // 将shell进程组放回前台
    if (tcsetpgrp(STDIN_FILENO, shell_pgid) == -1) {
        std::cout << RED << "tcsetpgrp: " << strerror(errno) << RESET << std::endl;
        return 1;
    }
    
    return 0;
}

int cash::bg(const std::vector<std::string>& args)
{
    int job_id = 0;
    
    // 解析作业号参数
    if (args.size() > 1) {
        if (args[1][0] == '%') {
            try {
                job_id = std::stoi(args[1].substr(1));
            } catch (const std::exception& e) {
                std::cout << "bg: " << args[1] << ": no such job" << std::endl;
                return 1;
            }
        } else {
            try {
                job_id = std::stoi(args[1]);
            } catch (const std::exception& e) {
                std::cout << "bg: " << args[1] << ": no such job" << std::endl;
                return 1;
            }
        }
    } else {
        // 如果未指定作业，查找最近的已停止作业
        for (int i = background_jobs.size() - 1; i >= 0; i--) {
            if (background_jobs[i].status == Job::STOPPED) {
                job_id = i + 1;
                break;
            }
        }
        
        if (job_id == 0) {
            std::cout << "bg: no current job" << std::endl;
            return 1;
        }
    }
    
    // 检查作业ID是否有效
    if (job_id <= 0 || job_id > background_jobs.size()) {
        std::cout << "bg: no such job" << std::endl;
        return 1;
    }
    
    Job& job = background_jobs[job_id-1];
    
    if (job.status == Job::DONE) {
        std::cout << "bg: job has terminated" << std::endl;
        return 1;
    }
    
    if (job.status != Job::STOPPED) {
        std::cout << "bg: job already in background" << std::endl;
        return 1;
    }
    
    // 发送继续信号
    if (kill(-job.pgid, SIGCONT) < 0) {
        std::cout << RED << "kill: " << strerror(errno) << RESET << std::endl;
        return 1;
    }
    
    job.status = Job::RUNNING;
    std::cout << "[" << job_id << "] " << job.cmd << " &" << std::endl;
    
    return 0;
}

int main(int argc, char* argv[])
{
    // 注册信号处理器
    signal(SIGTERM, handle_signal);
    signal(SIGQUIT, handle_signal);
    signal(SIGTSTP, handle_signal);  // 注册Ctrl+Z信号处理
    // 不注册SIGINT，让读取函数自己处理
    
    if (argc == 2 && (std::string(argv[1]) == "-v" || std::string(argv[1]) == "--version")) {
        std::cout << "cash version 0.1" << std::endl;
        return EXIT_SUCCESS;
    }
    
    cash::greet();
    cash::loop();
    
    // 确保终端恢复
    cash::disable_raw_mode();
    return EXIT_SUCCESS;
}
