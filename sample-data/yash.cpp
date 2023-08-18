#include<iostream>
#include <bits/stdc++.h>
using namespace std;

#include <unistd.h>
#include <stdio.h>
#include<string.h>
#include <stdlib.h>
#include<errno.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fstream>
#include <dirent.h>
#include<ctime>
#include <thread>
#include<sys/sendfile.h>
#include <sys/stat.h>
#include<fcntl.h>
#include<openssl/md5.h>
#include <sys/mman.h>
#include<chrono>

using namespace std::this_thread;     // sleep_for, sleep_until
using namespace std::chrono_literals; // ns, us, ms, s, h, etc.
using std::chrono::system_clock;

unsigned long get_size_by_fd(int fd) {
    struct stat statbuf;
    if(fstat(fd, &statbuf) < 0) exit(-1);
    return statbuf.st_size;
}

void print_md5_sum(unsigned char* md) {
    int i;
    for(i=0; i <MD5_DIGEST_LENGTH; i++) {
            printf("%02x",md[i]);
    }
}


string checkfiles(string searchstring,vector<string> myfiles){

	string ackstring="ack ";
	for (int i = 0; i < myfiles.size(); ++i) {
		if(searchstring.find(myfiles[i])!=string::npos){
			ackstring+=myfiles[i];
		}
	}
	return ackstring;
}


string convertToString(char* a){
    string s(a);
 
    return s;
}

