#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <iomanip>

using namespace std;

#define MAX_FILE_SIZE 1024
#define MSD 50  // �����Ŀ¼��
#define DISK_SIZE 67108864  // ���̴�С���ֽ�����
#define MAX_DISK_PARTITIONS 5  // �����̷�����

// �ļ�/Ŀ¼��Ľṹ
struct DirItem {
    string fileName;  // �ļ�/Ŀ¼����
    int type;  // 0 ��ʾ�ļ���1 ��ʾĿ¼
    int next;  // �ļ�/Ŀ¼����һ����Ĵ��̵�ַ
    int size;  // �ļ�/Ŀ¼�Ĵ�С���ֽ�����
    string updateTime;  // �ļ�/Ŀ¼��������ʱ��
};

// Ŀ¼�Ľṹ
struct Directory {
    DirItem directItem[MSD + 2];  // Ŀ¼������
};

// ���̷����Ľṹ
struct DiskPartition {
    string partitionName;  // �������ƣ����� A#��B#��C#��
    int partitionSize;  // ������С���� MB Ϊ��λ��
    string filePath;  // �����ļ���·��
    Directory rootDirectory;  // �����ĸ�Ŀ¼
    int bitmap[DISK_SIZE / MAX_FILE_SIZE];  // ���ڸ��ٿ���/�ѷ���Ŀ�
};
struct VirtualDisk {
    string filePath;
    char driveLetter;
    int bitmap[DISK_SIZE / MAX_FILE_SIZE];
    Directory rootDirectory;
};
vector<DiskPartition> diskPartitions;  // �洢���̷���������
DiskPartition* currentPartition;  // ��ǰ������ָ��
vector<VirtualDisk*> partitions;// �����б�
Directory* currentDirectory;  // ��ǰĿ¼
char nextDriveLetter = 'A';  // ��һ�����õ��̷�
string userName = "test";
string userDat;  // �û��Ĵ��������ļ�

void strip(string& str, char ch) {
    str.erase(remove(str.begin(), str.end(), ch), str.end());
}

void init(string fileName) {
    fstream fp;
    fp.open(fileName, ios::out | ios::binary);
    if (fp) {
        // �����̿ռ��ʼ��Ϊ 0
        char* buffer = new char[DISK_SIZE];
        memset(buffer, 0, DISK_SIZE);
        fp.write(buffer, DISK_SIZE);
        delete[] buffer;

        // ��ʼ�����̷���
        DiskPartition partition;
        partition.partitionName = "A#";
        partition.partitionSize = 0;
        partition.filePath = fileName;
        diskPartitions.push_back(partition);

        currentPartition = &diskPartitions[0];

        fp.close();
        cout << "���̳�ʼ���ɹ���" << endl;
    }
    else {
        cout << "�޷��������������ļ���" << endl;
    }
}

void readUserDisk(string fileName) {
    fstream fp;
    fp.open(fileName, ios::in | ios::binary);
    if (fp) {
        // ��ȡ���̷�����Ϣ
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
        // ������̷�����Ϣ
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

// ��ʼ�����̷���
void InitDisk(string partitionSizes) {
    istringstream iss(partitionSizes);
    string partitionSize;
    while (getline(iss, partitionSize, ',')) {
        strip(partitionSize, ' ');
        strip(partitionSize, 'M');

        int size = stoi(partitionSize);
        if (size <= 0) {
            cout << "���󣺷�����С��������㣡" << endl;
            return;
        }

        DiskPartition partition;
        partition.partitionName = string(1, 'A' + diskPartitions.size()) + "#";
        partition.partitionSize = size;
        partition.filePath = userDat;
        diskPartitions.push_back(partition);
    }

    currentPartition = &diskPartitions[0];
    cout << "������ʼ���ɹ���" << endl;
}

// �л���ǰ����
//void ChgDisk(string partitionName) {
//    for (int i = 0; i < diskPartitions.size(); i++) {
//        if (diskPartitions[i].partitionName == partitionName) {
//            currentPartition = &diskPartitions[i];
//            cout << "��ǰ�����л�Ϊ��" << partitionName << endl;
//            return;
//        }
//    }
//
//    cout << "����ָ���ķ��������ڣ�" << endl;
//}
//
//// ��ʾ�����Ŀռ�ͳ�����
//void ShowDisk(string partitionName) {
//    for (int i = 0; i < diskPartitions.size(); i++) {
//        if (diskPartitions[i].partitionName == partitionName) {
//            int totalSize = diskPartitions[i].partitionSize * 1024 * 1024;
//            int usedSize = totalSize - (currentPartition->bitmap[0] * MAX_FILE_SIZE);
//            int availableSize = currentPartition->bitmap[0] * MAX_FILE_SIZE;
//
//            cout << "������" << partitionName << endl;
//            cout << "�ܿռ��С��" << totalSize << " �ֽ�" << endl;
//            cout << "���ÿռ��С��" << usedSize << " �ֽ�" << endl;
//            cout << "���ÿռ��С��" << availableSize << " �ֽ�" << endl;
//            return;
//        }
//    }
//
//    cout << "����ָ���ķ��������ڣ�" << endl;
//}

// ����ָ��Ŀ¼
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
            cout << "�����޷�����Ŀ¼��Ŀ¼��������" << endl;
            return;
        }

        if (directoryExists) {
            currentDirectory = &currentPartition->rootDirectory;
            currentDirectory = reinterpret_cast<Directory*>(&currentPartition->rootDirectory.directItem[directoryIndex]);
        }
    }

    cout << "Ŀ¼�����ɹ���" << endl;
}

