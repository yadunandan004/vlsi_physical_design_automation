// header files
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <list>
#include <map>
#include <string>
#include <algorithm>
#include <limits>
#include "cell.h"
using namespace std;

int nNets;
float ratio_factor=0.5;
map<string,Cell> cellData;
map<int,vector<string> > netNodeMap;
vector<string> cellIds;		//vector containing all cellIds to access cellData map at later stage
//create maxheap to contain max cell area
vector<int> sheap;

void readCellArea(string);
void createPartition();
void readhgrFile(string);
int computeGain(string);
int FS(string);
int TS(string);
int computePartitionArea(int);
int computeTotalArea();
int cellMaxArea();
bool checkAreaContraint();
// main  program
int main()
{
	make_heap(sheap.begin(),sheap.end());
	readCellArea("ibm01\\ibm01.are");
	createPartition();
	readhgrFile("ibm01\\ibm01.hgr");
	// cout<<cellData["a24"].partition<<endl;
	cout<<checkAreaContraint()<<endl;
	// cout<<cellMaxArea()<<endl;
	return 0;
}
//splitting a string with a given delimiter
vector<string> split(const string &s, char delim) {
    stringstream ss(s);
    string item;
    vector<string> tokens;
    while (getline(ss, item, delim)) {
        tokens.push_back(item);
    }
    return tokens;
}
//read all the  cell areas from the ".are" file
void readCellArea(string file){
	string line;
	std::ifstream arefile(file.c_str());
	if(!arefile)
	{
		cout<<"File not found"<<endl;
	}
	//If file is found
	else{
		//parsing each line and intialize each cell in the cell map
		while(getline(arefile,line))
		{
				vector<string> x = split(line,' '); // split each line for cellid and area
				Cell temp; 	//create a temporary cell
				temp.Id=x.at(0);
				temp.locked=false;
				temp.area=atoi(x.at(1).c_str());
				sheap.push_back(atoi(x.at(1).c_str()));
				push_heap(sheap.begin(),sheap.end());
				cellIds.push_back(x.at(0));
				cellData.insert(pair<string,Cell>(x.at(0),temp));
		}
	}
}

//creates a partition  of nodes in the map
//first half goes to partition 0 and second half to partition 1
void createPartition()
{
	int size=cellData.size();
	int half=(int)size/2;
	for(int i=0;i<size;i++)
	{
			if(i<=half)
			{
				cellData[cellIds.at(i)].partition=0;
			}
			else{
				cellData[cellIds.at(i)].partition=1;
			}
	}
}

// read hgr files
void readhgrFile(string file)
{
	string line;
	string line2;
	ifstream hgrfile(file.c_str());
	if(!hgrfile)
	{
		cout<<"File cannot be opened"<<endl;
	}
	else
	{
		//get the 1st line containing header info
		getline(hgrfile,line);
		vector <string>x=split(line,' ');
		nNets=atoi(x.at(0).c_str());
		//get the rest of the lines containing individual nets and nodes
		//id is int to account for uniqueness in key
		int id=0;
		while(getline(hgrfile,line2))
		{
			vector<string> temp= split(line2,' ');
			//adding all netids to individual cell data, to know how many nets contain that cell or node
			for (int i=0;i<temp.size();i++)
			{
				cellData[temp.at(i)].netList.push_back(id);
			}
			//adding netid, and all nodes in that net to a map
			netNodeMap.insert(pair<int,vector<string> >(id,temp));
			id++;
		}
	}
}
int computeGain(string cell)
{
	return FS(cell)-TS(cell);
}
//compute no of times the all the cells except that cell in the net
//stay in other partition
int FS(string cell)
{
	int fs=0;
	//grab all netids to which the cell belongs to
	vector<int> netList=cellData[cell].netList;
	for(int i=0;i<netList.size();i++)
	{
		cout<<netList.at(i)<<endl;
		//grab all nodes or cells from that net, using netid
		vector<string> net=netNodeMap[netList.at(i)];
		int netSp=0;
		cout<<net.size()<<endl;
		for(int j=0;j<net.size();j++)
		{
				string node=net.at(j).c_str();
				if(cell != node)
				{
					//other cells in diff partition compared to current cell will yield netSp as zero
					if(cellData[cell].partition==cellData[node].partition)
					{
						netSp++;
					}
				}
		}
		cout<<"\n"<<endl;
		if(netSp==0)
		{
			fs++;
		}
	}
	cout<<fs<<endl;
	return fs;
}
//compute no of times the all the cells in the net belonging to that cell
//stay in the same partion
int TS(string cell)
{
	int ts=0;
		//grab all netids to which the cell belongs to
	vector<int> netList=cellData[cell].netList;
	for(int i=0;i<netList.size();i++)
	{
		//grab all nodes or cells from that net, using netid
		vector<string> net=netNodeMap[netList.at(i)];
		int netSp=0;
		for(int j=0;j<net.size();j++)
		{
				string node=net.at(j).c_str();
				if(cell != node)
				{
					//other cells in the same partition compared to current cell will yield netSp as zero
					if(cellData[cell].partition!=cellData[node].partition)
					{
						netSp++;
					}
				}
		}
		if(netSp==0)
		{
			ts++;
		}
	}
	cout<<ts<<endl;
	return ts;
}
int computePartitionArea(int p)
{
		int area=0;
		for(int i=0;i<cellIds.size();i++)
		{
			string id=cellIds.at(i).c_str();
			if(cellData[id].partition==p)
			{
				area+=cellData[id].area;
			}
		}
		return area;
}
int computeTotalArea()
{
	return(computePartitionArea(0)+computePartitionArea(1));
}
//pop heap top node to get max area
int cellMaxArea()
{
	int max=sheap.front();
	// pop_heap(sheap.begin(),sheap.end());
	return max;
}

bool checkAreaContraint()
{
		int p0=computePartitionArea(0);
		int totalArea=computeTotalArea();
		int maxAreaCell=cellMaxArea();
		if(((ratio_factor*totalArea-maxAreaCell)<=p0)&&(p0<=(ratio_factor*totalArea+maxAreaCell)))
		{
			return true;
		}
		else
		{
			return false;
		}
}

void moveCell()
{
	
}
