#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <iomanip>

using namespace std;

#define MAX_FILE_SIZE 1024
#define MSD 50  // 最大子目录数
#define DISK_SIZE 67108864  // 磁盘大小（字节数）
#define MAX_DISK_PARTITIONS 5  // 最大磁盘分区数

// 文件/目录项的结构
struct DirItem {
    string fileName;  // 文件/目录名称
    int type;  // 0 表示文件，1 表示目录
    int next;  // 文件/目录的下一个块的磁盘地址
    int size;  // 文件/目录的大小（字节数）
    string updateTime;  // 文件/目录的最后更新时间
};

// 目录的结构
struct Directory {
    DirItem directItem[MSD + 2];  // 目录项数组
};

// 磁盘分区的结构
struct DiskPartition {
    string partitionName;  // 分区名称（例如 A#、B#、C#）
    int partitionSize;  // 分区大小（以 MB 为单位）
    string filePath;  // 分区文件的路径
    Directory rootDirectory;  // 分区的根目录
    int bitmap[DISK_SIZE / MAX_FILE_SIZE];  // 用于跟踪空闲/已分配的块
};
struct VirtualDisk {
    string filePath;
    char driveLetter;
    int bitmap[DISK_SIZE / MAX_FILE_SIZE];
    Directory rootDirectory;
};
vector<DiskPartition> diskPartitions;  // 存储磁盘分区的向量
DiskPartition* currentPartition;  // 当前分区的指针
vector<VirtualDisk*> partitions;// 分区列表
Directory* currentDirectory;  // 当前目录
char nextDriveLetter = 'A';  // 下一个可用的盘符
string userName = "test";
string userDat;  // 用户的磁盘数据文件

void strip(string& str, char ch) {
    str.erase(remove(str.begin(), str.end(), ch), str.end());
}

void init(string fileName) {
    fstream fp;
    fp.open(fileName, ios::out | ios::binary);
    if (fp) {
        // 将磁盘空间初始化为 0
        char* buffer = new char[DISK_SIZE];
        memset(buffer, 0, DISK_SIZE);
        fp.write(buffer, DISK_SIZE);
        delete[] buffer;

        // 初始化磁盘分区
        DiskPartition partition;
        partition.partitionName = "A#";
        partition.partitionSize = 0;
        partition.filePath = fileName;
        diskPartitions.push_back(partition);

        currentPartition = &diskPartitions[0];

        fp.close();
        cout << "磁盘初始化成功！" << endl;
    }
    else {
        cout << "无法创建磁盘数据文件！" << endl;
    }
}

void readUserDisk(string fileName) {
    fstream fp;
    fp.open(fileName, ios::in | ios::binary);
    if (fp) {
        // 读取磁盘分区信息
        for (int i = 0; i < diskPartitions.size(); i++) {
            fp.seekg(i * DISK_SIZE, ios::beg);
            fp.read(reinterpret_cast<char*>(&diskPartitions[i].rootDirectory), sizeof(Directory));
        }

        fp.close();
    }
}

void save() {
    fstream fp;
    fp.open(userDat, ios::out | ios::binary);
    if (fp) {
        // 保存磁盘分区信息
        for (int i = 0; i < diskPartitions.size(); i++) {
            fp.seekp(i * DISK_SIZE, ios::beg);
            fp.write(reinterpret_cast<char*>(&diskPartitions[i].rootDirectory), sizeof(Directory));
        }

        fp.close();
    }
}

void release() {
    save();
}

void Shell() {
    cout << currentPartition->partitionName << "> ";
}

// 初始化磁盘分区
void InitDisk(string partitionSizes) {
    istringstream iss(partitionSizes);
    string partitionSize;
    while (getline(iss, partitionSize, ',')) {
        strip(partitionSize, ' ');
        strip(partitionSize, 'M');

        int size = stoi(partitionSize);
        if (size <= 0) {
            cout << "错误：分区大小必须大于零！" << endl;
            return;
        }

        DiskPartition partition;
        partition.partitionName = string(1, 'A' + diskPartitions.size()) + "#";
        partition.partitionSize = size;
        partition.filePath = userDat;
        diskPartitions.push_back(partition);
    }

    currentPartition = &diskPartitions[0];
    cout << "分区初始化成功！" << endl;
}

