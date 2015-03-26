#include "partitionmetadata.h"
#include <stdio.h>
#include <cstring>

PartitionMetadata::PartitionMetadata()
{
    this->ValidPartition=false;
    this->SizeDeviceMbs=0;
    this->BlockSizeKbs=4096;
    memset(this->CompletePathDevice,0,255);
    this->TotalNumberOfBlockDevice=0;
    memset(this->DeviceName,0,100);
    this->MaxNumberOfFile=200;
}

PartitionMetadata::PartitionMetadata(int sizeDeviceMbs,char pathDevice[255],char deviceName[100])
{
    this->ValidPartition=false;
    this->SizeDeviceMbs=sizeDeviceMbs;
    this->BlockSizeKbs=4096;
    memset(this->CompletePathDevice,0,255);
    strcpy(this->CompletePathDevice,pathDevice);
    this->TotalNumberOfBlockDevice=convertMbsToBytes(sizeDeviceMbs)/this->BlockSizeKbs;
    memset(this->DeviceName,0,100);
    strcpy(this->DeviceName,deviceName);
    this->MaxNumberOfFile=200;
}

PartitionMetadata::~PartitionMetadata()
{

}

int PartitionMetadata::convertMbsToBytes(int sizeMbs)
{
    return sizeMbs*1024*1024;

}

