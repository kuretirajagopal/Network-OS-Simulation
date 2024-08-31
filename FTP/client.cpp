#include <sys/socket.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <sys/sendfile.h>
#include <bits/stdc++.h>
#include <filesystem>
#include <fstream>
#include <arpa/inet.h>
#include <unistd.h>
using namespace std;

void handleError(string s){
    cout<<s<<endl;
    exit(1);
}
void transferToServer(int sockfd,int fileDescriptor,string s,string filename,int* fileExists,int* overwrite){
    int status,filesize;
    send(sockfd,s.c_str(),100,0);	
    recv(sockfd,fileExists,sizeof(int),0);
    if(*fileExists){
        cout<<filename<<" ALREADY EXISTS!!!"<<endl;
        cout<<"Do you want to overwrite? "<<endl<<"1: YES"<<endl<<"2: NO"<<endl;
        cin>>*overwrite;
    }
    send(sockfd,overwrite,sizeof(int),0);		 
    if(*overwrite==1)
    {
        filesize=filesystem::file_size(filename);
        send(sockfd, &filesize, sizeof(int), 0);
        sendfile(sockfd, fileDescriptor, NULL, filesize);				
        recv(sockfd, &status, sizeof(int), 0);
        if(status)
            cout<<filename<<"("<<filesize<<"Bytes)"<<" transfer successful "<<endl;						
        else
            cout<<filename<<" Failed to transfer"<<endl; 
    }
}
void getFromServer(int sockfd,char* filename,int* overwrite,int* fileExists){
    int filesize,fileDescriptor;
    char* f;		
    recv(sockfd, &filesize, sizeof(int), 0);
    if(!filesize)
    {
        cout<<filename<<" doesn't exists in the server"<<endl;
        return;
    }
    if( filesystem::exists(filename) )
    {
        *fileExists = 1;
        cout<<filename<<" ALREADY EXISTS!!!"<<endl;
        cout<<"Do you want to overwrite? "<<endl<<"1: YES"<<endl<<"2: NO"<<endl;
        cin>>*overwrite;
    }
    send(sockfd,overwrite,sizeof(int),0);

    if(*fileExists==1 && *overwrite == 1)
    {
        fileDescriptor = open(filename, O_WRONLY | O_CREAT | O_TRUNC, 644);		
    }
    else if( *fileExists ==0 && *overwrite == 1)
    {
        fileDescriptor = open(filename, O_CREAT | O_EXCL | O_WRONLY, 0666);	     
    } 
    if(*overwrite==1)
    {  
        f = new char[filesize];
        recv(sockfd, f, filesize, 0);
        write(fileDescriptor, f, filesize);
        cout<<filename<<"("<<filesize<<"Bytes)"<<" received successfully "<<endl;	
          
    } 
    close(fileDescriptor);        
}
int main(int argc, char *argv[])
{
    int sockfd = 0, n = 0,port=atoi(argv[2]);
    char recvBuffer[1024];
    struct sockaddr_in serv_addr; 
    if(argc != 3)
    {
      handleError("incorrect arguments");
    } 
    bzero(recvBuffer, sizeof(recvBuffer)); 
    if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
       handleError("socket error");
    } 
    bzero(&serv_addr, sizeof(serv_addr)); 			
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port); 				
    if(inet_pton(AF_INET, argv[1], &serv_addr.sin_addr)<=0)
    {
        handleError("invalid address");
    } 
    if( connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)			
    {
       handleError("connection error");
    } 
    int choice;
    char filename[20],buf[100];
    string command,extension;
    int fileDescriptor;
    int filesize,status,i=1;
    int fileExists=0;
    int overwrite = 1;
    int num_files;
    while(1)
    {
        cout<<"Enter a choice:"<<endl;
        cout<<"1: PUT"<<endl<<"2: GET"<<endl<<"3: MPUT"<<endl<<"4: MGET"<<endl<<"5: EXIT"<<endl;
        cin>>choice;
        switch(choice)
        {
        // PUT
            case 1:		{												
                cout<<"Give the filename:"<<endl;
                cin>>filename;
                if( !filesystem::exists(filename) )
	            {
                    cout<<" File doesn't exist"<<endl;
                    break;
	            } 
                fileDescriptor = open(filename,O_RDONLY);						
                string s="put ";
                s+=filename;
             // send(sockfd,s.c_str(),100,0);	
                transferToServer(sockfd, fileDescriptor, s,filename, &fileExists, &overwrite);
                close(fileDescriptor); 
              break;
            }
// GET
            case 2:{
                cout<<"Give the filename:"<<endl;
                cin>>filename;
                string s="get ";
                s+=filename;
                send(sockfd, s.c_str(), 100, 0);	
                getFromServer(sockfd,filename,&overwrite, &fileExists);			 
                break;
            }
//  MPUT
          case 3:{
                cout<<"Enter extension"<<endl;
                cin>>extension;								 
                string fileListingCommand="ls *.";							
               	fileListingCommand+=extension;
                fileListingCommand+="> temp.txt";
                system(fileListingCommand.c_str());
                string line;                   
                ifstream inputFile("temp.txt");
               while (getline(inputFile, line))           
                {   
                   string str="put ";
                   str+=line;
                    fileDescriptor = open(line.c_str(),O_RDONLY);			
                    transferToServer(sockfd,fileDescriptor,str,line,&fileExists, &overwrite);
                    close(fileDescriptor); 
                    overwrite = 1;						
                } 
                cout<<"END"<<endl;
                inputFile.close(); 	
                remove("temp.txt");
                break;
          }
// MGET
            case 4:{
                cout<<"enter the extension"<<endl;				
                cin>>extension;
                string s="mget ";
                s+=extension;
                send(sockfd,s.c_str(),100,0);									 
                recv(sockfd,&num_files,sizeof(int),0);					
                while(num_files > 0)
                {
                    recv(sockfd,filename,20,0);	
                    getFromServer(sockfd, filename,&overwrite, &fileExists);						
                    overwrite = 1;
                    fileExists = 0;
                    num_files--; 
                }
                break;
            }
//  QUIT
            case 5:{
                string s="quit";
                send(sockfd, s.c_str(), 100, 0);					
                recv(sockfd, &status, 100, 0);
                if(status)
                {
                    cout<<"Server closed the connection!!"<<endl;
                    close(sockfd);
                    exit(0);
                }
                cout<<"Server failed to close connection"<<endl;		
                break;
            }
            default:
            	printf("choose the vaild option\n");
            	break;
        } 
    }
    return 0;
}