// ɾ����Ŀ¼
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
            cout << "����ָ����Ŀ¼�����ڣ�" << endl;
            return;
        }

        currentDirectory = &currentPartition->rootDirectory;
        currentDirectory = reinterpret_cast<Directory*>(&currentPartition->rootDirectory.directItem[directoryIndex]);
    }

    // ���Ŀ¼�Ƿ�Ϊ��
    for (int i = 0; i < MSD + 2; i++) {
        if (!currentDirectory->directItem[i].fileName.empty()) {
            cout << "����Ŀ¼�ǿգ��޷�ɾ����" << endl;
            return;
        }
    }

    currentDirectory->directItem[0].fileName = "";
    currentDirectory->directItem[0].type = 0;
    currentDirectory->directItem[0].next = -1;
    currentDirectory->directItem[0].size = 0;
    currentDirectory->directItem[0].updateTime = "";
}

// ��ʾĿ¼����
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
                cout << "����ָ����Ŀ¼�����ڣ�" << endl;
                return;
            }

            tempDirectory = reinterpret_cast<Directory*>(&tempDirectory->directItem[directoryIndex]);
        }

        currentDirectory = tempDirectory;
    }

    cout << "Ŀ¼��" << path << endl;
    cout << setw(20) << left << "����" << setw(20) << left << "����" << setw(20) << left << "��С" << setw(20) << left << "����ʱ��" << endl;

    for (int i = 1; i < MSD + 2; i++) {
        if (!currentDirectory->directItem[i].fileName.empty()) {
            cout << setw(20) << left << currentDirectory->directItem[i].fileName;
            cout << setw(20) << left << (currentDirectory->directItem[i].type == 1 ? "Ŀ¼" : "�ļ�");
            cout << setw(20) << left << currentDirectory->directItem[i].size;
            cout << setw(20) << left << currentDirectory->directItem[i].updateTime << endl;
        }
    }
}

// �л���ǰĿ¼
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
            cout << "����ָ����Ŀ¼�����ڣ�" << endl;
            return;
        }

        tempDirectory = reinterpret_cast<Directory*>(&tempDirectory->directItem[directoryIndex]);
    }

    currentDirectory = tempDirectory;
}

// ��״��ʾĿ¼�ṹ
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
                cout << "����ָ����Ŀ¼�����ڣ�" << endl;
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

// ��Ŀ¼Ǩ�Ƶ���һ��Ŀ¼��
void MoveDir(string sourcePath, string destinationPath) {
    strip(sourcePath, '\\');
    strip(sourcePath, '/');
    strip(destinationPath, '\\');
    strip(destinationPath, '/');

    if (sourcePath == destinationPath) {
        cout << "����ԴĿ¼��Ŀ��Ŀ¼��ͬ��" << endl;
        return;
    }

    if (sourcePath.empty() || destinationPath.empty()) {
        cout << "����ԴĿ¼��Ŀ��Ŀ¼����Ϊ�գ�" << endl;
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
            cout << "����ԴĿ¼�����ڣ�" << endl;
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
            cout << "����Ŀ��Ŀ¼�����ڣ�" << endl;
            return;
        }

        destinationDirectory = reinterpret_cast<Directory*>(&destinationDirectory->directItem[destinationDirectoryIndex]);
    }

    // ���Ŀ��Ŀ¼�Ƿ��Ѵ���ͬ����ԴĿ¼
    for (int i = 0; i < MSD + 2; i++) {
        if (destinationDirectory->directItem[i].fileName == sourceDirectoryName && destinationDirectory->directItem[i].type == 1) {
            cout << "����Ŀ��Ŀ¼���Ѵ���ͬ����ԴĿ¼��" << endl;
            return;
        }
    }

    // ����ԴĿ¼
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

    // ɾ��ԴĿ¼
    sourceDirectory->directItem[0].fileName = "";
    sourceDirectory->directItem[0].type = 0;
    sourceDirectory->directItem[0].next = -1;
    sourceDirectory->directItem[0].size = 0;
    sourceDirectory->directItem[0].updateTime = "";
}

