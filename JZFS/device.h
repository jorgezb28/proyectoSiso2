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
    bool FileExist(string nameFile);
    void ListarArchivos();
    bool ImportFile(char path[100], char name[30], char type[5]);
    bool ExportFile(char destPath[100], char fileName[30], char type[5]);
    bool AvaibleFreeSpace(int totalRequiredBlocks);
    bool RenameFile(char currentFileName[30],char newName[30]);

private:
    //metodos
    bool FortmatDevice();
    bool ReserveBlocks(int cantidadBloques);
    int NextFreeBlock();
    int GetFileId(string fileName);

};

#endif // DEVICE_H
