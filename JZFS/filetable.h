#ifndef FILETABLE_H
#define FILETABLE_H
#include "file.h"

class FileTable
{
public:
    FileTable();
    ~FileTable();
    File *TableFile[200];
    bool SaveEntryFile(File newEntryfile);

    int SizeFileTable();
    int nextFileId();



private:
    //int maxNumberOfFiles;

};

#endif // FILETABLE_H
