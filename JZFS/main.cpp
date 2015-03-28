#include <QCoreApplication>
#include <QDebug>
#include "device.h"
#include <iostream>
#include <fstream>
#include <sstream>

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

        if(metaDevice->ValidPartition){
            //delete metaDevice;
            return true;
        }else{
            //delete metaDevice;
            return false;
        }

    }
    return false;
}

Device *LoadDevice(string path,string nombre)
{
    string name = path+"\\"+nombre+".JZFS";
    Device *particion = new Device();

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

            //int pos=file.tellg();

            file.seekg (0, ios::beg);
            file.read ((char *)metaDevice, sizeof(PartitionMetadata));

            //pos=file.tellg();

            //double fileTableLenght= metaDevice->MaxNumberOfFile*sizeof(File);
            //qDebug()<<file.tellg();
            for(int i=0;i<metaDevice->MaxNumberOfFile;i++){
                file.read((char *)fileTable->TableFile[i],sizeof(File));//fileTableLenght);
                //qDebug()<<file.tellg();
            }

            //pos=file.tellg();

            double bitMapLenght=ceil(metaDevice->TotalNumberOfBlockDevice/8);
            bitmap=new char[(int)bitMapLenght];
            file.read ((char *)bitmap, bitMapLenght);

            //pos=file.tellg();
            file.close();

            particion->_deviceMetaData=metaDevice;
            particion->FileTableDevice=fileTable;
            particion->BitMap=bitmap;
            particion->BitArraySizeInBytes=metaDevice->TotalNumberOfBlockDevice/8;

            //delete metaDevice;
            //delete fileTable;
            //delete bitmap;

            return particion;
        }
        return particion;
    }catch(int ex)
    {
        qDebug() << "Error: "+ex;
        return particion;
    }
}

void SaveDeviceChanges(Device *partition)
{
    string path=string(partition->_deviceMetaData->CompletePathDevice);

    string name =  path+"\\"+partition->_deviceMetaData->DeviceName+".JZFS";
    ofstream outFile;
    outFile.open( name.c_str() , ios::out | ios::in | ios::binary);

    if(outFile.is_open())
    {
        outFile.seekp(0,ios::beg);

        double metaDataSize=sizeof(PartitionMetadata);        
        outFile.write((char *)partition->_deviceMetaData,metaDataSize);

        for(int i=0;i<partition->_deviceMetaData->MaxNumberOfFile;i++)
        {
            outFile.write((char *)partition->FileTableDevice->TableFile[i],sizeof(File));
        }

        double BitMapSize= partition->BitArraySizeInBytes;
        outFile.write((char *)partition->BitMap,BitMapSize);

        outFile.flush();
        outFile.close();
    }
}