// 切换当前分区
//void ChgDisk(string partitionName) {
//    for (int i = 0; i < diskPartitions.size(); i++) {
//        if (diskPartitions[i].partitionName == partitionName) {
//            currentPartition = &diskPartitions[i];
//            cout << "当前分区切换为：" << partitionName << endl;
//            return;
//        }
//    }
//
//    cout << "错误：指定的分区不存在！" << endl;
//}
//
//// 显示分区的空间统计情况
//void ShowDisk(string partitionName) {
//    for (int i = 0; i < diskPartitions.size(); i++) {
//        if (diskPartitions[i].partitionName == partitionName) {
//            int totalSize = diskPartitions[i].partitionSize * 1024 * 1024;
//            int usedSize = totalSize - (currentPartition->bitmap[0] * MAX_FILE_SIZE);
//            int availableSize = currentPartition->bitmap[0] * MAX_FILE_SIZE;
//
//            cout << "分区：" << partitionName << endl;
//            cout << "总空间大小：" << totalSize << " 字节" << endl;
//            cout << "已用空间大小：" << usedSize << " 字节" << endl;
//            cout << "可用空间大小：" << availableSize << " 字节" << endl;
//            return;
//        }
//    }
//
//    cout << "错误：指定的分区不存在！" << endl;
//}

// 创建指定目录
void MkDir(string path) {
    strip(path, '\\');
    strip(path, '/');

    istringstream iss(path);
    string directoryName;
    Directory* currentDirectory = &currentPartition->rootDirectory;

    while (getline(iss, directoryName, '\\')) {
        if (directoryName.empty()) {
            continue;
        }

        bool directoryExists = false;
        int directoryIndex = -1;
        for (int i = 0; i < MSD + 2; i++) {
            if (currentDirectory->directItem[i].fileName == directoryName && currentDirectory->directItem[i].type == 1) {
                directoryExists = true;
                directoryIndex = i;
                break;
            }
        }

        if (!directoryExists) {
            for (int i = 0; i < MSD + 2; i++) {
                if (currentDirectory->directItem[i].fileName.empty()) {
                    currentDirectory->directItem[i].fileName = directoryName;
                    currentDirectory->directItem[i].type = 1;
                    currentDirectory->directItem[i].next = -1;
                    currentDirectory->directItem[i].size = 0;
                    currentDirectory->directItem[i].updateTime = __TIMESTAMP__;
                    directoryIndex = i;
                    break;
                }
            }
        }

        if (directoryIndex == -1) {
            cout << "错误：无法创建目录，目录项已满！" << endl;
            return;
        }

        if (directoryExists) {
            currentDirectory = &currentPartition->rootDirectory;
            currentDirectory = reinterpret_cast<Directory*>(&currentPartition->rootDirectory.directItem[directoryIndex]);
        }
    }

    cout << "目录创建成功！" << endl;
}

// 删除空目录
void DelDir(string path) {
    strip(path, '\\');
    strip(path, '/');

    istringstream iss(path);
    string directoryName;
    Directory* currentDirectory = &currentPartition->rootDirectory;

    while (getline(iss, directoryName, '\\')) {
        if (directoryName.empty()) {
            continue;
        }

        bool directoryExists = false;
        int directoryIndex = -1;
        for (int i = 0; i < MSD + 2; i++) {
            if (currentDirectory->directItem[i].fileName == directoryName && currentDirectory->directItem[i].type == 1) {
                directoryExists = true;
                directoryIndex = i;
                break;
            }
        }

        if (!directoryExists) {
            cout << "错误：指定的目录不存在！" << endl;
            return;
        }

        currentDirectory = &currentPartition->rootDirectory;
        currentDirectory = reinterpret_cast<Directory*>(&currentPartition->rootDirectory.directItem[directoryIndex]);
    }

    // 检查目录是否为空
    for (int i = 0; i < MSD + 2; i++) {
        if (!currentDirectory->directItem[i].fileName.empty()) {
            cout << "错误：目录非空，无法删除！" << endl;
            return;
        }
    }

    currentDirectory->directItem[0].fileName = "";
    currentDirectory->directItem[0].type = 0;
    currentDirectory->directItem[0].next = -1;
    currentDirectory->directItem[0].size = 0;
    currentDirectory->directItem[0].updateTime = "";
}