// �����ļ�
void Create(string fileName) {
    strip(fileName, '\\');
    strip(fileName, '/');

    if (fileName.empty()) {
        cout << "�����ļ�������Ϊ�գ�" << endl;
        return;
    }

    for (int i = 1; i < MSD + 2; i++) {
        if (currentDirectory->directItem[i].fileName == fileName && currentDirectory->directItem[i].type == 0) {
            cout << "����ͬ���ļ��Ѵ��ڣ�" << endl;
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
        cout << "�����޷������ļ���Ŀ¼��������" << endl;
        return;
    }

    // ���´��̿��п�
    int blockCount = (size + sizeof(DirItem) - 1) / sizeof(DirItem);
    for (int i = 0; i < blockCount; i++) {
        for (int j = 0; j < DISK_SIZE / MAX_FILE_SIZE; j++) {
            if (currentPartition->bitmap[j] == 0) {
                currentPartition->bitmap[j] = i + 1;
                break;
            }
        }
    }

    cout << "�ļ������ɹ���" << endl;
}

// �����ļ�
void Copy(string sourceFileName, string destinationFileName) {
    strip(sourceFileName, '\\');
    strip(sourceFileName, '/');
    strip(destinationFileName, '\\');
    strip(destinationFileName, '/');

    if (sourceFileName.empty() || destinationFileName.empty()) {
        cout << "����Դ�ļ���Ŀ���ļ�����Ϊ�գ�" << endl;
        return;
    }

    if (sourceFileName == destinationFileName) {
        cout << "����Դ�ļ���Ŀ���ļ���ͬ��" << endl;
        return;
    }

    for (int i = 1; i < MSD + 2; i++) {
        if (currentDirectory->directItem[i].fileName == sourceFileName && currentDirectory->directItem[i].type == 0) {
            for (int j = 1; j < MSD + 2; j++) {
                if (currentDirectory->directItem[j].fileName == destinationFileName && currentDirectory->directItem[j].type == 0) {
                    cout << "����Ŀ��Ŀ¼���Ѵ���ͬ���ļ���" << endl;
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
                cout << "�����޷������ļ������̿ռ�������" << endl;
                return;
            }

            for (int j = 0; j < DISK_SIZE / MAX_FILE_SIZE; j++) {
                if (currentPartition->bitmap[j] == 0) {
                    destinationIndex = j;
                    break;
                }
            }

            if (destinationIndex == -1) {
                cout << "�����޷������ļ������̿ռ�������" << endl;
                return;
            }

            // �����ļ�
            currentPartition->bitmap[sourceIndex] = currentPartition->bitmap[0];
            currentPartition->bitmap[0] = 0;

            currentPartition->bitmap[destinationIndex] = currentPartition->bitmap[0];
            currentPartition->bitmap[0] = sourceIndex + 1;

            currentDirectory->directItem[i].fileName = destinationFileName;
            currentDirectory->directItem[i].updateTime = __TIMESTAMP__;

            cout << "�ļ����Ƴɹ���" << endl;
            return;
        }
    }

    cout << "����ָ����Դ�ļ������ڣ�" << endl;
}

// ɾ���ļ�
void Delete(string fileName) {
    strip(fileName, '\\');
    strip(fileName, '/');

    if (fileName.empty()) {
        cout << "�����ļ�������Ϊ�գ�" << endl;
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
                cout << "�����޷�ɾ���ļ������̿ռ�������" << endl;
                return;
            }

            // ɾ���ļ�
            currentPartition->bitmap[index] = currentPartition->bitmap[0];
            currentPartition->bitmap[0] = size / sizeof(DirItem);

            currentDirectory->directItem[i].fileName = "";
            currentDirectory->directItem[i].type = 0;
            currentDirectory->directItem[i].next = -1;
            currentDirectory->directItem[i].size = 0;
            currentDirectory->directItem[i].updateTime = "";

            cout << "�ļ�ɾ���ɹ���" << endl;
            return;
        }
    }

    cout << "����ָ�����ļ������ڣ�" << endl;
}