int main(int argc, char const *argv[]) {
	

	if(argc !=3){
		cout<<"Usage: executable config-file.txt file_path"<<endl;
		return 1;
	}

	//reading the data from the config file
	ifstream infile;
	infile.open(argv[1]);

	int MYPORT,id,UNIQUE_id;
	infile>>id;
	infile>>MYPORT;
	infile>>UNIQUE_id;

	int no_neighbours;
	infile>>no_neighbours;

	int neigh_id[no_neighbours],neigh_port[no_neighbours];
	for (int i = 0; i < no_neighbours; ++i) {
		infile>>neigh_id[i];
		infile>>neigh_port[i];
	}

	int no_searchfiles;
	infile>>no_searchfiles;

	vector<string> searchfiles;
	char files[no_searchfiles][65546];
	for (int i = 0; i < no_searchfiles; ++i) {
		infile>>files[i];
		searchfiles.push_back(convertToString(files[i]));
	}

	vector<string> myfiles;

	DIR* dir=opendir(argv[2]);
	if(dir==NULL){
		cout<<"Two arguments nedded";
		return 1;
	}

	struct dirent* entity;
	entity = readdir(dir);

	while(entity != NULL){
		if(strcmp(entity->d_name,".")!=0 && strcmp(entity->d_name,"..")!=0 && entity->d_type!=DT_DIR){
			cout<<entity->d_name<<endl;
			myfiles.push_back(entity->d_name);
		}
		entity=readdir(dir);
	}
	closedir(dir);

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	int uniqueId[no_neighbours]; //table for storing the unique ID

	int searchfilesid[no_searchfiles];

	for (int i = 0; i < no_searchfiles; ++i) {
		searchfilesid[i]=0;
	}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//connecting with the neighbours

	int sockfd=socket(AF_INET,SOCK_STREAM,0);
	int sock_to_neighbours[no_neighbours];

	struct sockaddr_in myaddr;

	myaddr.sin_family = AF_INET;
	myaddr.sin_port=htons(MYPORT);
	myaddr.sin_addr.s_addr=INADDR_ANY;
	memset(&(myaddr.sin_zero),'\0',8);

	bind(sockfd,(struct sockaddr *)&myaddr,sizeof(struct sockaddr));

	for (int i = 0; i < no_neighbours; ++i) {
		sock_to_neighbours[i]=socket(AF_INET,SOCK_STREAM,0);
	}

	
	struct sockaddr_in neigh_addrs[no_neighbours];

	for (int i = 0; i < no_neighbours; ++i) { //filling the neighbour address
		neigh_addrs[i].sin_family=AF_INET;
		neigh_addrs[i].sin_port=htons(neigh_port[i]);
		inet_aton("127.0.0.1",&(neigh_addrs[i].sin_addr));
		memset(&(neigh_addrs[i].sin_zero),'\0',8);
	}
	

	listen(sockfd,20);			//listening for the incoming connections

	for (int i = 0; i < no_neighbours; ++i) {	//establishing the connections

		if(id > neigh_id[i]){
			bool a=true;

			while(a){
				int b=connect(sock_to_neighbours[i],(struct sockaddr *)&neigh_addrs[i],sizeof(neigh_addrs[i]));
				if(b ==0 ){
					a=false;
				}
			}
			if(!a){
				//cout<<"connected"<<endl;
				///////////////////////////////////////////////////////
				string s=to_string(UNIQUE_id);
				int n=s.length();
				char msg[n+1];
				strcpy(msg,s.c_str());
				int len,bytes_sent;
				len=strlen(msg);
				bytes_sent=send(sock_to_neighbours[i],msg,len,0);
				//////////////////////////////////////////////////////

				char buffer[65546];
				int trail=recv(sock_to_neighbours[i],buffer,65546,0);
				uniqueId[i]=atoi(buffer);
				memset(&buffer[0],0,sizeof(buffer));
				////////////////////////////////////////////////////////
				cout<<"Connected to "<<neigh_id[i]<<" with unique-ID "<<uniqueId[i]<<" on port "<<neigh_port[i]<<endl;

				/*
					sending the information about their files and searching in them
				*/
				string sendsearch="";
				for (int j = 0; j < no_searchfiles; ++j) {
					sendsearch+=searchfiles[j];
					sendsearch+=" ";
				}
				//cout<<"search string: "<<sendsearch<<endl;

				int n1=sendsearch.length();
				char msg1[n1+1];
				strcpy(msg1,sendsearch.c_str());
				int len1,bytes_sent1;
				len1=strlen(msg1);
				bytes_sent1=send(sock_to_neighbours[i],msg1,len1,0);


				/*
					receiveing the requests for searching
				*/
				char buffer1[65546];
				int trail1=recv(sock_to_neighbours[i],buffer1,65546,0);
				string receivedstring=convertToString(buffer1);
				memset(&buffer1[0],0,sizeof(buffer1));
				//cout<<"Received the string "<<receivedstring<<endl;
				
				string acksend=checkfiles(receivedstring,myfiles);

				int n2=acksend.length();
				char msg2[n2+1];
				strcpy(msg2,acksend.c_str());
				int len2,bytes_sent2;
				len2=strlen(msg2);
				//cout<<"Sending the ACK string "<<msg2<<endl;
				bytes_sent2=send(sock_to_neighbours[i],msg2,len2,0);

				char buffer2[65546];
				int trail2=recv(sock_to_neighbours[i],buffer2,65546,0);
				string ackreceive=convertToString(buffer2);
				memset(&buffer2[0],0,sizeof(buffer2));
				//cout<<"ack received the string "<<ackreceive<<endl;
				
				for (int j = 0; j < searchfiles.size(); ++j) {
					if(ackreceive.find(searchfiles[j])!=string::npos){
						//cout<<"Found "<<searchfiles[j]<<" at "<<neigh_id[i]<<endl;
						if(searchfilesid[j]==0){
                            searchfilesid[j]=uniqueId[i];
                        } else{
                            searchfilesid[j]=min(searchfilesid[j],uniqueId[i]);
                        }

					}
				}

			}
		}	
		else if(id < neigh_id[i]){
			bool a=true;
			int ready_socket;
			while(a){
				socklen_t sin_size=sizeof(struct sockaddr_in);
				sock_to_neighbours[i]=accept(sockfd,(struct sockaddr *)&neigh_addrs[i],&sin_size);
				if(sock_to_neighbours[i] >=0){
					a=false;
				}
			}
			if (!a) {

				///////////////////////////////////////////////////////
				string s=to_string(UNIQUE_id);
				int n=s.length();
				char msg[n+1];
				strcpy(msg,s.c_str());
				int len,bytes_sent;
				len=strlen(msg);
				bytes_sent=send(sock_to_neighbours[i],msg,len,0);
				//////////////////////////////////////////////////////

				char buffer[65546];
				int trail=recv(sock_to_neighbours[i],buffer,65546,0);
				uniqueId[i]=atoi(buffer);
				memset(&buffer[0],0,sizeof(buffer));
				////////////////////////////////////////////////////////
				cout<<"Connected to "<<neigh_id[i]<<" with unique-ID "<<uniqueId[i]<<" on port "<<neigh_port[i]<<endl;

				/*
					sending the information about their files and searching in them
				*/
				string sendsearch="";
				for (int j = 0; j < no_searchfiles; ++j) {
					sendsearch+=searchfiles[j];
					sendsearch+=" ";
				}
				

				char buffer1[65546];
				int trail1=recv(sock_to_neighbours[i],buffer1,65546,0);
				string receivedstring=convertToString(buffer1);
				memset(&buffer1[0],0,sizeof(buffer1));

				//////////////////////////////////////////////////////////////

				int n1=sendsearch.length();
				char msg1[n1+1];
				strcpy(msg1,sendsearch.c_str());
				int len1,bytes_sent1;
				len1=strlen(msg1);
				bytes_sent1=send(sock_to_neighbours[i],msg1,len1,0);

				string acksend=checkfiles(receivedstring,myfiles);
				

				char buffer2[65546];
				int trail2=recv(sock_to_neighbours[i],buffer2,65546,0);
				string ackreceive=convertToString(buffer2);
				memset(&buffer2[0],0,sizeof(buffer2));

				int n2=acksend.length();
				char msg2[n2+1];
				strcpy(msg2,acksend.c_str());
				int len2,bytes_sent2;
				len2=strlen(msg2);
				bytes_sent2=send(sock_to_neighbours[i],msg2,len2,0);

				for (int j = 0; j < searchfiles.size(); ++j) {
					if(ackreceive.find(searchfiles[j])!=string::npos){
						//cout<<"Found "<<searchfiles[j]<<" at "<<neigh_id[i]<<endl;
						if(searchfilesid[j]==0){
                            searchfilesid[j]=uniqueId[i];
                        } else{
                            searchfilesid[j]=min(searchfilesid[j],uniqueId[i]);
                        }
					}
				}
				
			}
		}	
	}


	string path=string(argv[2])+"Downloaded";
	char loc[path.length()+1];
	strcpy(loc,path.c_str());
	mkdir(loc,0777);


	for(int i=0; i < no_neighbours; ++i){
		if(id > neigh_id[i]){
			string requeststring="requesting-- ";
			int count=0;
			vector<string> requestedfiles;
			for(int j=0;j<no_searchfiles; ++j){
				if(searchfilesid[j]==uniqueId[i]){
					requeststring+=searchfiles[j];
					count++;
					requestedfiles.push_back(searchfiles[j]);
				}
			}
			int n=requeststring.length();
			char msg[n+1];
			strcpy(msg,requeststring.c_str());
			int len,bytes_sent;
			len=strlen(msg);
			bytes_sent=send(sock_to_neighbours[i],msg,len,0);
			//cout<<"sent "<<msg<<endl;

			char buffer[65546];
			int trail2=recv(sock_to_neighbours[i],buffer,65546,0);
			string receivedrequest=convertToString(buffer);
			memset(&buffer[0],0,sizeof(buffer));
			//cout<<"received "<<receivedrequest<<endl;
			


			for(int j=0;j<myfiles.size();j++){
				if(receivedrequest.find(myfiles[j])!=string::npos){
					
					int fd;
					char file_size[256];
					struct stat file_stat;
					int remain_data;
					int sent_bytes=0;

                    
					
					//////////////
					string file=string(argv[2])+myfiles[j];
					fd=open(file.c_str(),O_RDONLY);
					if(fd==-1){
						fprintf(stderr, "Error opening file --> %s", strerror(errno));

                		exit(EXIT_FAILURE);
					}

					if(fstat(fd,&file_stat)<0){
						 fprintf(stderr, "Error fstat --> %s", strerror(errno));

                		exit(EXIT_FAILURE);
					}
					
					sprintf(file_size, "%d", (int)file_stat.st_size);

					len=send(sock_to_neighbours[i],file_size,sizeof(file_size),0);
					if (len < 0){
						fprintf(stderr, "Error on sending greetings --> %s", strerror(errno));

						exit(EXIT_FAILURE);
					}

					off_t offset=0;
					remain_data=file_stat.st_size;

                    string a=convertToString(file_size);
                    int n=stoi(a);

					while((( sent_bytes=sendfile(sock_to_neighbours[i],fd, &offset,n))>0) && (remain_data>0)){
						remain_data-=sent_bytes;
						cout<<"sending side : "<<remain_data<<endl;
					}
					//sleep_for(1000000000ns);
                    /*
					char buffer[1];
					int trail=recv(sock_to_neighbours[i],buffer,1,0);
                    memset(&buffer[0],0,sizeof(buffer)); */

				}
			}

			for(int j=0;j<count;j++){
				
				char buffer[BUFSIZ];
				int len;
				int file_size;
				FILE *receivedfile;
				int remain_data=0;

				int trial=recv(sock_to_neighbours[i],buffer,BUFSIZ,0);
				file_size=atoi(buffer);

				string filename=string(argv[2]) +"Downloaded/"+requestedfiles[j];
				const char *FILENAME=filename.c_str();
				receivedfile=fopen(FILENAME,"w");

				remain_data=file_size;

                char buffer0[file_size];

				memset(&buffer[0],0,BUFSIZ);

				while(remain_data>0 && ((len=recv(sock_to_neighbours[i],buffer0,file_size,0))>0)){
					fwrite(buffer0,sizeof(char),len,receivedfile);
					memset(&buffer0[0],0,BUFSIZ);
					remain_data-=len;
					cout<<"receiving side: "<<remain_data<<endl;
				}
				//cout<<"len: "<<len<<endl;
				//cout<<remain_data<<endl;

                //char msg[]="/";
                //int bytes_sent1=send(sock_to_neighbours[i],msg,strlen(msg),0);

				memset(&buffer0[0],0,file_size);

				unsigned char result[MD5_DIGEST_LENGTH];

				int file_descript;
				unsigned long file_size1;
				char* file_buffer;

				file_descript=open(FILENAME,O_RDONLY);

				file_size1=get_size_by_fd(file_descript);

				file_buffer=(char *)mmap(0,file_size,PROT_READ,MAP_SHARED,file_descript,0);

				MD5((unsigned char*) file_buffer,file_size1,result);
				munmap(file_buffer,file_size1);

				cout<<"Found "<<requestedfiles[j]<<" at "<<uniqueId[i]<<" with MD5 ";
				print_md5_sum(result);
				cout<<" at depth 1"<<endl;

			}

			



		}
		else if(id < neigh_id[i]){
			string requeststring="requesting-- ";
			int count=0;
			vector<string> requestedfiles;
			for(int j=0;j<no_searchfiles; ++j){
				if(searchfilesid[j]==uniqueId[i]){
					requeststring+=searchfiles[j];
					count++;
					requestedfiles.push_back(searchfiles[j]);
				}
			}

			char buffer[65546];
			int trail2=recv(sock_to_neighbours[i],buffer,65546,0);
			string receivedrequest=convertToString(buffer);
			memset(&buffer[0],0,sizeof(buffer));
			//cout<<"received "<<receivedrequest<<endl;

			int n=requeststring.length();
			char msg[n+1];
			strcpy(msg,requeststring.c_str());
			int len,bytes_sent;
			len=strlen(msg);
			bytes_sent=send(sock_to_neighbours[i],msg,len,0);
			//cout<<"sent "<<msg<<endl;

			for(int j=0;j<count;j++){
				
				char buffer[BUFSIZ];
				int len;
				int file_size;
				FILE *receivedfile;
				int remain_data=0;

				int trial=recv(sock_to_neighbours[i],buffer,BUFSIZ,0);
				file_size=atoi(buffer);

				string filename=string(argv[2]) +"Downloaded/"+requestedfiles[j];
				const char *FILENAME=filename.c_str();
				receivedfile=fopen(FILENAME,"w");

				remain_data=file_size;

                char buffer0[file_size];

				memset(&buffer[0],0,BUFSIZ);

				while(remain_data>0 && ((len=recv(sock_to_neighbours[i],buffer0,file_size,0))>0)){
					fwrite(buffer0,sizeof(char),len,receivedfile);
					memset(&buffer0[0],0,BUFSIZ);
					remain_data-=len;
					cout<<"receiving side: "<<remain_data<<endl;
				}
				//cout<<"len: "<<len<<endl;
				//cout<<remain_data<<endl;

                //char msg[]="/";
                //int bytes_sent1=send(sock_to_neighbours[i],msg,strlen(msg),0);

				memset(&buffer0[0],0,file_size);

				unsigned char result[MD5_DIGEST_LENGTH];

				int file_descript;
				unsigned long file_size1;
				char* file_buffer;

				file_descript=open(FILENAME,O_RDONLY);

				file_size1=get_size_by_fd(file_descript);

				file_buffer=(char *)mmap(0,file_size,PROT_READ,MAP_SHARED,file_descript,0);

				MD5((unsigned char*) file_buffer,file_size1,result);
				munmap(file_buffer,file_size1);

				cout<<"Found "<<requestedfiles[j]<<" at "<<uniqueId[i]<<" with MD5 ";
				print_md5_sum(result);
				cout<<" at depth 1"<<endl;

			}


			for(int j=0;j<myfiles.size();j++){
				if(receivedrequest.find(myfiles[j])!=string::npos){
					
					int fd;
					char file_size[256];
					struct stat file_stat;
					int remain_data;
					int sent_bytes=0;

					
					//////////////
					string file=string(argv[2])+myfiles[j];
					fd=open(file.c_str(),O_RDONLY);
					if(fd==-1){
						fprintf(stderr, "Error opening file --> %s", strerror(errno));

                		exit(EXIT_FAILURE);
					}

					if(fstat(fd,&file_stat)<0){
						 fprintf(stderr, "Error fstat --> %s", strerror(errno));

                		exit(EXIT_FAILURE);
					}
					
					sprintf(file_size, "%d", (int)file_stat.st_size);

					len=send(sock_to_neighbours[i],file_size,sizeof(file_size),0);
					if (len < 0){
						fprintf(stderr, "Error on sending greetings --> %s", strerror(errno));

						exit(EXIT_FAILURE);
					}

					off_t offset=0;
					remain_data=file_stat.st_size;

                    string a=convertToString(file_size);
                    int n=stoi(a);
                    
					while((( sent_bytes=sendfile(sock_to_neighbours[i],fd, &offset,n))>0) && (remain_data>0)){
						remain_data-=sent_bytes;
						cout<<"sending side : "<<remain_data<<endl;
					}
					//sleep_for(1000000000ns);
                    /*
					char buffer[1];
					int trail=recv(sock_to_neighbours[i],buffer,1,0);
                    memset(&buffer[0],0,sizeof(buffer)); */

				}
			}

		}
	}
	//sleep_for(5000000000ns);
	
	close(sockfd);
	for (int i = 0; i < no_neighbours; ++i) {
		close(sock_to_neighbours[i]);
	}
	
	return 0;
}