// 显示目录详情
void Dir(string path) {
    strip(path, '\\');
    strip(path, '/');

    if (path.empty()) {
        currentPartition->rootDirectory.directItem[MSD + 1].fileName = "";
        currentPartition->rootDirectory.directItem[MSD + 1].type = 0;
        currentPartition->rootDirectory.directItem[MSD + 1].next = -1;
        currentPartition->rootDirectory.directItem[MSD + 1].size = 0;
        currentPartition->rootDirectory.directItem[MSD + 1].updateTime = "";

        currentDirectory = &currentPartition->rootDirectory;
        path = ".";
    }
    else {
        istringstream iss(path);
        string directoryName;
        Directory* tempDirectory = &currentPartition->rootDirectory;

        while (getline(iss, directoryName, '\\')) {
            if (directoryName.empty()) {
                continue;
            }

            bool directoryExists = false;
            int directoryIndex = -1;
            for (int i = 0; i < MSD + 2; i++) {
                if (tempDirectory->directItem[i].fileName == directoryName && tempDirectory->directItem[i].type == 1) {
                    directoryExists = true;
                    directoryIndex = i;
                    break;
                }
            }

            if (!directoryExists) {
                cout << "错误：指定的目录不存在！" << endl;
                return;
            }

            tempDirectory = reinterpret_cast<Directory*>(&tempDirectory->directItem[directoryIndex]);
        }

        currentDirectory = tempDirectory;
    }

    cout << "目录：" << path << endl;
    cout << setw(20) << left << "名称" << setw(20) << left << "类型" << setw(20) << left << "大小" << setw(20) << left << "更新时间" << endl;

    for (int i = 1; i < MSD + 2; i++) {
        if (!currentDirectory->directItem[i].fileName.empty()) {
            cout << setw(20) << left << currentDirectory->directItem[i].fileName;
            cout << setw(20) << left << (currentDirectory->directItem[i].type == 1 ? "目录" : "文件");
            cout << setw(20) << left << currentDirectory->directItem[i].size;
            cout << setw(20) << left << currentDirectory->directItem[i].updateTime << endl;
        }
    }
}

// 切换当前目录
void ChgDir(string path) {
    strip(path, '\\');
    strip(path, '/');

    if (path.empty() || path == ".") {
        currentDirectory = &currentPartition->rootDirectory;
        return;
    }

    istringstream iss(path);
    string directoryName;
    Directory* tempDirectory = &currentPartition->rootDirectory;

    while (getline(iss, directoryName, '\\')) {
        if (directoryName.empty()) {
            continue;
        }

        bool directoryExists = false;
        int directoryIndex = -1;
        for (int i = 0; i < MSD + 2; i++) {
            if (tempDirectory->directItem[i].fileName == directoryName && tempDirectory->directItem[i].type == 1) {
                directoryExists = true;
                directoryIndex = i;
                break;
            }
        }

        if (!directoryExists) {
            cout << "错误：指定的目录不存在！" << endl;
            return;
        }

        tempDirectory = reinterpret_cast<Directory*>(&tempDirectory->directItem[directoryIndex]);
    }

    currentDirectory = tempDirectory;
}

// 树状显示目录结构
void TreeDir(string path, int level = 0) {
    strip(path, '\\');
    strip(path, '/');

    if (path.empty()) {
        currentPartition->rootDirectory.directItem[MSD + 1].fileName = "";
        currentPartition->rootDirectory.directItem[MSD + 1].type = 0;
        currentPartition->rootDirectory.directItem[MSD + 1].next = -1;
        currentPartition->rootDirectory.directItem[MSD + 1].size = 0;
        currentPartition->rootDirectory.directItem[MSD + 1].updateTime = "";

        currentDirectory = &currentPartition->rootDirectory;
        path = ".";
    }
    else {
        istringstream iss(path);
        string directoryName;
        Directory* tempDirectory = &currentPartition->rootDirectory;

        while (getline(iss, directoryName, '\\')) {
            if (directoryName.empty()) {
                continue;
            }

            bool directoryExists = false;
            int directoryIndex = -1;
            for (int i = 0; i < MSD + 2; i++) {
                if (tempDirectory->directItem[i].fileName == directoryName && tempDirectory->directItem[i].type == 1) {
                    directoryExists = true;
                    directoryIndex = i;
                    break;
                }
            }

            if (!directoryExists) {
                cout << "错误：指定的目录不存在！" << endl;
                return;
            }

            tempDirectory = reinterpret_cast<Directory*>(&tempDirectory->directItem[directoryIndex]);
        }

        currentDirectory = tempDirectory;
    }

    cout << setw(level * 4) << "" << path << endl;

    for (int i = 1; i < MSD + 2; i++) {
        if (!currentDirectory->directItem[i].fileName.empty()) {
            if (currentDirectory->directItem[i].type == 1) {
                TreeDir(path + "\\" + currentDirectory->directItem[i].fileName, level + 1);
            }
            else {
                cout << setw((level + 1) * 4) << "" << currentDirectory->directItem[i].fileName << endl;
            }
        }
    }
}