// ���ļ���֧��д�뱣��
void Write(string fileName) {
    strip(fileName, '\\');
    strip(fileName, '/');

    if (fileName.empty()) {
        cout << "�����ļ�������Ϊ�գ�" << endl;
        return;
    }

    for (int i = 1; i < MSD + 2; i++) {
        if (currentDirectory->directItem[i].fileName == fileName && currentDirectory->directItem[i].type == 0) {
            cout << "�������ļ����ݣ���EOF��������" << endl;

            string content;
            string line;
            while (getline(cin, line) && line != "EOF") {
                content += line + "\n";
            }

            if (content.empty()) {
                cout << "�����ļ����ݲ���Ϊ�գ�" << endl;
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
                cout << "�����޷�д���ļ������̿ռ�������" << endl;
                return;
            }

            // �����ļ���С
            currentDirectory->directItem[i].size = size;
            currentDirectory->directItem[i].updateTime = __TIMESTAMP__;

            // д���ļ�����
            ofstream outFile;
            outFile.open(currentPartition->filePath, ios::binary | ios::in | ios::out);
            if (outFile) {
                int offset = (index - 1) * MAX_FILE_SIZE;
                outFile.seekp(offset, ios::beg);
                outFile.write(reinterpret_cast<const char*>(&currentDirectory->directItem[i]), sizeof(DirItem));
                outFile.write(content.c_str(), content.size());
                outFile.close();

                cout << "�ļ�д��ɹ���" << endl;
            }
            else {
                cout << "�����޷����ļ���" << endl;
            }

            return;
        }
    }

    cout << "����ָ�����ļ������ڣ�" << endl;
}

// ���ļ��������ʾ
void Read(string fileName) {
    strip(fileName, '\\');
    strip(fileName, '/');

    if (fileName.empty()) {
        cout << "�����ļ�������Ϊ�գ�" << endl;
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
                cout << "�����ļ�����Ϊ�գ�" << endl;
                return;
            }

            // ��ȡ�ļ�����
            ifstream inFile;
            inFile.open(currentPartition->filePath, ios::binary | ios::in | ios::out);
            if (inFile) {
                int offset = index * MAX_FILE_SIZE;
                inFile.seekg(offset, ios::beg);

                DirItem item;
                inFile.read(reinterpret_cast<char*>(&item), sizeof(DirItem));

                char* content = new char[size];
                inFile.read(content, size);

                cout << "�ļ����ݣ�" << endl;
                cout.write(content, size);
                cout << endl;

                inFile.close();
            }
            else {
                cout << "�����޷����ļ���" << endl;
            }

            return;
        }
    }

    cout << "����ָ�����ļ������ڣ�" << endl;
}

// �����ļ����ⲿӲ��
void Export(string fileName, string externalDirectory) {
    strip(fileName, '\\');
    strip(fileName, '/');
    strip(externalDirectory, '\\');
    strip(externalDirectory, '/');

    if (fileName.empty() || externalDirectory.empty()) {
        cout << "�����ļ������ⲿĿ¼����Ϊ�գ�" << endl;
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
                cout << "�����ļ�����Ϊ�գ�" << endl;
                return;
            }

            // ��ȡ�ļ�����
            ifstream inFile;
            inFile.open(currentPartition->filePath, ios::binary | ios::in | ios::out);
            if (inFile) {
                int offset = index * MAX_FILE_SIZE;
                inFile.seekg(offset, ios::beg);

                DirItem item;
                inFile.read(reinterpret_cast<char*>(&item), sizeof(DirItem));

                char* content = new char[size];
                inFile.read(content, size);

                // �����ļ����ⲿӲ��
                string externalFilePath = externalDirectory + "\\" + fileName;
                ofstream outFile;
                outFile.open(externalFilePath, ios::binary);
                if (outFile) {
                    outFile.write(content, size);
                    outFile.close();

                    cout << "�ļ������ɹ���" << endl;
                }
                else {
                    cout << "�����޷����ⲿĿ¼��" << endl;
                }

                inFile.close();
            }
            else {
                cout << "�����޷����ļ���" << endl;
            }

            return;
        }
    }

    cout << "����ָ�����ļ������ڣ�" << endl;
}

