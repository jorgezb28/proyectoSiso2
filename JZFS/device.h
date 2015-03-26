#ifndef DEVICE_H
#define DEVICE_H
#include <string>
#include <fstream>
#include <iostream>
#include <stdio.h>
#include <partitionmetadata.h>
#include <datablock.h>

using namespace std;

class Device
{
public:
    Device();
    Device(int sizeDeviceMbs, string pathDevice ,string nameDevice);
    ~Device();

    PartitionMetadata *_deviceMetaData;
    int BitArraySizeInBytes;
    char * BitMap;
    FileTable *FileTableDevice;


    bool CreateDevice();    
    bool createFile(char nombre[30], char tipo[5],char * data);    

private:
    //propiedades

    //ifstream inFile;



    //metodos
    bool FortmatDevice();
    bool ReserveBlocks(int cantidadBloques);
    int NextFreeBlock();
};

#endif // DEVICE_H
