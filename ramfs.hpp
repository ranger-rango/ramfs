#pragma once

#include <iostream>
#include <vector>
#include <map>
#include <string>
#include <sstream>
#include <memory>
#include <functional>
#include <algorithm>

using namespace std;

class RamFSExceptions
{
    private:
        string errMsg;
    public:
        RamFSExceptions(string err)
        : errMsg(err)
        {}

        string getErrMsg()
        {
            return errMsg;
        }

        ~RamFSExceptions()
        {}
};

class NodeExistsException : public RamFSExceptions
{
    public:
        NodeExistsException(string err)
        : RamFSExceptions(err)
        {}

};

class InvalidPathException : public RamFSExceptions
{
    public:
        InvalidPathException(string err)
        : RamFSExceptions(err)
        {}
};

class Node
{
    private:
        bool isDir = false;
        string name = "";
        string fileContent;
        map< string, shared_ptr<Node> > kids;
        weak_ptr<Node> parent;
    public:
        Node()
        {}

        Node(bool is_dir, const string& dirName)
        : isDir(is_dir), name(dirName)
        {}

        void setType (bool is_dir)
        {
            isDir = is_dir;
        }

        void setName(string objName)
        {
            name = objName;
        }

        void setContent(const string& strContent)
        {
            if (!isDir)
            {
                fileContent = strContent;
            }
            else
            {
                cerr << "Can not write to directory" << endl;
            }
        }

        void setKids(string childName, shared_ptr<Node> childPtr)
        {

            auto status = kids.insert({childName, childPtr});
            if (!status.second)
            {
                throw NodeExistsException("RamFS Exception: Directory Already Exists ");
            }
        }

        void rmKid(string childName)
        {
            // shared_ptr<Node> child = kids.at(childName);
            auto it = kids.find(childName);
            if (it != kids.end())
            {
                shared_ptr<Node> child = it->second;
                kids.erase(it);
                child.reset();
            }

        }

        void setParent(shared_ptr<Node> prnt)
        {
            parent = prnt;
        }

        shared_ptr<Node> getChild(const string& childName)
        {
            auto it = kids.find(childName);
            if (it != kids.end())
            {
                return it -> second;
            }
            return nullptr;
        }

        vector<string> getChildren()
        {
            vector<string> children;
            for (const auto& [key, value] : kids)
            {
                children.emplace_back(key);
            }

            return children;
        }

        weak_ptr<Node> getParent()
        {
            return parent;
        }

        string getContent()
        {
            return fileContent;
        }

        bool getType()
        {
            return isDir;
        }

        string getName()
        {
            return name;
        }

        ~Node()
        {}
};

class RamFS
{
    private:
        static string currentDir;
        static shared_ptr<Node> currentNode;
        static shared_ptr<Node> rootNode;
    public:
        RamFS()
        {}

        vector<string> split (const string& str, char delimiter)
        {
            vector<string> tokens;
            stringstream ss(str);
            string item;

            while(getline(ss, item, delimiter))
            {
                tokens.emplace_back(item);
            }
            return tokens;
        }

        shared_ptr<Node> pathResolution (const string& path)
        {
            auto nodes = split(path, '/');
            auto currNode = currentNode;
            shared_ptr<Node> nodeObj;

            if (path[0] == '/')
            {
                currNode = rootNode;
                nodes.erase(nodes.begin());
                if (nodes.empty())
                {
                    return currNode;
                }
                for (string& node : nodes)
                {
                    nodeObj = currNode -> getChild(node);

                    try
                    {
                        if (!nodeObj)
                        {
                            throw InvalidPathException("RamFS Exception: Path Component Not Found");
                        }
                    }
                    catch(InvalidPathException& e)
                    {
                        std::cerr << e.getErrMsg() << endl;
                    }
                    
                    if (nodeObj -> getType())
                    {
                        currNode = nodeObj;
                    }
                }
            }
            else if (path.substr(0, 2) == "..")
            {
                if (auto currNodeParent = currNode -> getParent().lock())
                {
                    currNode = currNodeParent;
                }
                nodes.erase(nodes.begin());
                if (nodes.empty())
                {
                    return currNode;
                }
                for (string& node : nodes)
                {
                    nodeObj = currNode -> getChild(node);
                    try
                    {
                        if (!nodeObj)
                        {
                            throw InvalidPathException("RamFS Exception: Path Component Not Found");
                        }
                    }
                    catch(InvalidPathException& e)
                    {
                        std::cerr << e.getErrMsg() << endl;
                        return currentNode;
                    }
                    if (nodeObj -> getType())
                    {
                        currNode = nodeObj;
                    }
                }
            }
            else if (path[0] == '.')
            {
                return currNode;
            }
            else
            {
                for (string& node : nodes)
                {
                    nodeObj = currNode -> getChild(node);
                    try
                    {
                        if (!nodeObj)
                        {
                            throw InvalidPathException("RamFS Exception: Path Component Not Found");
                        }
                    }
                    catch(InvalidPathException& e)
                    {
                        std::cerr << e.getErrMsg() << endl;
                        return currentNode;
                    }
                    if (nodeObj -> getType())
                    {
                        currNode = nodeObj;
                    }
                }
            }

            return nodeObj;
        }

