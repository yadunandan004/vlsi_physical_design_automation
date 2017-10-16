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
float ratio_factor=0.6;//ratio fixed initially
map<string,Cell> cellData;// an object of the cell class is created for each cell
map<int,vector<string> > netNodeMap;
map<int,vector<string> >gainBucket;
vector<string> cellIds;		//vector containing all cellIds to access cellData map at later stage
int initcutSz;
int mincutSz;
int totalArea;
//create maxheap to contain max cell area
vector<int> sheap;

void readCellArea(string);
void createPartition();
void readhgrFile(string);
int computeGain(string);
int FS(string);
int TS(string);
int computePartitionArea(int);// gives arez of each partion
int computeTotalArea();// sum of all cells
int cellMaxArea();// cell with max area is returned
bool checkAreaConstraint();
void moveCells();// to move locked cell to  coplementary partition
int computeCutSize();// calculates cut size
void createGainBucket();
void updateGainBucket();
// main  program
int main()
{
	make_heap(sheap.begin(),sheap.end());
	readCellArea("ibm01\\ibm01.are");
	createPartition();
	// cout<<totalArea<<endl;
	// cout<<computePartitionArea(0)<<endl;
	// cout<<computePartitionArea(1)<<endl;
	// cout<<computePartitionArea(0)+computePartitionArea(1)<<endl;
	readhgrFile("ibm01\\ibm01.hgr");
	createGainBucket();
	initcutSz=computeCutSize();
	mincutSz=initcutSz;
	moveCells();
	cout<<"initial cutset size: "<<initcutSz<<endl;
	cout<<"min cutset size: "<<mincutSz<<endl;
	cout<<"max gain is: "<<initcutSz-mincutSz<<endl;
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
				push_heap(sheap.begin(),sheap.end()) ;
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
	totalArea=computeTotalArea();
	long areaCal=0;
	long areaA=(ratio_factor) * (totalArea);
	int areaindex = 0;
	for(int i=0;i<size;i++)
	{
		string id=cellIds.at(i).c_str();
		areaCal+=cellData[id].area;

			if (areaCal>=areaA)
			{
			   areaindex = i-1;
				 break;
			}
	}
	for(int j=0;j<size;j++)
	{
			if(j<=areaindex)
			{
				cellData[cellIds.at(j)].partition=0;
			}
			else{
				cellData[cellIds.at(j)].partition=1;
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
		//grab all nodes or cells from that net, using netid
		vector<string> net=netNodeMap[netList.at(i)];
		int netSp=0;
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
		if(netSp==0)
		{
			fs++;
		}
	}
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
//compute the total area for all cells
int computeTotalArea()
{
	int tarea=0;
	for(int i=0;i<cellIds.size();i++)
	{
		string id=cellIds.at(i).c_str();
		tarea+=cellData[id].area;
	}
	return tarea;
	// return(computePartitionArea(0)+computePartitionArea(1));
}
//pop heap top node to get max area
int cellMaxArea()
{
	int max=sheap.front();
	// pop_heap(sheap.begin(),sheap.end());
	return max;
}
//check the area criterion using given ratio factor
bool checkAreaConstraint()
{
		int p0=computePartitionArea(0);
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
//compute cut size
int computeCutSize()
{
		int cutSize=0;
		for(int i=0;i<nNets;i++)
		{
				string initNode=netNodeMap[i].at(0);
				for(int j=1;j<netNodeMap[i].size();j++)
				{
						if(cellData[initNode].partition!=cellData[netNodeMap[i].at(j)].partition)
						{
								cutSize++;
								break;
						}
				}
		}
		return cutSize;
}
//gain bucket is a map where each key which is gain contains a list of nodes
//which have that gain
void createGainBucket()
{
	//clear all unwanted data if already present
	gainBucket.clear();
	for(int i=0;i<cellIds.size();i++)
	{
		string cellId=cellIds.at(i).c_str();
		// proceed if not locked
		if(cellData[cellId].getLockStatus()==false)
		{
				int gain=computeGain(cellId);
				cellData[cellId].gain=gain;
				map<int,vector<string> >::iterator pos;
				pos=gainBucket.find(gain);
				//if gain not found, create new list and add cell to it
				if(pos==gainBucket.end())
				{
					vector<string> cellist;
					cellist.push_back(cellId);
					gainBucket.insert(pair<int,vector<string> >(gain,cellist));
				}
				//if gain found just append cell to it
				else{
					pos->second.push_back(cellId);
				}
		}
	}
}
void updateGainBucket(string cellId)
{
		std::vector<int> netlist=cellData[cellId].netList;
		//grab every netid the cell is part of
		for(int i=0; i<netlist.size();i++)
		{
				std::vector<string> net=netNodeMap[netlist.at(i)];
				//access every other cell in that net
				for(int j=0;j<net.size();j++)
				{
						string node=net.at(j);
						//we are accessing every other node
						if(cellId!=node)
						{
							if(cellData[node].getLockStatus()==false)
							{
								//get the gain of every node
								int ngain=cellData[node].gain;
								//find the position of the node in the gain bucket and remove it
								// std::vector<string>::iterator pos=gainBucket[ngain].find(node);
								for(int k=0;k<gainBucket[ngain].size();k++)
								{
									if(gainBucket[ngain].at(k)==node)
									{
										gainBucket[ngain].erase(gainBucket[ngain].begin()+k);

										//compute new gain and add it to the new position in the gain bucket
										int newGain=computeGain(node);
										cellData[node].gain=newGain;
										gainBucket[newGain].push_back(node);
										break;
									}
								}
							}
						}
				}
		}

}
//moving cell from one partition to partition
void moveCells()
{
		//starting from highest gain
		map<int,vector<string> >::reverse_iterator it1=gainBucket.rbegin();
		while(it1 !=gainBucket.rend())
		{
			string cellId="";
			cout<<" Gain: "<<it1->first<<endl;
			bool flag=false;
			//starting iteration for all nodes having said gain
			for(int i=0;i<(it1->second).size();i++)
			{
					string node=(it1->second).at(i);
					if(cellData[node].getLockStatus()==false)
					{
							cout<<"changing partition of cell: "<<node<<endl;
							cout<<"from partion: "<<cellData[node].partition<<endl;
							cellData[node].togglePartition();
							cout<<"to new partition: "<<cellData[node].partition<<endl;
							if(checkAreaConstraint()==false)
							{
								cellData[node].togglePartition();
								(it1->second).erase((it1->second).begin()+i);
								cout<<"reverting partition as area constraints are not met"<<endl;
								continue;
							}
							else
							{
									//locking the node
									cellData[node].setLockStatus(true);
									cellId=node;
									cout<<"cell is now locked"<<endl;
									int cs=computeCutSize();
									if(cs<mincutSz)
									{
										mincutSz=cs;
									}
									else if(cs>mincutSz)
									{
										return;
									}
									cout<<"cut set size after move: "<<cs<<endl;
									gainBucket[it1->first].erase(find(gainBucket[it1->first].begin(),gainBucket[it1->first].end(),node));
									flag=true;
									break;
							}
					}
			}
			if(flag==true)
			{
				updateGainBucket(cellId);
				it1=gainBucket.rbegin();
			  	continue;
			}
			it1++;
		}
}