// ��ʼ��������̺ͷ���
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
//        // ��ʼ��λʾͼ
//        for (int i = 0; i < DISK_SIZE / MAX_FILE_SIZE; i++) {
//            newPartition->bitmap[i] = 0;
//        }
//
//        // ��ʼ����Ŀ¼
//        newPartition->rootDirectory.directItem[0].fileName = "";
//        newPartition->rootDirectory.directItem[0].type = 1;
//        newPartition->rootDirectory.directItem[0].next = -1;
//        newPartition->rootDirectory.directItem[0].size = 0;
//        newPartition->rootDirectory.directItem[0].updateTime = "";
//
//        // ��ʼ��������С
//        int size = stoi(partitionSize.substr(0, partitionSize.size() - 1));
//        newPartition->rootDirectory.directItem[0].size = size / sizeof(DirItem);
//
//        // ���´��̿��п�
//        int blockCount = (size + sizeof(DirItem) - 1) / sizeof(DirItem);
//        for (int i = 0; i < blockCount; i++) {
//            newPartition->bitmap[i] = i + 1;
//        }
//
//        // ���·�����������б�
//        partitions.push_back(newPartition);
//
//        cout << "���� " << partitionName << " ��ʼ���ɹ�����СΪ " << partitionSize << "��" << endl;
//    }
//
//    currentPartition = &diskPartitions[0];
//    currentDirectory = &currentPartition->rootDirectory;
//}

// �л���ǰ����
void ChgDisk(char driveLetter) {
    for (int i = 0; i < partitions.size(); i++) {
        if (partitions[i]->driveLetter == driveLetter) {
            currentPartition = &diskPartitions[0];
            currentDirectory = &currentPartition->rootDirectory;
            return;
        }
    }

    cout << "����ָ���ķ��������ڣ�" << endl;
}

// ��ʾ�����ռ�ͳ�����
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

            cout << "���� " << partitions[i]->driveLetter << "# �Ŀռ�ͳ�������" << endl;
            cout << "�ܿռ��С��" << totalSize << "B" << endl;
            cout << "���ÿռ��С��" << usedSize << "B" << endl;
            cout << "���ÿռ��С��" << availableSize << "B" << endl;

            return;
        }
    }

    cout << "����ָ���ķ��������ڣ�" << endl;
}
void help()
{
    cout << fixed << left;
    cout << "******************** ���� ********************" << endl << endl;
    cout << setw(40) << "InitDisk A xxM, B yyM, C zzM,..." << setw(10) << "��ʼ�����̷���" << endl;
    cout << setw(40) << "ChgDisk ����" << setw(10) << "�л���ǰ����" << endl;
    cout << setw(40) << "ShowDisk ����" << setw(10) << "��ʾ�����ռ�ͳ�����" << endl;
    cout << setw(40) << "MkDir Ŀ¼��" << setw(10) << "����Ŀ¼" << endl;
    cout << setw(40) << "DelDir Ŀ¼��" << setw(10) << "ɾ����Ŀ¼" << endl;
    cout << setw(40) << "Dir Ŀ¼��" << setw(10) << "��ʾĿ¼����" << endl;
    cout << setw(40) << "ChgDir Ŀ¼��" << setw(10) << "�л���ǰĿ¼" << endl;
    cout << setw(40) << "TreeDir Ŀ¼��" << setw(10) << "��״��ʾĿ¼�ṹ" << endl;
    cout << setw(40) << "MoveDir Ŀ¼1 Ŀ¼2" << setw(10) << "�ƶ�Ŀ¼" << endl;
    cout << setw(40) << "Create �ļ���" << setw(10) << "�����ļ�" << endl;
    cout << setw(40) << "Copy Դ�ļ� Ŀ���ļ�" << setw(10) << "�����ļ�" << endl;
    cout << setw(40) << "Delete �ļ���" << setw(10) << "ɾ���ļ�" << endl;
    cout << setw(40) << "Write �ļ���" << setw(10) << "�򿪲�д���ļ�" << endl;
    cout << setw(40) << "Read �ļ���" << setw(10) << "�򿪲���ʾ�ļ�����" << endl;
    cout << setw(40) << "Export �ļ��� Ŀ¼��" << setw(10) << "���ļ��������ⲿӲ��" << endl;
    cout << setw(40) << "Exit" << setw(10) << "�˳�" << endl << endl;
}
// ������
int main() {
    cout << "��ӭʹ���������ϵͳ��" << endl;
    cout << "������ָ�����help��ȡ��������" << endl;

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
            cout << "�ټ���" << endl;
            break;
        }
        else {
            cout << "������Ч��ָ�" << endl;
        }
    }

    // �ͷŷ����ڴ�
    for (int i = 0; i < partitions.size(); i++) {
        delete partitions[i];
    }

    return 0;
}