// 把目录迁移到另一个目录下
void MoveDir(string sourcePath, string destinationPath) {
    strip(sourcePath, '\\');
    strip(sourcePath, '/');
    strip(destinationPath, '\\');
    strip(destinationPath, '/');

    if (sourcePath == destinationPath) {
        cout << "错误：源目录和目标目录相同！" << endl;
        return;
    }

    if (sourcePath.empty() || destinationPath.empty()) {
        cout << "错误：源目录和目标目录不能为空！" << endl;
        return;
    }

    istringstream sourceIss(sourcePath);
    istringstream destinationIss(destinationPath);
    string sourceDirectoryName, destinationDirectoryName;
    Directory* sourceDirectory = &currentPartition->rootDirectory;
    Directory* destinationDirectory = &currentPartition->rootDirectory;

    while (getline(sourceIss, sourceDirectoryName, '\\')) {
        if (sourceDirectoryName.empty()) {
            continue;
        }

        bool sourceDirectoryExists = false;
        int sourceDirectoryIndex = -1;
        for (int i = 0; i < MSD + 2; i++) {
            if (sourceDirectory->directItem[i].fileName == sourceDirectoryName && sourceDirectory->directItem[i].type == 1) {
                sourceDirectoryExists = true;
                sourceDirectoryIndex = i;
                break;
            }
        }

        if (!sourceDirectoryExists) {
            cout << "错误：源目录不存在！" << endl;
            return;
        }

        sourceDirectory = reinterpret_cast<Directory*>(&sourceDirectory->directItem[sourceDirectoryIndex]);
    }

    while (getline(destinationIss, destinationDirectoryName, '\\')) {
        if (destinationDirectoryName.empty()) {
            continue;
        }

        bool destinationDirectoryExists = false;
        int destinationDirectoryIndex = -1;
        for (int i = 0; i < MSD + 2; i++) {
            if (destinationDirectory->directItem[i].fileName == destinationDirectoryName && destinationDirectory->directItem[i].type == 1) {
                destinationDirectoryExists = true;
                destinationDirectoryIndex = i;
                break;
            }
        }

        if (!destinationDirectoryExists) {
            cout << "错误：目标目录不存在！" << endl;
            return;
        }

        destinationDirectory = reinterpret_cast<Directory*>(&destinationDirectory->directItem[destinationDirectoryIndex]);
    }

    // 检查目标目录是否已存在同名的源目录
    for (int i = 0; i < MSD + 2; i++) {
        if (destinationDirectory->directItem[i].fileName == sourceDirectoryName && destinationDirectory->directItem[i].type == 1) {
            cout << "错误：目标目录下已存在同名的源目录！" << endl;
            return;
        }
    }

    // 复制源目录
    destinationDirectory->directItem[MSD + 1].fileName = sourceDirectoryName;
    destinationDirectory->directItem[MSD + 1].type = 1;
    destinationDirectory->directItem[MSD + 1].next = -1;
    destinationDirectory->directItem[MSD + 1].size = sourceDirectory->directItem[0].size;
    destinationDirectory->directItem[MSD + 1].updateTime = sourceDirectory->directItem[0].updateTime;

    for (int i = 1; i < MSD + 2; i++) {
        if (!sourceDirectory->directItem[i].fileName.empty()) {
            if (sourceDirectory->directItem[i].type == 1) {
                MoveDir(sourcePath + "\\" + sourceDirectory->directItem[i].fileName, destinationPath + "\\" + sourceDirectory->directItem[i].fileName);
            }
            else {
                destinationDirectory->directItem[i].fileName = sourceDirectory->directItem[i].fileName;
                destinationDirectory->directItem[i].type = 0;
                destinationDirectory->directItem[i].next = sourceDirectory->directItem[i].next;
                destinationDirectory->directItem[i].size = sourceDirectory->directItem[i].size;
                destinationDirectory->directItem[i].updateTime = sourceDirectory->directItem[i].updateTime;
            }
        }
    }

    // 删除源目录
    sourceDirectory->directItem[0].fileName = "";
    sourceDirectory->directItem[0].type = 0;
    sourceDirectory->directItem[0].next = -1;
    sourceDirectory->directItem[0].size = 0;
    sourceDirectory->directItem[0].updateTime = "";
}