int main(int argc, char *argv[])
{
    //QCoreApplication a(argc, argv);


    bool continuar=true;
    cout<<"Welcome to JZ File System"<<endl;
    cout<<"Mount a device to begin. Select Mount in Menu Option\n" ;


    int  option=0;
    string inputData="";
    Device *Particion;

    //Device _particion ;
    do
    {
        cout<<"\nMenu Option\n"<<endl;
        cout<<"1.Mount Device"<<endl;
        cout<<"2.Unmount Device"<<endl;
        cout<<"3.Import File"<<endl;
        cout<<"4.Export File"<<endl;
        cout<<"5.Create File"<<endl;
        cout<<"6.Rename File"<<endl;
        cout<<"7.List Files"<<endl;
        cout<<"8.Delete File"<<endl;

        cout<<"\nSelect option: ";
        getline(cin,inputData);
        stringstream(inputData) >> option;

        if(option>=1 || option<=8){
            switch(option)
            {
            case 1:
            {
                string path="";//"E:\\SISO 2\\proyecto\\";
                string name="";//prueba";
                int size=0;

                cout<<"\nMount Device"<<endl;
                cout<<"Path: ";
                cin>>path;

                cout<<"Name: ";
                cin>>name;

                if(!LoadMetaDataDevice(path,name))
                //if(true)
                {
                    cout<<"Size (Mbs): ";
                    cin>>size;

                    Device *nuevo= new Device(size,path,name);//"E:\\SISO 2\\proyecto\\","prueba");
                    if(nuevo->CreateDevice())
                    {
                        printf("\nDevice creado\n");
                    }else{
                        printf("\nDevice no pudo se creado\n");
                    }
                    //delete nuevo;
                }
                Particion = LoadDevice(path,name);


                cout<<"\nDevice mounted succesfully\n"<<endl;
                break;
            }
            case 2:{
                if(Particion->_deviceMetaData->SizeDeviceMbs>0)
                {
                    Particion->~Device();
                    cout<<"Device unmounted succesfully"<<endl;
                    break;
                }
                cout<<"No partition is mounted"<<endl;
                break;
            }
            case 3:
            {
                string path="";//"E:\\SISO 2\\proyecto\\";
                string name="";//prueba";
                string type="";


                cout<<"\nImport File"<<endl;
                cout<<"Path: ";
                cin>>path;

                cout<<"Name: ";
                cin>>name;

                cout<<"Type: ";
                cin>>type;

                char pathArray[100];
                strcpy(pathArray,path.c_str());

                char nameArray[30];
                strcpy(nameArray,name.c_str());

                char typeArray[5];
                strcpy(typeArray,type.c_str());

                if(Particion->ImportFile(pathArray, nameArray,typeArray)){
                    SaveDeviceChanges(Particion);
                    cout<<"Importacion exitosa"<<endl;
                }else
                    cout<<"Archivo no existe"<<endl;


                break;
            }
            case 4:
            {
                string path="";//"E:\\SISO 2\\proyecto\\";
                string name="";//prueba";
                string type="";


                cout<<"\nExport File"<<endl;
                cout<<"Path: ";
                cin>>path;

                cout<<"Name: ";
                cin>>name;

                cout<<"Type: ";
                cin>>type;

                char pathArray[100];
                strcpy(pathArray,path.c_str());

                char nameArray[30];
                strcpy(nameArray,name.c_str());

                char typeArray[5];
                strcpy(typeArray,type.c_str());


                if(Particion->ExportFile(pathArray,nameArray,typeArray)){
                    cout<<"Exportacion exitosa"<<endl;
                    SaveDeviceChanges(Particion);
                }else
                    cout<<"El archivo no existe"<<endl;


                break;
            }
            case 5:
            {
                string name="";//"E:\\SISO 2\\proyecto\\";
                string type="";//prueba";


                cout<<"\nCreate File"<<endl;
                cout<<"Name: ";
                cin>>name;

                cout<<"Type: ";
                cin>>type;

                char nameArray[30];
                strcpy(nameArray,name.c_str());

                char typeArray[5];
                strcpy(typeArray,type.c_str());


                if(Particion->createFile(nameArray,typeArray,"")){
                    SaveDeviceChanges(Particion);
                    cout<<"\nArchivo Creado"<<endl;
                }else
                    cout<<"Error"<<endl;


                break;
            }

            case 6:
            {
                string nameFile="";//"E:\\SISO 2\\proyecto\\";
                string newNameFile="";//prueba";


                cout<<"\nRename File"<<endl;
                cout<<"Name of file to rename: ";
                cin>>nameFile;

                cout<<"New name: ";
                cin>>newNameFile;

                char nameArray[30];
                strcpy(nameArray,nameFile.c_str());

                char newNameArray[30];
                strcpy(newNameArray,newNameFile.c_str());

                if(Particion->RenameFile(nameArray,newNameArray))
                {
                    SaveDeviceChanges(Particion);
                    cout<<"Archivo renombrado"<<endl;
                }else
                    cout<<"No se puedo renombrar el archivo"<<endl;
                break;
            }
            case 7:
            {

                string messageHeader="\nFiles list";
                cout<<messageHeader+Particion->_deviceMetaData->DeviceName<<endl;
                Particion->ListarArchivos();

                break;
            }

            case 8:
            {
                cout<<"Incomplete functionality x_x :p"<<endl;
                break;
            }
            }
        }

        //cout<<"Invalid Option"<<endl;
        cout<<"\nWould you like to continue? [Y/N]: ";
        getline(cin,inputData);
        if(inputData[0] == 'N' || inputData[0]=='n')
            continuar=false;

    }while(continuar);

    cout<<"See you!!"<<endl;
    system("pause");

/*    //Crear archivo

    if(Particion.createFile("archivo 10",".rar","hola soy un arvhivo de prueba en examen"))
        cout<<"\nArchivo Creado"<<endl;
    else
        cout<<"archivo1 ya existe"<<endl;

    if(Particion.createFile("archivo1",".txt","hola soy un arvhivo de prueba"))
        cout<<"\nArchivo Creado"<<endl;
    else
        cout<<"archivo1 ya existe"<<endl;

    char *dataFile2=new char[15000];
    memset(dataFile2,'Z',15000);
    if(Particion.createFile("archivo2",".jrz", dataFile2))
        cout<<"\nArchivo creado"<<endl;

    if(Particion.createFile("archivo3",".txt","hola"))
        cout<<"\nArchivo Creado"<<endl;

    char *dataFile2=new char[15000];
    memset(dataFile2,'B',15000);
    if(Particion.createFile("archivo de prueba",".jrz", dataFile2))
        cout<<"\nArchivo creado"<<endl;

    if(Particion.ImportFile("E:\\SISO 2","Sistemas operativos modernos",".pdf"))
        cout<<"Importacion exitosa"<<endl;
    else
        cout<<"El archivo es muy grande o ya existe"<<endl;

    SaveDeviceChanges(Particion);*/
}
