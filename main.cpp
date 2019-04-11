#include "manager.cpp"

int main() 
{
    std::vector<std::pair<pcb*, pcb*> > creationTree;
	std::vector<pcb*> rdy_list, waitingList, user_level, system_level;
    
    rcb R1, R2, R3, R4;
    R1.rid = "R1"; R1.counter = std::pair<int, int>(1, 1);
    R2.rid = "R2"; R2.counter = std::pair<int, int>(2, 2);
    R3.rid = "R3"; R3.counter = std::pair<int, int>(3, 3);
    R4.rid = "R4"; R4.counter = std::pair<int, int>(4, 4);
	std::vector<rcb*> resourceList;
    resourceList.push_back(&R1);
    resourceList.push_back(&R2);
    resourceList.push_back(&R3);
    resourceList.push_back(&R4);
    std::string s;
    std::cout << "Enter path: " << std::endl;
    std::getline(std::cin, s);
    s[s.length()-1] = '\0';

	pcb init;
	init.id = "init";
	init.priority = 0;
	init.parent = NULL;
	rdy_list.push_back(&init);
	std::string runningPid = "init";

	shell(rdy_list, waitingList, runningPid, creationTree, 
		resourceList, user_level, system_level, init, s.c_str()); 

	return 0;
}
