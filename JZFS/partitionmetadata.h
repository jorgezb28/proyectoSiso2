#ifndef PARTITIONMETADATA_H
#define PARTITIONMETADATA_H
#include <string>
#include <filetable.h>
#include <math.h>
#include <bitset>
using namespace std;

//Seria la Volume Control Block
class PartitionMetadata
{
public:
    PartitionMetadata();
    PartitionMetadata(int sizeDeviceMbs,char pathDevice[255],char deviceName[100]);
    ~PartitionMetadata();

    bool ValidPartition;
    int BlockSizeKbs ;
    int TotalNumberOfBlockDevice;
    int SizeDeviceMbs;
    char CompletePathDevice[255];
    char DeviceName[100];
    int MaxNumberOfFile;

    int convertMbsToBytes(int sizeMbs);

};

#endif // PARTITIONMETADATA_H
