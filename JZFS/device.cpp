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
            memset(this->BitMap,0,BitMapSize);
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
    if(!FileExist(nombre) && this->_deviceMetaData->ValidPartition)
    {
        int nextId=FileTableDevice->nextFileId();
        int sizeNewFile=strlen(data);
        int requiredBlock=  ceil((double)sizeNewFile/this->_deviceMetaData->BlockSizeKbs);

        if(nextId!=-1 && AvaibleFreeSpace(requiredBlock))
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


bool Device::FileExist(string nameFile)
{
    for(int i=0;i<200;i++)
    {
        File *currentFile=this->FileTableDevice->TableFile[i];
        string name=currentFile->_nombre;
        int compare = strcmp(name.c_str(),nameFile.c_str());
        if(compare==0)
            return true;
    }
    return false;
}

int Device::GetFileId(string fileName)
{
    for(int i=0;i<200;i++)
        {
            File *currentFile=this->FileTableDevice->TableFile[i];
            string name=currentFile->_nombre;
            int compare = strcmp(name.c_str(),fileName.c_str());
            if(compare==0)
                return i;
        }
        return -1;
}

void Device::ListarArchivos()
{
    for(int i=0;i<200;i++)
    {
        File *currentFile=this->FileTableDevice->TableFile[i];
        string nombre= string(currentFile->_nombre);
        if (!(nombre.empty() || nombre == " "))
            cout<<nombre+currentFile->_tipo<<endl;
    }
}

bool Device::ImportFile(char path[100], char nameFile[30], char type[5])
{
    string filepath=string(path);
    string fullPath=filepath+"\\"+nameFile+type;
    ifstream file;
    file.open(fullPath.c_str(),ios::in | ios::binary);
    if (file.is_open())
    {
        file.seekg(0,ios::end);
        int sizeFile=file.tellg();
        file.seekg(0,ios::beg);

        int bloquesRequeridos=sizeFile/4092;
        if(AvaibleFreeSpace(bloquesRequeridos) && !FileExist(nameFile))
        {
            ofstream outfile;
            string path=string(this->_deviceMetaData->CompletePathDevice);
            string name =  path+"\\"+this->_deviceMetaData->DeviceName+".JZFS";

            outfile.open(name.c_str(),ios::out | ios::in | ios::binary);
            if(outfile.is_open())
            {
                int fileId=FileTableDevice->nextFileId();
                if(fileId==-1)
                    return false;

                File *newFile= new File(fileId,nameFile,type,sizeFile,NextFreeBlock());
                this->FileTableDevice->TableFile[fileId]= newFile;

                int currenDataFilePos=0;
                for(int i=0;i<=bloquesRequeridos;i++)
                {
                    int nextDataBlock=NextFreeBlock();
                    ReserveBlocks(1);
                    outfile.seekp(nextDataBlock*4096);

                    if(sizeFile>4092)
                    {
                        DataBlock *data=new DataBlock();
                        file.read ((char *)data->DataFile, 4092);
                        //if(bloquesRequeridos>1){
                            data->NextDataBlock=NextFreeBlock();
                        //}
                        outfile.write((char *)data,sizeof(DataBlock));
                        currenDataFilePos+=4092;
                        sizeFile-=4092;

                    }else{
                        DataBlock *data=new DataBlock();
                        file.read ((char *)data->DataFile, sizeFile);

                        outfile.write((char *)data,sizeFile);
                        currenDataFilePos+=sizeFile;
                        sizeFile=0;
                    }

                }
                outfile.close();
                file.close();
                return true;
            }
            outfile.close();
            file.close();
            return false;
        }else
            return false;
    }
    if(file.is_open())
        file.close();
    return false;
}


bool Device::ExportFile(char destPath[], char fileName[], char type[])
{
    string filepathDest=string(destPath);
    string fullDestPath=filepathDest+"\\"+fileName+type;
    ofstream file;
    file.open(fullDestPath.c_str(),ios::out | ios::binary);
    if (file.is_open())
    {
        file.seekp(0,ios::beg);
        if(FileExist(fileName))
        {
            ifstream infile;
            string path=string(this->_deviceMetaData->CompletePathDevice);
            string name =  path+"\\"+this->_deviceMetaData->DeviceName+".JZFS";

            infile.open(name.c_str(), ios::in | ios::binary);
            if(infile.is_open())
            {
                int fileId=GetFileId(fileName);

                File *newFile= this->FileTableDevice->TableFile[fileId];

                int bloquesRequeridos=ceil((double)newFile->_size/4092);
                int nextDataBlock=newFile->_dataBlock;

               /* int posInFile=0;
                int pendingReadSize=newFile->_size;
                while(infile)
                {
                    infile.seekg(nextDataBlock*4096);

                    if(pendingReadSize<)
                    char *data=new char[4092];
                    infile.read ((char *)data, 4092);

                    posInFile+=4092;

                    char nextBlock[4];
                    infile.read((char *)nextBlock,4);
                    nextDataBlock= atoi(nextBlock);

                    file.write((char *)data,sizeof(4092));
                }*/

                int posInFile=0;
                int sizeFile=newFile->_size;
                for(int i=0;i<bloquesRequeridos;i++)
                {

                    if(sizeFile>4092)
                    {
                        infile.seekg(nextDataBlock*4096);
                        char data[4092];
                        infile.read ((char *)data, 4092);
                        infile.read(reinterpret_cast<char*>(&nextDataBlock), sizeof(nextDataBlock));
                        file.write((char *)data,4092);

                        posInFile+=4092;
                        sizeFile-=4092;

                    }else{
                        infile.seekg(nextDataBlock*4096);
                        char data[sizeFile];
                        infile.read ((char *)data, sizeFile);
                        file.write((char *)data,sizeFile);
                        posInFile+=sizeFile;
                        sizeFile=0;
                    }
                }
                infile.close();
                file.close();
                return true;
            }
            infile.close();
            file.close();
            return false;
        }
        file.close();
        return false;
    }
    if(file.is_open())
        file.close();
    return false;

}

bool Device::AvaibleFreeSpace(int totalRequiredBlocks)
{
    for(int i=0;i<this->BitArraySizeInBytes;i++)
    {
        bitset<8> currentBits(this->BitMap[i]);

        for(int k=0;k<8;k++)
        {
            if(!currentBits.test(k))
            {
                totalRequiredBlocks--;
                if(totalRequiredBlocks<=0)
                {
                    return true;
                }
            }
        }
    }
    return false;
}

bool Device::RenameFile(char currentFileName[30],char newName[30])
{
    for(int i=0;i<200;i++)
    {
        File *currentFile=this->FileTableDevice->TableFile[i];
        string name=currentFile->_nombre;
        int compare = strcmp(name.c_str(),currentFileName);
        if(compare==0)
        {
            memccpy(currentFile->_nombre,newName,0,30);
            this->FileTableDevice->TableFile[i]=currentFile;
            return true;
        }
    }
    return false;
}
