#ifndef FILE_H
#define FILE_H
#include<string>
#include <stdio.h>
using namespace std;

//FCB(File Control Block)
class File
{
public:
    File();
    File(int fileId, char nombre[30], char tipo[5], int size, int startdataBlock);
    ~File();
    char _nombre[30];
    int _idFile;
    char _tipo[5];
    int _size;
    int _dataBlock;


};

#endif // FILE_H
