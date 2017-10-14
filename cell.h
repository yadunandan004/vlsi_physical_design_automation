#include <string>
class Cell{
public:
	std::string Id; //cell id
	int partition;// partition which the cell belongs to
	bool locked;//is cell locked or not
	int gain;// gain of the cell
	int area;
	std::vector<int> netList;
	//set cell lock status
	bool getLockStatus(){
		return locked;
	}
	//set cell lock status between true and false
	void setLockStatus(bool status){
		locked=status;
	}
	//toggle partition from 1 to 0
	void togglePartition(){
		partition=partition^1;
	}
};
