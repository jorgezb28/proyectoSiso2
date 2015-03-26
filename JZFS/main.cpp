#include <QCoreApplication>
#include <QDebug>
#include "device.h"
#include <iostream>
#include <fstream>
using namespace std;



bool LoadMetaDataDevice(string path,string nameDevice)
{

    string name = path+"\\"+nameDevice+".JZFS";
    PartitionMetadata *metaDevice=new PartitionMetadata();

    ifstream file;
    file.open(name.c_str(),ios::in | ios::binary);
    if (file.is_open())
    {
        //int pos=file.tellg();
        file.seekg (0, ios::beg);
        file.read ((char *)metaDevice, sizeof(PartitionMetadata));
        //pos=file.tellg();
        file.close();

        if(metaDevice->ValidPartition)
            return true;
        else
            return false;
    }
    return false;
}

Device LoadDevice(string path,string nombre)
{
    string name = path+"\\"+nombre+".JZFS";
    Device particion;

    PartitionMetadata *metaDevice=new PartitionMetadata();
    FileTable *fileTable=new FileTable();
    char * bitmap;

    ifstream file;
    try
    {
        file.open(name.c_str(),ios::in | ios::binary);
        if (file.is_open())
        {
            file.seekg (0, file.end);
            //int length = file.tellg();
            file.seekg (0, file.beg);

            int pos=file.tellg();

            file.seekg (0, ios::beg);
            file.read ((char *)metaDevice, sizeof(PartitionMetadata));

            pos=file.tellg();

            //double fileTableLenght= metaDevice->MaxNumberOfFile*sizeof(File);
            qDebug()<<file.tellg();
            for(int i=0;i<metaDevice->MaxNumberOfFile;i++){
                file.read((char *)fileTable->TableFile[i],sizeof(File));//fileTableLenght);
                qDebug()<<file.tellg();
            }

            pos=file.tellg();

            double bitMapLenght=ceil(metaDevice->TotalNumberOfBlockDevice/8);
            bitmap=new char[(int)bitMapLenght];
            file.read ((char *)bitmap, bitMapLenght);

            pos=file.tellg();
            file.close();

            particion._deviceMetaData=metaDevice;
            particion.FileTableDevice=fileTable;
            particion.BitMap=bitmap;
            particion.BitArraySizeInBytes=metaDevice->TotalNumberOfBlockDevice/8;

            return particion;
        }
        return particion;
    }catch(int ex)
    {
        qDebug() << "Error: "+ex;
        return particion;
    }
}

void SaveDeviceChanges(Device partition)
{
    string path=string(partition._deviceMetaData->CompletePathDevice);

    string name =  path+"\\"+partition._deviceMetaData->DeviceName+".JZFS";
    fstream outFile;
    outFile.open( name.c_str() , ios::out | ios::in | ios::binary);

    if(outFile.is_open())
    {
        outFile.seekp(0,ios::beg);

        double metaDataSize=sizeof(PartitionMetadata);        
        outFile.write((char *)partition._deviceMetaData,metaDataSize);

        for(int i=0;i<partition._deviceMetaData->MaxNumberOfFile;i++)
        {
            outFile.write((char *)partition.FileTableDevice->TableFile[i],sizeof(File));
        }

        double BitMapSize= partition.BitArraySizeInBytes;        
        outFile.write((char *)partition.BitMap,BitMapSize);

        outFile.flush();
        outFile.close();
    }
}

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    string path="E:\\SISO 2\\proyecto\\";
    string name="prueba";

    //if(!LoadMetaDataDevice(path,name))
    if(true)
    {
        Device *nuevo= new Device(10,"E:\\SISO 2\\proyecto\\","prueba");
        if(nuevo->CreateDevice())
        {
            printf("Device creado\n");
        }else{
            printf("Device no pudo se creado\n");
        }
    }
    Device Particion=LoadDevice(path,name);

    //Crear archivo
    if(Particion.createFile("archivo1",".txt","hola soy un arvhivo de prueba"))
        cout<<"\nArchivo Creado"<<endl;

    char *dataFile2=new char[15000];
    memset(dataFile2,'Z',15000);
    if(Particion.createFile("archivo2",".jrz", dataFile2))
        cout<<"\nArchivo creado"<<endl;

    if(Particion.createFile("archivo3",".txt","hola"))
        cout<<"\nArchivo Creado"<<endl;

    SaveDeviceChanges(Particion);

    system("pause");
}
