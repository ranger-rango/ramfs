#include "ramfs.hpp"
#include <unordered_map>
#include <functional>
#include <ranges>

string join(vector<string> arr, const string& sep = " ")
{
    string joined;
    for (int i = 0; i < arr.size() - 1; i ++)
    {
        joined += arr[i] + sep;
    }
    joined += arr[arr.size() - 1];

    return joined;
}
int main()
{
    // cmds - cd, ls, touch, rm, mkdir, rmdir, exit
    RamFS ramFS;
    string fsCmd;
    string ramFSAscii = R"(
 _  .-')     ('-.     _   .-')                .-')    
( \( -O )   ( OO ).-.( '.( OO )_             ( OO ).  
 ,------.   / . --. / ,--.   ,--.)  ,------.(_)---\_) 
 |   /`. '  | \-.  \  |   `.'   |('-| _.---'/    _ |  
 |  /  | |.-'-'  |  | |         |(OO|(_\    \  :` `.  
 |  |_.' | \| |_.'  | |  |'.'|  |/  |  '--.  '..`''.) 
 |  .  '.'  |  .-.  | |  |   |  |\_)|  .--' .-._)   \ 
 |  |\  \   |  | |  | |  |   |  |  \|  |_)  \       / 
 `--' '--'  `--' `--' `--'   `--'   `--'     `-----'  
    )";

    string helpMsg = R"(
    +-------------------+----------------------------------+
    | Command           | Usage                            |
    +-------------------+----------------------------------+
    | make directory    | mkdir <dir-name>                 |
    | delete directory  | rmdir <dir-name>                 |
    | make file         | touch <file-name>                |
    | delete file / dir | rm <file-name> / <dir-name>      |
    | navigate to dir   | cd <dir-name>                    |
    | list contents     | ls <dir-name>                    |
    | write to file     | write <file-name> <contents>     |
    | read file         | read <file-name>                 |
    +-------------------+----------------------------------+

    Notes:
    - Current directory is represented by `.`  
    - Parent directory is represented by `..`
    )";
    unordered_map<string, function< void(const string&, const string&) > > commands = 
    {
        {"cd", [&](const string& path, const string&){ramFS.cd(path.empty() ? "." : path);} },
        {"ls", [&](const string& path, const string&){ramFS.ls(path.empty() ? "." : path);} },
        {"touch", [&](const string& path, const string&){ramFS.mknod(path);} },
        {"rm", [&](const string& path, const string&){ramFS.rmnod(path);} },
        {"mkdir", [&](const string& path, const string&){ramFS.mkdir(path);} },
        {"rmdir", [&](const string& path, const string&){ramFS.rmdir(path);} },
        {"write", [&](const string& path, const string& content){ramFS.write(path, content);} },
        {"read", [&](const string& path, const string& content){ramFS.read(path);} },
        {"help", [&](const string& path, const string&){cout << helpMsg << endl;} },
        {"exit", [&](const string& path, const string&){cout << "Catchya Later:)" << endl; exit(0);} }
    };

    cout << ramFSAscii << endl << endl;
    while (true)
    {
        cout << "--(RamFS)--[" << ramFS.getCurrentDir() << "]--# ";
        getline(cin, fsCmd);
        if (fsCmd.empty()) continue;

        auto fsCmds = ramFS.split(fsCmd, ' ');
        string cmd = fsCmds[0];
        string path;
        string fileContent;
        if (fsCmds.size() >= 2)
        {
            path = fsCmds[1];
        }
        if (fsCmds.size() >= 3)
        {
            vector<string> fileContentArr(fsCmds.begin() + 2, fsCmds.end());
            fileContent = join(fileContentArr);
        }
        auto it = commands.find(cmd);
        if (it != commands.end())
        {
            it->second(path, fileContent);
        }
        else
        {
            cout << "Unknown command" << endl;
        }
    }
    
    return 0;
}