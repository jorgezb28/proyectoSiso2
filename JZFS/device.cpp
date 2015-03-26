#include "device.h"
#include <QFile>
#include <QDataStream>
#include <QDebug>

Device::Device()
{
    this->_deviceMetaData= new PartitionMetadata();
    this->BitArraySizeInBytes=0;
    this->FileTableDevice=new FileTable();
}

Device::Device(int sizeDeviceMbs, string pathDevice ,string nameDevice)
{    
    char path[255];
    char name[100];
    memset(path,0,255);
    memset(name,0,100);
    strcpy(path,pathDevice.c_str());
    strcpy(name,nameDevice.c_str());


    this->_deviceMetaData = new PartitionMetadata(sizeDeviceMbs,path,name);

    this->BitArraySizeInBytes= ceil(this->_deviceMetaData->TotalNumberOfBlockDevice/8);
    this->BitMap=new char[this->BitArraySizeInBytes];
    memset(this->BitMap,0,this->BitArraySizeInBytes);

    this->FileTableDevice=new FileTable();

}

Device::~Device()
{
    //borrar apuntadores
    delete this->_deviceMetaData;
    delete this->BitMap;
    delete this->FileTableDevice;
}

bool Device::CreateDevice()
{
    FortmatDevice();
    return true;

}

bool Device::FortmatDevice()
{
    try
    {
        string completePathStr= string(this->_deviceMetaData->CompletePathDevice);
        string name = completePathStr +"\\"+this->_deviceMetaData->DeviceName+".JZFS";

        ofstream outFile;
        outFile.open( name.c_str() , ios::out | ios::binary);

        if(outFile.is_open())
        {
            //int pos=outFile.tellp();

            outFile.seekp(0);
            double metaDataSize=sizeof(PartitionMetadata);
            int bloquesRequeridosMetadata=ceil( metaDataSize/_deviceMetaData->BlockSizeKbs);
            this->ReserveBlocks(bloquesRequeridosMetadata);
            this->_deviceMetaData->ValidPartition=true;
            outFile.write((char *)this->_deviceMetaData,metaDataSize);

            //outFile.seekp(metaDataSize);
            //pos=outFile.tellp();
            double tableFileSize=this->FileTableDevice->SizeFileTable();
            int bloquesRequeridosTableFile=ceil( tableFileSize/this->_deviceMetaData->BlockSizeKbs);
            this->ReserveBlocks(bloquesRequeridosTableFile);
            for(int i=0;i<this->_deviceMetaData->MaxNumberOfFile;i++)
            {
                outFile.write((char *)this->FileTableDevice->TableFile[i],sizeof(File));
            }

            double BitMapSize= this->BitArraySizeInBytes;
            int bloquesRequeridosBitMap=ceil( BitMapSize/this->_deviceMetaData->BlockSizeKbs);
            this->ReserveBlocks(bloquesRequeridosBitMap);
            outFile.write((char *)this->BitMap,BitMapSize);

            int bloquesIniciales=bloquesRequeridosMetadata+bloquesRequeridosTableFile+bloquesRequeridosBitMap;

            for(int i=0;i<this->_deviceMetaData->TotalNumberOfBlockDevice-bloquesIniciales;i++)
            {
                DataBlock *emptyDataBlock=new DataBlock();
                outFile.write((char *)emptyDataBlock,sizeof(DataBlock));
            }

            outFile.flush();
            outFile.close();            
            return true;
        }else{
            return false;
        }
    }catch(int ex)
    {
        printf("Error creating device.Error code: "+ex);
        return false;
    }
}

bool Device::ReserveBlocks(int cantidadBloques) //pendiente de probar bien
{
    for(int i=0;i<this->BitArraySizeInBytes;i++)
    {
        bitset<8> currentBits(this->BitMap[i]);

        for(int k=0;k<8;k++)
        {
            if(!currentBits.test(k))
            {
                if(cantidadBloques>0)
                {
                    currentBits[k]=1;
                    cantidadBloques--;
                    continue;
                }else{
                    this->BitMap[i]=(char)currentBits.to_ulong();
                    return true;
                }
            }
        }
        this->BitMap[i]=(char)currentBits.to_ulong();
    }
    return false;
}

bool Device::createFile(char nombre[30], char tipo[5],char * data)
{
    int nextId=FileTableDevice->nextFileId();
    int sizeNewFile=strlen(data);
    int requiredBlock=  ceil((double)sizeNewFile/this->_deviceMetaData->BlockSizeKbs);

    if(nextId!=-1)
    {
        File *newFile= new File(nextId,nombre,tipo,sizeNewFile,NextFreeBlock());
        if(ReserveBlocks(1))
        {
            //crear entrada en file table
            this->FileTableDevice->TableFile[nextId]= newFile;

            //obtine path del archivo
            string completePathStr= string(this->_deviceMetaData->CompletePathDevice);
            string name = completePathStr +"\\"+this->_deviceMetaData->DeviceName+".JZFS";

            ofstream outFile;
            outFile.open( name.c_str() ,ios::out | ios::in | ios::binary);
            if(outFile.is_open())
            {
                //posiciona puntero en el bloque inicial de la data del nuevo archivo
                outFile.seekp(newFile->_dataBlock*this->_deviceMetaData->BlockSizeKbs);
                //cout<<("\nSeek blqoue data:"+outFile.tellp())<<endl;

                //Creaa los bloques de data requeridos con su data correspondiente
                int currenDataFilePos=0;
                for(int i=requiredBlock;i>0;i--)
                {
                    DataBlock *newDataBlock=new DataBlock();
                    if(sizeNewFile>4092)
                    {
                        memccpy(newDataBlock->DataFile,data,currenDataFilePos,4092);
                        if(ReserveBlocks(1))
                            newDataBlock->NextDataBlock=NextFreeBlock();
                        outFile.write((char *)newDataBlock,sizeof(DataBlock));
                        outFile.flush();
                        //printf("Escritura blqoue:"+outFile.tellp());

                        currenDataFilePos+=4092;
                        sizeNewFile-=4092;
                    }else
                    {
                        memccpy(newDataBlock->DataFile,data,currenDataFilePos,sizeNewFile);
                        outFile.write((char *)newDataBlock,sizeof(DataBlock));
                        outFile.flush();
                        //printf("Escritura blqoue:"+outFile.tellp());

                        currenDataFilePos+=sizeNewFile;
                        sizeNewFile=0;
                    }
                }
                outFile.close();
                return true;
            }
        }else
        {
            return false;
        }
    }else
    {
        return false;
    }
    return false;
}

int Device::NextFreeBlock()
{
    for(int i=0;i<this->BitArraySizeInBytes;i++)
    {
        bitset<8> currentBits(this->BitMap[i]);

        for(int k=0;k<8;k++)
        {
            if(!currentBits.test(k))
            {
                int nextBlock=(i*8)+k;
                return nextBlock;
            }
        }
    }
    return -1;
}