        string buildPath(shared_ptr<Node> node)
        {
            vector<string> parts;
            while (auto parent = node->getParent().lock())
            {
                parts.push_back(node->getName());
                node = parent;
            }
            if (!node->getName().empty())
            {
                parts.push_back(node->getName());
            }
            reverse(parts.begin(), parts.end());

            string path = "/";
            for (size_t i = 1; i < parts.size(); i++)
            {
                path += parts[i];
                if (i + 1 < parts.size()) path += "/";
            }
            return path;
        }

        void ls(const string& path = ".")
        {
            auto resolvedNode = pathResolution(path);
            if (!resolvedNode -> getType())
            {
                cout << resolvedNode -> getName() << endl;
                return;
            }
            auto children = resolvedNode -> getChildren();
            for (const auto& child : children)
            {
                cout << child << endl;
            }
            return;
        }

        void cd(const string& path = ".")
        {
            auto resolvedNode = pathResolution(path);
            
            try
            {
                if (!resolvedNode -> getType()) throw InvalidPathException("RamFS Exception: Not a directory");
            }
            catch (InvalidPathException& e)
            {
                cerr << e.getErrMsg() << endl;
                return;
            }
            currentNode = resolvedNode;
            currentDir = buildPath(resolvedNode);
            return;
        }

        int mkdir(const string& dirName)
        {
            shared_ptr<Node> child = make_shared<Node> (true, dirName);
            try
            {
                currentNode -> setKids(dirName, child);
            }
            catch(NodeExistsException& e)
            {
                cerr << e.getErrMsg() << endl;
            }
            child -> setParent(currentNode);

            return 0;
        }

        int rmdir(const string& dirName)
        {
            currentNode -> rmKid(dirName);
            return 0;
        }

        int mknod(const string& fileName)
        {
            shared_ptr<Node> childFile = make_shared<Node> (false, fileName);
            try
            {
                currentNode -> setKids(fileName, childFile);
            }
            catch(NodeExistsException& e)
            {
                cerr << e.getErrMsg() << endl;
            }
            childFile -> setParent(currentNode);
            return 0;
        }

        int rmnod(const string& fileName)
        {
            currentNode -> rmKid(fileName);
            return 0;
        }

        int write(const string& filePath, string content)
        {
            // shared_ptr<Node> fileNode = currentNode -> getChild(fileName);
            auto resolvedNode = pathResolution(filePath);
            if (resolvedNode != nullptr)
            {
                resolvedNode -> setContent(content);
                return 0;
            }
            return 1;
        }

        void read(const string& filePath)
        {
            auto resolvedNode = pathResolution(filePath);
            if (resolvedNode != nullptr)
            {
                string fileContent = resolvedNode -> getContent();
                cout << fileContent << endl;
            }
        }

        string getCurrentDir()
        {
            return RamFS::currentDir;
        }

        ~RamFS()
        {}

};

string RamFS::currentDir = "/";
shared_ptr<Node> RamFS::rootNode = make_shared<Node> (true, "root");
shared_ptr<Node> RamFS::currentNode = RamFS::rootNode;