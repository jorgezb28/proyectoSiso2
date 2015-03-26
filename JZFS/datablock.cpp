#include "datablock.h"

DataBlock::DataBlock()
{
    memset(this->DataFile,0,4092);
    this->NextDataBlock=-1;
}

