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

void pairsort(int *a, int *b, int n)
{
    pair<int, int> pairt[n];

    for (int i = 0; i < n; i++) 
    {
        pairt[i].first = a[i];
        pairt[i].second = b[i];
    }

    sort(pairt, pairt + n);

    for (int i = 0; i < n; i++) 
    {
        a[i] = pairt[i].first;
        b[i] = pairt[i].second;
    }
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
	pairsort(neigh_id,neigh_port,no_neighbours);


	int no_searchfiles;
	infile>>no_searchfiles;

	vector<string> searchfiles;
	char files[no_searchfiles][65546];
	for (int i = 0; i < no_searchfiles; ++i) {
		infile>>files[i];
	}
	qsort(files, no_searchfiles, 65546, (int (*)(const void *, const void *)) strcmp);

	for (int i = 0; i < no_searchfiles; ++i)
	{
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
			//cout<<entity->d_name<<endl;
			myfiles.push_back(entity->d_name);
		}
		entity=readdir(dir);
	}
	closedir(dir);
	sort(myfiles.begin(), myfiles.end());
	for(auto x: myfiles)
		cout<<x<<endl;

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

	for (int i = 0; i < no_searchfiles; ++i) {
		if (searchfilesid[i]!=0) {
			cout<<"Found "<<searchfiles[i]<<" at "<<searchfilesid[i]<<" with MD5 0 at depth 1"<<endl;
		}
		else
			cout<<"Found "<<searchfiles[i]<<" at "<<searchfilesid[i]<<" with MD5 0 at depth 0"<<endl;
	}

	close(sockfd);
	for (int i = 0; i < no_neighbours; ++i) {
		close(sock_to_neighbours[i]);
	}
	
	return 0;
}
