#ifndef MANAGER_HPP
#define MANAGER_HPP

#include <utility>
#include <string>
#include <vector>

struct rcb;

struct pcb 
{
	std::string id;
	std::vector<std::pair<rcb*, int> > other_rsc;
	std::pair<std::string, std::string> status;
	pcb* parent;
	std::vector<pcb*> children;
	int priority;
};

struct rcb
{
	std::string rid;
	std::pair<int, int> counter;
	std::vector<std::pair<pcb*, int> > waiting_list;
};

void write_to_file(std::string message, bool newLine = false);

void clear_vector(std::vector<pcb*> &deleteVector);

void reset_sys(std::string &running, std::vector<pcb*> &rdy_list, std::vector<pcb*> &waitingList,
	std::vector<pcb*> &user_level, std::vector<pcb*> &system_level, std::vector<rcb*> &resources, 
	std::vector<std::pair<pcb*, pcb*> > &creationTree, pcb &init);

pcb* find_pcb(std::string name, std::vector<pcb*> &rdy_list, std::vector<pcb*> &waitingList);

rcb* find_rcb(std::string name, std::vector<rcb*> &resourceList);

pcb* get_highest_prio(std::vector<pcb*> &user_level, std::vector<pcb*> &system_level, pcb &init);

bool has_rsc(pcb &process, rcb &resource);

int check_units_holding(pcb &process, std::string resourceName);

bool valid_cr(std::string name, std::string priority, std::vector<pcb*> &rdy_list, 
	std::vector<pcb*> &waitingList);

bool valid_de(pcb &deletepcb, std::string &running);

bool valid_req(pcb &reqProcess, rcb &reqrcb, std::string unit);

bool valid_rel(pcb &relProcess, std::string resourceName, std::string unit, 
	std::vector<rcb*> &resourceList);

void del_child(pcb &parentpcb, std::string childName);

void rmv_link(std::string parentName, std::string childName, 
	std::vector<std::pair<pcb*, pcb*> > &creationTree);

void priority_list_rmv(std::string pid, std::vector<pcb*> &user_level, std::vector<pcb*> &system_level);

void list_rmv(std::string pid, std::vector<pcb*> &rdy_list, std::vector<pcb*> &waitingList,
 std::vector<pcb*> &user_level, std::vector<pcb*> &system_level, std::vector<rcb*> &resourceList);

void check_waitlist(pcb &waiting_listProcess, rcb &resource, int counter, std::vector<pcb*> &rdy_list, 
	std::vector<pcb*> &waitingList, std::vector<pcb*> &user_level, std::vector<pcb*> &system_level,
	std::vector<rcb*> &resourceList);

void scheduler(std::string &running, std::vector<pcb*> &rdy_list, std::vector<pcb*> & waitingList, 
	std::vector<pcb*> &user_level, std::vector<pcb*> &system_level, pcb &init);

void rel_rsc(std::string &running, std::string resourceName, std::string counter,
	std::vector<pcb*> &rdy_list, std::vector<pcb*> &waitingList, 
	std::vector<rcb*> &resourceList, std::vector<pcb*> &user_level, 
	std::vector<pcb*> &system_level, pcb &init, bool shouldSchedule = 1);

void rel_all(pcb &process, std::vector<pcb*> &rdy_list, std::vector<pcb*> &waitingList,
	std::vector<rcb*> &resourceList, std::vector<pcb*> &user_level, std::vector<pcb*> &system_level, pcb &init);

void del_tree(std::string &running, pcb &deletepcb, std::vector<std::pair<pcb*, pcb*> > &creationTree, 
	std::vector<pcb*> &rdy_list, std::vector<pcb*> &waitingList, std::vector<rcb*> &resourceList, 
	std::vector<pcb*> &user_level, std::vector<pcb*> &system_level, pcb &init);

void give_rsc(pcb &reqProcess, rcb &resource, int counter);

void block_rsc_req(pcb &reqProcess, rcb &resource, int counter, 
	std::vector<pcb*> &rdy_list, std::vector<pcb*> &waitingList, std::vector<pcb*> &user_level, 
	std::vector<pcb*> &system_level, std::vector<rcb*> &resourceList);

void req_rsc(std::string &running, std::string resourceName, std::string counter,
	std::vector<pcb*> &rdy_list, std::vector<pcb*> &waitingList, std::vector<rcb*> &resourceList, 
	std::vector<pcb*> &user_level, std::vector<pcb*> &system_level, pcb &init);

void timeout(std::string &running, std::vector<pcb*> &rdy_list, std::vector<pcb*> &waitingList, 
	std::vector<pcb*> &user_level, std::vector<pcb*> &system_level, pcb &init, std::vector<rcb*> &resourceList);

void cr_pcb(std::string &running, std::string name, 
	std::string priority, std::vector<std::pair<pcb*, pcb*> > &creationTree, 
	std::vector<pcb*> &rdy_list, std::vector<pcb*> &waitingList, 
	std::vector<pcb*> &user_level, std::vector<pcb*> &system_level, pcb &init);

void de_pcb(std::string &running, std::string name,
	std::vector<std::pair<pcb*, pcb*> > &creationTree, std::vector<pcb*> &rdy_list,
	std::vector<pcb*> &waitingList, std::vector<rcb*> &resourceList, 
	std::vector<pcb*> &user_level, std::vector<pcb*> &system_level, pcb &init);

void shell(std::vector<pcb*> &readys, std::vector<pcb*> &waitings,
	std::string &running, std::vector<std::pair<pcb*, pcb*> > &creationTree, 
	std::vector<rcb*> &resources, std::vector<pcb*> &user_level, 
	std::vector<pcb*> &system_level, pcb &init, std::string fileName);

#endif
