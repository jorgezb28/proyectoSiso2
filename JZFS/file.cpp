#include "file.h"
#include <stdio.h>
#include <cstring>

using namespace std;

File::File()
{
    this->_idFile=0;
    this->_dataBlock=0;
    memset(this->_nombre,0,30);
    memset(this->_tipo,0,5);
    this->_size=0;
}

File::File(int fileId,char nombre[30],char tipo[5],int size,int startdataBlock)
{    
    this->_idFile=fileId;
    strcpy (this->_nombre,nombre);
    strcpy(this->_tipo,tipo);    
    this->_size=size;
    this->_dataBlock=startdataBlock;
}