// 创建文件
void Create(string fileName) {
    strip(fileName, '\\');
    strip(fileName, '/');

    if (fileName.empty()) {
        cout << "错误：文件名不能为空！" << endl;
        return;
    }

    for (int i = 1; i < MSD + 2; i++) {
        if (currentDirectory->directItem[i].fileName == fileName && currentDirectory->directItem[i].type == 0) {
            cout << "错误：同名文件已存在！" << endl;
            return;
        }
    }

    int size = (MAX_FILE_SIZE + sizeof(DirItem) - 1) / sizeof(DirItem);

    int index = -1;
    for (int i = 1; i < MSD + 2; i++) {
        if (currentDirectory->directItem[i].fileName.empty()) {
            currentDirectory->directItem[i].fileName = fileName;
            currentDirectory->directItem[i].type = 0;
            currentDirectory->directItem[i].next = -1;
            currentDirectory->directItem[i].size = size;
            currentDirectory->directItem[i].updateTime = __TIMESTAMP__;
            index = i;
            break;
        }
    }

    if (index == -1) {
        cout << "错误：无法创建文件，目录项已满！" << endl;
        return;
    }

    // 更新磁盘空闲块
    int blockCount = (size + sizeof(DirItem) - 1) / sizeof(DirItem);
    for (int i = 0; i < blockCount; i++) {
        for (int j = 0; j < DISK_SIZE / MAX_FILE_SIZE; j++) {
            if (currentPartition->bitmap[j] == 0) {
                currentPartition->bitmap[j] = i + 1;
                break;
            }
        }
    }

    cout << "文件创建成功！" << endl;
}

// 复制文件
void Copy(string sourceFileName, string destinationFileName) {
    strip(sourceFileName, '\\');
    strip(sourceFileName, '/');
    strip(destinationFileName, '\\');
    strip(destinationFileName, '/');

    if (sourceFileName.empty() || destinationFileName.empty()) {
        cout << "错误：源文件和目标文件不能为空！" << endl;
        return;
    }

    if (sourceFileName == destinationFileName) {
        cout << "错误：源文件和目标文件相同！" << endl;
        return;
    }

    for (int i = 1; i < MSD + 2; i++) {
        if (currentDirectory->directItem[i].fileName == sourceFileName && currentDirectory->directItem[i].type == 0) {
            for (int j = 1; j < MSD + 2; j++) {
                if (currentDirectory->directItem[j].fileName == destinationFileName && currentDirectory->directItem[j].type == 0) {
                    cout << "错误：目标目录下已存在同名文件！" << endl;
                    return;
                }
            }

            int size = currentDirectory->directItem[i].size;
            int sourceIndex = -1;
            int destinationIndex = -1;
            for (int j = 0; j < DISK_SIZE / MAX_FILE_SIZE; j++) {
                if (currentPartition->bitmap[j] == 0) {
                    sourceIndex = j;
                    break;
                }
            }

            if (sourceIndex == -1) {
                cout << "错误：无法复制文件，磁盘空间已满！" << endl;
                return;
            }

            for (int j = 0; j < DISK_SIZE / MAX_FILE_SIZE; j++) {
                if (currentPartition->bitmap[j] == 0) {
                    destinationIndex = j;
                    break;
                }
            }

            if (destinationIndex == -1) {
                cout << "错误：无法复制文件，磁盘空间已满！" << endl;
                return;
            }

            // 复制文件
            currentPartition->bitmap[sourceIndex] = currentPartition->bitmap[0];
            currentPartition->bitmap[0] = 0;

            currentPartition->bitmap[destinationIndex] = currentPartition->bitmap[0];
            currentPartition->bitmap[0] = sourceIndex + 1;

            currentDirectory->directItem[i].fileName = destinationFileName;
            currentDirectory->directItem[i].updateTime = __TIMESTAMP__;

            cout << "文件复制成功！" << endl;
            return;
        }
    }

    cout << "错误：指定的源文件不存在！" << endl;
}

