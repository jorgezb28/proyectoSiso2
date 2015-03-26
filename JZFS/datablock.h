#ifndef DATABLOCK_H
#define DATABLOCK_H
#include <stdio.h>
#include <string.h>

class DataBlock
{
public:
    DataBlock();
    ~DataBlock();


    char DataFile[4092];
    int NextDataBlock;


};

#endif // DATABLOCK_H
