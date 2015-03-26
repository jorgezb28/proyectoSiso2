#include "filetable.h"

FileTable::FileTable()
{
    //this->maxNumberOfFiles=200;
    //this->TableFile=new File[maxNumberOfFiles];
    for(int i=0;i<200;i++)
    {
        TableFile[i]=new File();
        //TableFile[i]._idFile=0;

    }
}


FileTable::~FileTable()
{
    delete TableFile;
}

int FileTable::SizeFileTable()
{
    int size=0;
    size+=200*sizeof(File);
    return size;
}

int FileTable::nextFileId()
{
    for(int i=0;i<200;i++)
    {
        if(TableFile[i]->_size==0)
        {
            return i;
        }
    }
    return -1;
}