// 删除文件
void Delete(string fileName) {
    strip(fileName, '\\');
    strip(fileName, '/');

    if (fileName.empty()) {
        cout << "错误：文件名不能为空！" << endl;
        return;
    }

    for (int i = 1; i < MSD + 2; i++) {
        if (currentDirectory->directItem[i].fileName == fileName && currentDirectory->directItem[i].type == 0) {
            int size = currentDirectory->directItem[i].size;
            int index = -1;
            for (int j = 0; j < DISK_SIZE / MAX_FILE_SIZE; j++) {
                if (currentPartition->bitmap[j] == 0) {
                    index = j;
                    break;
                }
            }

            if (index == -1) {
                cout << "错误：无法删除文件，磁盘空间已满！" << endl;
                return;
            }

            // 删除文件
            currentPartition->bitmap[index] = currentPartition->bitmap[0];
            currentPartition->bitmap[0] = size / sizeof(DirItem);

            currentDirectory->directItem[i].fileName = "";
            currentDirectory->directItem[i].type = 0;
            currentDirectory->directItem[i].next = -1;
            currentDirectory->directItem[i].size = 0;
            currentDirectory->directItem[i].updateTime = "";

            cout << "文件删除成功！" << endl;
            return;
        }
    }

    cout << "错误：指定的文件不存在！" << endl;
}

// 打开文件并支持写入保存
void Write(string fileName) {
    strip(fileName, '\\');
    strip(fileName, '/');

    if (fileName.empty()) {
        cout << "错误：文件名不能为空！" << endl;
        return;
    }

    for (int i = 1; i < MSD + 2; i++) {
        if (currentDirectory->directItem[i].fileName == fileName && currentDirectory->directItem[i].type == 0) {
            cout << "请输入文件内容（以EOF结束）：" << endl;

            string content;
            string line;
            while (getline(cin, line) && line != "EOF") {
                content += line + "\n";
            }

            if (content.empty()) {
                cout << "错误：文件内容不能为空！" << endl;
                return;
            }

            int size = (content.size() + sizeof(DirItem) - 1) / sizeof(DirItem);

            int index = -1;
            for (int j = 0; j < DISK_SIZE / MAX_FILE_SIZE; j++) {
                if (currentPartition->bitmap[j] == 0) {
                    index = j;
                    break;
                }
            }

            if (index == -1) {
                cout << "错误：无法写入文件，磁盘空间已满！" << endl;
                return;
            }

            // 更新文件大小
            currentDirectory->directItem[i].size = size;
            currentDirectory->directItem[i].updateTime = __TIMESTAMP__;

            // 写入文件内容
            ofstream outFile;
            outFile.open(currentPartition->filePath, ios::binary | ios::in | ios::out);
            if (outFile) {
                int offset = (index - 1) * MAX_FILE_SIZE;
                outFile.seekp(offset, ios::beg);
                outFile.write(reinterpret_cast<const char*>(&currentDirectory->directItem[i]), sizeof(DirItem));
                outFile.write(content.c_str(), content.size());
                outFile.close();

                cout << "文件写入成功！" << endl;
            }
            else {
                cout << "错误：无法打开文件！" << endl;
            }

            return;
        }
    }

    cout << "错误：指定的文件不存在！" << endl;
}

// 打开文件并输出显示
void Read(string fileName) {
    strip(fileName, '\\');
    strip(fileName, '/');

    if (fileName.empty()) {
        cout << "错误：文件名不能为空！" << endl;
        return;
    }

    for (int i = 1; i < MSD + 2; i++) {
        if (currentDirectory->directItem[i].fileName == fileName && currentDirectory->directItem[i].type == 0) {
            int size = currentDirectory->directItem[i].size;

            int index = currentPartition->bitmap[0] - 1;
            if (index == 0) {
                index = -1;
            }

            if (index == -1) {
                cout << "错误：文件内容为空！" << endl;
                return;
            }

            // 读取文件内容
            ifstream inFile;
            inFile.open(currentPartition->filePath, ios::binary | ios::in | ios::out);
            if (inFile) {
                int offset = index * MAX_FILE_SIZE;
                inFile.seekg(offset, ios::beg);

                DirItem item;
                inFile.read(reinterpret_cast<char*>(&item), sizeof(DirItem));

                char* content = new char[size];
                inFile.read(content, size);

                cout << "文件内容：" << endl;
                cout.write(content, size);
                cout << endl;

                inFile.close();
            }
            else {
                cout << "错误：无法打开文件！" << endl;
            }

            return;
        }
    }

    cout << "错误：指定的文件不存在！" << endl;
}

