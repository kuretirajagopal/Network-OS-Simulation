#include <sys/socket.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <sys/sendfile.h>
#include <bits/stdc++.h>
#include <filesystem>
#include <fstream>
#include <unistd.h>
using namespace std;

void handleError(string s){
    cout<<s<<endl;
    exit(1);
}
void transferToClient(int fileDescriptor,int* overwrite,string filename,int new_sockfd){
    int filesize;
    if(fileDescriptor == -1){
            filesize = 0;
        } 
        else  {  
            filesize=filesystem::file_size(filename);
        }
        send(new_sockfd, &filesize, sizeof(int), 0);       
        if(filesize==0)
            return;
        recv(new_sockfd,overwrite,sizeof(int),0);   
        if( *overwrite == 1)
        sendfile(new_sockfd, fileDescriptor, NULL, filesize);    
}
int main(int argc, char *argv[])
{   
    if(argc!=2){
        handleError("incorrect arguments");
    }
    int port=atoi(argv[1]);
    int sockfd = 0, new_sockfd = 0,k;
    struct sockaddr_in serv_addr ; 
    char sendBuffer[1025]; 
    sockfd = socket(AF_INET, SOCK_STREAM, 0);        
    if(sockfd==-1){
        handleError("socket error");
    }
    bzero(&serv_addr,  sizeof(serv_addr));
    bzero(sendBuffer, sizeof(sendBuffer)); 
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;             
    serv_addr.sin_port = htons(port);                     
    k = bind(sockfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr));        
    if(k==-1){
        handleError("bind error");
    }
    k = listen(sockfd, 10); 
    if(k==-1){
        handleError("listen error");
    }
    new_sockfd = accept(sockfd, (struct sockaddr*)NULL, NULL);
    char buffer[100];
    string fileListingCommand,extension,action,filename;       
    int filesize,i,fileDescriptor;
    int fileExists = 0;
    int overwrite = 1;
    while(1)                    
    {   
        recv(new_sockfd,buffer,100,0);
// PUT
        stringstream ss(buffer);
        ss>>action;
        if(action=="put")
        {
            int c = 0;
            char *f;
            ss>>filename;
            i = 1;
            if( filesystem::exists( filename))
            {
                fileExists = 1;
                send(new_sockfd,&fileExists,sizeof(int),0);              
            } 
            else 
            {
                fileExists = 0;
                 send(new_sockfd,&fileExists,sizeof(int),0);             
            }
            recv(new_sockfd,&overwrite,sizeof(int),0);                
            if(fileExists==1 && overwrite == 1)
            {
                fileDescriptor = open(filename.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 644);        
            }
            else if(fileExists == 0 && overwrite == 1)           
            {
                fileDescriptor = open(filename.c_str(), O_CREAT | O_EXCL | O_WRONLY, 0666);   
            }
            if(overwrite==1)
            {
                recv(new_sockfd, &filesize, sizeof(int), 0);
                f = new char[filesize];
                recv(new_sockfd, f, filesize, 0);
                c = write(fileDescriptor, f, filesize);
                
                send(new_sockfd, &c, sizeof(int), 0);
            }
            close(fileDescriptor);
        } 
// GET
        else if(action=="get")
        {
            ss>>filename;
            fileDescriptor = open(filename.c_str(), O_RDONLY);   
            transferToClient( fileDescriptor, &overwrite, filename,  new_sockfd);                   
            close(fileDescriptor);
        }
//  MGET
        else if(action=="mget")
        {
            ss>>extension;
            cout<<extension<<endl; 
            fileListingCommand="ls *.";
            fileListingCommand+=extension;
            fileListingCommand+="> temp.txt";
            system(fileListingCommand.c_str());
            ifstream inputFile("temp.txt");
            vector<string>files;
            string line;
            while (getline(inputFile, line)) {
                    files.push_back(line);
            }
            inputFile.close();  
            int num_files = files.size(); 
            send(new_sockfd,&num_files,sizeof(int),0);             
            for (int i=0;i<num_files;i++)
            {
                send(new_sockfd,files[i].c_str(),20,0); 
                fileDescriptor = open(files[i].c_str(), O_RDONLY);        
                transferToClient(fileDescriptor, &overwrite,files[i],new_sockfd) ;                
                close(fileDescriptor);
            }
            remove("temp.txt"); 
        }
        // quit
        else if(action== "quit")
        {
            cout<<"Server Closed the connection.."<<endl;
            i = 1;
            send(new_sockfd, &i, sizeof(int), 0);   
            close(new_sockfd);
            close(sockfd);        
            exit(0);
        }
    }
}