// 导出文件到外部硬盘
void Export(string fileName, string externalDirectory) {
    strip(fileName, '\\');
    strip(fileName, '/');
    strip(externalDirectory, '\\');
    strip(externalDirectory, '/');

    if (fileName.empty() || externalDirectory.empty()) {
        cout << "错误：文件名和外部目录不能为空！" << endl;
        return;
    }

    for (int i = 1; i < MSD + 2; i++) {
        if (currentDirectory->directItem[i].fileName == fileName && currentDirectory->directItem[i].type == 0) {
            int size = currentDirectory->directItem[i].size;

            int index = currentPartition->bitmap[0] - 1;
            if (index == 0) {
                index = -1;
            }

            if (index == -1) {
                cout << "错误：文件内容为空！" << endl;
                return;
            }

            // 读取文件内容
            ifstream inFile;
            inFile.open(currentPartition->filePath, ios::binary | ios::in | ios::out);
            if (inFile) {
                int offset = index * MAX_FILE_SIZE;
                inFile.seekg(offset, ios::beg);

                DirItem item;
                inFile.read(reinterpret_cast<char*>(&item), sizeof(DirItem));

                char* content = new char[size];
                inFile.read(content, size);

                // 导出文件到外部硬盘
                string externalFilePath = externalDirectory + "\\" + fileName;
                ofstream outFile;
                outFile.open(externalFilePath, ios::binary);
                if (outFile) {
                    outFile.write(content, size);
                    outFile.close();

                    cout << "文件导出成功！" << endl;
                }
                else {
                    cout << "错误：无法打开外部目录！" << endl;
                }

                inFile.close();
            }
            else {
                cout << "错误：无法打开文件！" << endl;
            }

            return;
        }
    }

    cout << "错误：指定的文件不存在！" << endl;
}

// 初始化虚拟磁盘和分区
//void InitDisk(string partitionSizes) {
//    istringstream iss(partitionSizes);
//    string partitionSize;
//
//    while (getline(iss, partitionSize, ',')) {
//        strip(partitionSize, ' ');
//
//        if (partitionSize.empty()) {
//            continue;
//        }
//
//        string partitionName = string(1, nextDriveLetter) + "#";
//        nextDriveLetter++;
//
//        VirtualDisk* newPartition = new VirtualDisk();
//        newPartition->filePath = partitionName + ".vd";
//        newPartition->driveLetter = partitionName[0];
//
//        // 初始化位示图
//        for (int i = 0; i < DISK_SIZE / MAX_FILE_SIZE; i++) {
//            newPartition->bitmap[i] = 0;
//        }
//
//        // 初始化根目录
//        newPartition->rootDirectory.directItem[0].fileName = "";
//        newPartition->rootDirectory.directItem[0].type = 1;
//        newPartition->rootDirectory.directItem[0].next = -1;
//        newPartition->rootDirectory.directItem[0].size = 0;
//        newPartition->rootDirectory.directItem[0].updateTime = "";
//
//        // 初始化分区大小
//        int size = stoi(partitionSize.substr(0, partitionSize.size() - 1));
//        newPartition->rootDirectory.directItem[0].size = size / sizeof(DirItem);
//
//        // 更新磁盘空闲块
//        int blockCount = (size + sizeof(DirItem) - 1) / sizeof(DirItem);
//        for (int i = 0; i < blockCount; i++) {
//            newPartition->bitmap[i] = i + 1;
//        }
//
//        // 将新分区加入分区列表
//        partitions.push_back(newPartition);
//
//        cout << "分区 " << partitionName << " 初始化成功，大小为 " << partitionSize << "！" << endl;
//    }
//
//    currentPartition = &diskPartitions[0];
//    currentDirectory = &currentPartition->rootDirectory;
//}

// 切换当前分区
void ChgDisk(char driveLetter) {
    for (int i = 0; i < partitions.size(); i++) {
        if (partitions[i]->driveLetter == driveLetter) {
            currentPartition = &diskPartitions[0];
            currentDirectory = &currentPartition->rootDirectory;
            return;
        }
    }

    cout << "错误：指定的分区不存在！" << endl;
}

// 显示分区空间统计情况
void ShowDisk(char driveLetter) {
    for (int i = 0; i < partitions.size(); i++) {
        if (partitions[i]->driveLetter == driveLetter) {
            int totalSize = DISK_SIZE;
            int usedSize = 0;
            int availableSize = 0;

            for (int j = 0; j < DISK_SIZE / MAX_FILE_SIZE; j++) {
                if (partitions[i]->bitmap[j] == 0) {
                    availableSize += MAX_FILE_SIZE;
                }
                else {
                    usedSize += MAX_FILE_SIZE;
                }
            }

            cout << "分区 " << partitions[i]->driveLetter << "# 的空间统计情况：" << endl;
            cout << "总空间大小：" << totalSize << "B" << endl;
            cout << "已用空间大小：" << usedSize << "B" << endl;
            cout << "可用空间大小：" << availableSize << "B" << endl;

            return;
        }
    }

    cout << "错误：指定的分区不存在！" << endl;
}
void help()
{
    cout << fixed << left;
    cout << "******************** 帮助 ********************" << endl << endl;
    cout << setw(40) << "InitDisk A xxM, B yyM, C zzM,..." << setw(10) << "初始化磁盘分区" << endl;
    cout << setw(40) << "ChgDisk 分区" << setw(10) << "切换当前分区" << endl;
    cout << setw(40) << "ShowDisk 分区" << setw(10) << "显示分区空间统计情况" << endl;
    cout << setw(40) << "MkDir 目录名" << setw(10) << "创建目录" << endl;
    cout << setw(40) << "DelDir 目录名" << setw(10) << "删除空目录" << endl;
    cout << setw(40) << "Dir 目录名" << setw(10) << "显示目录详情" << endl;
    cout << setw(40) << "ChgDir 目录名" << setw(10) << "切换当前目录" << endl;
    cout << setw(40) << "TreeDir 目录名" << setw(10) << "树状显示目录结构" << endl;
    cout << setw(40) << "MoveDir 目录1 目录2" << setw(10) << "移动目录" << endl;
    cout << setw(40) << "Create 文件名" << setw(10) << "创建文件" << endl;
    cout << setw(40) << "Copy 源文件 目标文件" << setw(10) << "复制文件" << endl;
    cout << setw(40) << "Delete 文件名" << setw(10) << "删除文件" << endl;
    cout << setw(40) << "Write 文件名" << setw(10) << "打开并写入文件" << endl;
    cout << setw(40) << "Read 文件名" << setw(10) << "打开并显示文件内容" << endl;
    cout << setw(40) << "Export 文件名 目录名" << setw(10) << "将文件导出到外部硬盘" << endl;
    cout << setw(40) << "Exit" << setw(10) << "退出" << endl << endl;
}
// 主函数
int main() {
    cout << "欢迎使用虚拟磁盘系统！" << endl;
    cout << "请输入指令（输入help获取帮助）：" << endl;

    while (true) {
        cout << "> ";
        string command;
        getline(cin, command);

        if (command.empty()) {
            continue;
        }

        istringstream iss(command);
        string keyword;
        iss >> keyword;

        if (keyword == "help") {
            help();
        }
        else if (keyword == "InitDisk") {
            string partitionSizes;
            iss >> partitionSizes;
            InitDisk(partitionSizes);
        }
        else if (keyword == "ChgDisk") {
            char driveLetter;
            iss >> driveLetter;
            ChgDisk(driveLetter);
        }
        else if (keyword == "ShowDisk") {
            char driveLetter;
            iss >> driveLetter;
            ShowDisk(driveLetter);
        }
        else if (keyword == "MkDir") {
            string path;
            iss >> path;
            MkDir(path);
        }
        else if (keyword == "DelDir") {
            string path;
            iss >> path;
            DelDir(path);
        }
        else if (keyword == "Dir") {
            string path;
            iss >> path;
            Dir(path);
        }
        else if (keyword == "ChgDir") {
            string path;
            iss >> path;
            ChgDir(path);
        }
        else if (keyword == "TreeDir") {
            string path;
            iss >> path;
            TreeDir(path);
        }
        else if (keyword == "MoveDir") {
            string sourcePath, destinationPath;
            iss >> sourcePath >> destinationPath;
            MoveDir(sourcePath, destinationPath);
        }
        else if (keyword == "Create") {
            string fileName;
            iss >> fileName;
            Create(fileName);
        }
        else if (keyword == "Copy") {
            string sourceFileName, destinationFileName;
            iss >> sourceFileName >> destinationFileName;
            Copy(sourceFileName, destinationFileName);
        }
        else if (keyword == "Delete") {
            string fileName;
            iss >> fileName;
            Delete(fileName);
        }
        else if (keyword == "Write") {
            string fileName;
            iss >> fileName;
            Write(fileName);
        }
        else if (keyword == "Read") {
            string fileName;
            iss >> fileName;
            Read(fileName);
        }
        else if (keyword == "Export") {
            string fileName, externalDirectory;
            iss >> fileName >> externalDirectory;
            Export(fileName, externalDirectory);
        }
        else if (keyword == "exit") {
            cout << "再见！" << endl;
            break;
        }
        else {
            cout << "错误：无效的指令！" << endl;
        }
    }

    // 释放分区内存
    for (int i = 0; i < partitions.size(); i++) {
        delete partitions[i];
    }

    return 0;
}
