#include <iostream>
#include <fstream>
#include "manager.hpp"


void clear_vector(std::vector<pcb*> &deleteVector)
{
	std::vector<pcb*>::iterator it;

    for (int i = 0; i < deleteVector.size(); i++)
    {
        if (deleteVector[i]->id != "init")
            delete(deleteVector[i]);
    }
}


void reset_sys(std::string &running, std::vector<pcb*> &rdy_list, std::vector<pcb*> &waitingList,
	std::vector<pcb*> &user_level, std::vector<pcb*> &system_level, std::vector<rcb*> &resources, 
	std::vector<std::pair<pcb*, pcb*> > &creationTree, pcb &init, bool &firstLine)
{
	running = "init";
	user_level.clear();
	system_level.clear();
	clear_vector(waitingList);
	clear_vector(rdy_list);
	creationTree.clear();
	rdy_list.push_back(&init);

    for (int i = 0, sz = resources.size(); i < sz; i++)
    {
        resources[i]->counter = std::pair<int, int> (resources[i]->counter.first, resources[i]->counter.first);
        resources[i]->waiting_list.clear();
    }

	write_to_file("\ninit");
}

pcb* find_pcb(std::string pid, std::vector<pcb*> &rdy_list, std::vector<pcb*> &waitingList)
{
    for (int i = 0, sz = rdy_list.size(); i < sz; i++)
    {
        if (rdy_list[i]->id == pid)
            return rdy_list[i];
    }
    for (int i = 0, sz = waitingList.size(); i < sz; i++)
    {
        if (waitingList[i]->id == pid)
            return waitingList[i];
    }

	return NULL;
}

rcb* find_rsc(std::string name, std::vector<rcb*> &resourceList)
{
    for (int i = 0, sz= resourceList.size(); i < sz; i++)
    {
        if (resourceList[i]->rid == name)
            return resourceList[i];
    }

    return NULL;
}

pcb* get_highest_prio(std::vector<pcb*> &user_level, std::vector<pcb*> &system_level, pcb &init)
{
    int sz;
    if ((sz = system_level.size()) != 0)
    {
        for(int i =0;i < sz ;i++ )
        {
            if (system_level[i]->status.first == "ready")
                return system_level[i];
        }
    }

    if ((sz = user_level.size()) != 0)
    {
        for(int i =0;i < sz ;i++ )
        {
            if (user_level[i]->status.first == "ready")
                return user_level[i];
        }
    }

	return &init;
}

bool has_rsc(pcb &process, rcb &resource)
{

    for (int i = 0, sz = process.other_rsc.size(); i < sz; i++)
    {
        if (process.other_rsc[i].first->rid == resource.rid)
            return true;
    }
	return false;
}

int check_counter_holding(pcb &process, std::string resourceName)
{
    for (int i = 0, sz = process.other_rsc.size(); i < sz; i++)
    {
        if (process.other_rsc[i].first->rid == resourceName)
            return process.other_rsc[i].second;
    }
	return 0;
}

bool valid_cr(std::string name, std::string priority, std::vector<pcb*> &rdy_list, 
	std::vector<pcb*> &waitingList)
{
    return ((stoi(priority) <= 0 || stoi(priority) > 2 || find_pcb(name, rdy_list, waitingList)) ? false : true);
}

bool valid_de(pcb &del, std::string &running)
{
	if (del.id == "init")
	{
		return false;
	}

	if (del.id == running || del.parent->id == running)
	{
		return true;
	}

	return valid_de((*del.parent), running);
}

bool valid_req(pcb &reqProcess, rcb &reqrcb, std::string unit)
{
	int u = stoi(unit);

	if (has_rsc(reqProcess, reqrcb))
	{
		if (reqrcb.counter.second - u < 0)
		{
			return false;
		}
	}

	if (reqProcess.id == "init" || u <= 0 || u > 4)
	{
		return false;
	}

	if (u > (reqrcb.counter).first)
	{
		return false;
	}

	return true;
}

bool valid_rel(pcb &relProcess, std::string resourceName, std::string unit, 
	std::vector<rcb*> &resourceList)
{
	int u = stoi(unit);

	if (relProcess.id == "init" || u <= 0 || u > 4)
	{
		return false;
	}

	rcb* relrcb = find_rsc(resourceName, resourceList);
	
	if ( !has_rsc(relProcess, *relrcb) || (relrcb->counter.second+u) > (relrcb->counter.first))
	{
		return false;
	}

	int processHas = check_counter_holding(relProcess, resourceName);
	if (processHas < u)
	{
		return false;
	}

	return true;
}

void del_child(pcb &parentpcb, std::string childName)
{
    for(int i =0, sz = parentpcb.children.size(); i<sz ;i++)
    {
        if (parentpcb.children[i]->id == childName)
        {
            parentpcb.children.erase(parentpcb.children.begin()+i);
            return;
        }
    }
}

void rmv_link(std::string parent_pid, std::string child_pid, 
	std::vector<std::pair<pcb*, pcb*> > &creationTree)
{
    for (int i = 0, sz = creationTree.size(); i < sz; i++)
    {
        if (creationTree[i].first->id == parent_pid && creationTree[i].second->id == child_pid)
        {
            creationTree.erase(creationTree.begin()+i);
            return;
        }
    }
}

void priority_list_rmv(std::string pid, std::vector<pcb*> &user_level, std::vector<pcb*> &system_level)
{
    for(int i = 0, sz = system_level.size(); i<sz ;i++)
    {
        if (system_level[i]->id == pid)
        {
            system_level.erase(system_level.begin()+i);
            return;
        }
    }
    for(int i = 0, sz = user_level.size(); i<sz ;i++)
    {
        if (user_level[i]->id == pid)
        {
            user_level.erase(user_level.begin()+i);
            return;
        }
    }

}

void list_rmv(std::string pid, std::vector<pcb*> &rdy_list, std::vector<pcb*> &waitingList,
 std::vector<pcb*> &user_level, std::vector<pcb*> &system_level, std::vector<rcb*> &resourceList)
{

	std::vector<rcb*>::iterator itR;

	for (itR = resourceList.begin(); itR < resourceList.end(); ++itR)
	{
		std::vector<std::pair<pcb*, int> >::iterator itRWait;
		for ( itRWait = (*itR) -> waiting_list.begin(); itRWait < (*itR) -> waiting_list.end(); ++itRWait)
		{
			if ((*itRWait).first -> id == pid)	
			{
				(*itR) -> waiting_list.erase(itRWait);
			}
		}	
	}

	std::vector<pcb*>::iterator it;

	for ( it = rdy_list.begin(); it < rdy_list.end(); ++it)
	{
		if ( (*it) -> id == pid )
		{
			rdy_list.erase(it);
			priority_list_rmv(pid, user_level, system_level);
			return;
		}
	}

	for ( it = waitingList.begin() ; it < waitingList.end(); ++it)
	{
		if ( (*it) -> id == pid )
		{
			waitingList.erase(it);
			priority_list_rmv(pid, user_level, system_level);
			return;
		} 
	}
}

void check_waitlist(pcb &waiting_listProcess, rcb &resource, int counter, std::vector<pcb*> &rdy_list, 
	std::vector<pcb*> &waitingList, std::vector<pcb*> &user_level, std::vector<pcb*> &system_level,
	std::vector<rcb*> &resourceList)
{
	std::vector<std::pair<pcb*, int> >::iterator it;
	int counterWanted;

	for (it = resource.waiting_list.begin(); it < resource.waiting_list.end(); ++it)
	{
		if ((*it).first -> id == waiting_listProcess.id)
		{
			counterWanted = (*it).second;
		}
	}

	if (counterWanted < counter && !(resource.counter.second >= counterWanted))
	{
		resource.counter.second += counter;
	}

	if (resource.counter.second >= counterWanted)
	{
		resource.waiting_list.erase(resource.waiting_list.begin()+it->second);
		waiting_listProcess.status.first = "ready";
		waiting_listProcess.status.second = "ready";
		resource.counter.second -= counterWanted;
		waiting_listProcess.other_rsc.push_back(std::pair<rcb*, int>(&resource, counterWanted));

		list_rmv(waiting_listProcess.id, rdy_list, waitingList, user_level, system_level, resourceList);
		rdy_list.push_back(&waiting_listProcess);

		if (waiting_listProcess.priority == 1)
		{
			user_level.push_back(&waiting_listProcess);
		}

		else if (waiting_listProcess.priority == 2)
		{
			system_level.push_back(&waiting_listProcess);
		}
	}
}

void scheduler(std::string &running, std::vector<pcb*> &rdy_list, std::vector<pcb*> & waitingList, 
	std::vector<pcb*> &user_level, std::vector<pcb*> &system_level, pcb &init)
{
	pcb* runningProcess = find_pcb(running, rdy_list, waitingList);
	pcb* highestPriorityProcess = get_highest_prio(user_level, system_level, init);

	if ((runningProcess == NULL) || (runningProcess -> priority < highestPriorityProcess -> priority) 
		|| (runningProcess -> status).first == "blocked")
	{
		running = highestPriorityProcess -> id;
	}
	write_to_file(highestPriorityProcess -> id);
}

void rel_rsc(std::string &running, std::string resourceName, std::string counter,
	std::vector<pcb*> &rdy_list, std::vector<pcb*> &waitingList, 
	std::vector<rcb*> &resourceList, std::vector<pcb*> &user_level, 
	std::vector<pcb*> &system_level, pcb &init, bool shouldSchedule)
{

	if (resourceName != "R1" && resourceName != "R2" && resourceName != "R3" && resourceName != "R4")
	{
		write_to_file("error");
		return;
	}

	pcb *relProcess = find_pcb(running, rdy_list, waitingList); 
	int counterRequested = stoi(counter);
	
	if (!valid_rel(*relProcess, resourceName, counter, resourceList))
	{
		write_to_file("error");
		return;
	}

	rcb* relrcb = find_rsc(resourceName, resourceList);

	int processHas = check_counter_holding(*relProcess, (*relrcb).rid);

	if (processHas > counterRequested)
	{
		std::vector<std::pair<rcb*, int> >::iterator itR;
		for (itR = (relProcess -> other_rsc).begin(); itR < (relProcess -> other_rsc).end(); ++itR)
		{
			if ((*itR).first -> rid == resourceName)
			{
				(*itR).second -= counterRequested;
			}
		}
	}

	else if (processHas == counterRequested)
	{
		std::vector<std::pair<rcb*, int> >::iterator it;
		for (it = relProcess -> other_rsc.begin(); it < relProcess -> other_rsc.end(); ++it)
		{
			if ((*it).first -> rid == relrcb -> rid)
			{
				relProcess -> other_rsc.erase(it);
				break;
			}
		}
	}

	relrcb -> counter.second += counterRequested;

	if (!(relrcb -> waiting_list.empty()))
	{
		pcb* waitingProcess = ((relrcb -> waiting_list).front()).first;
		check_waitlist(*waitingProcess, *relrcb, counterRequested, rdy_list, waitingList, 
			user_level, system_level, resourceList);
	}

	if (shouldSchedule)
	{
		scheduler(running, rdy_list, waitingList, user_level, system_level, init);
	}
}

void rel_all(pcb &process, std::vector<pcb*> &rdy_list, std::vector<pcb*> &waitingList,
	std::vector<rcb*> &resourceList, std::vector<pcb*> &user_level, std::vector<pcb*> &system_level, pcb &init)
{
	std::vector<std::pair<rcb*, int> >::iterator it;

	for (it = process.other_rsc.begin(); it < process.other_rsc.end(); ++it)
	{
		(*it).first -> counter.second += (*it).second;

		if (!((*it).first -> waiting_list.empty()))
		{
			pcb* waitingProcess = (((*it).first -> waiting_list).front()).first;

			check_waitlist(*waitingProcess, *((*it).first), (*it).first -> counter.second, rdy_list, waitingList, 
				user_level, system_level, resourceList);

		}
	}

	process.other_rsc.clear();
}

void del_tree(std::string &running, pcb &deletepcb, std::vector<std::pair<pcb*, pcb*> > &creationTree, 
	std::vector<pcb*> &rdy_list, std::vector<pcb*> &waitingList, std::vector<rcb*> &resourceList, 
	std::vector<pcb*> &user_level, std::vector<pcb*> &system_level, pcb &init)
{
	std::vector<pcb*>::iterator it;
	if (deletepcb.id == running)
	{
		del_child(*deletepcb.parent, deletepcb.id);
	}

	for (it = deletepcb.children.begin(); it < deletepcb.children.end(); ++it)
	{
		del_tree(running, *(*it), creationTree, rdy_list, waitingList, resourceList, 
			user_level, system_level, init);
	} 

	rel_all(deletepcb, rdy_list, waitingList, resourceList, user_level, system_level, init);

	rmv_link((deletepcb.parent) -> id, deletepcb.id, creationTree);

	list_rmv(deletepcb.id, rdy_list, waitingList, user_level, system_level, resourceList);
}

void give_rsc(pcb &reqProcess, rcb &resource, int counter)
{
	if (has_rsc(reqProcess, resource))
	{
		std::vector<std::pair<rcb*, int> >::iterator it;
		for (it = reqProcess.other_rsc.begin(); it < reqProcess.other_rsc.end(); ++it)
		{
			if ((*it).first -> rid == resource.rid)
			{
				(*it).second += counter;
			}
		}
	}

	else
	{
		reqProcess.other_rsc.push_back(std::pair<rcb*, int>(&resource, counter));
	}

	resource.counter.second -= counter;
}

void block_rsc_req(pcb &reqProcess, rcb &resource, int counter, 
	std::vector<pcb*> &rdy_list, std::vector<pcb*> &waitingList, std::vector<pcb*> &user_level, 
	std::vector<pcb*> &system_level, std::vector<rcb*> &resourceList)
{
	reqProcess.status = std::pair<std::string, std::string>("blocked", "waiting");
	list_rmv(reqProcess.id, rdy_list, waitingList, user_level, system_level, resourceList);
	waitingList.push_back(&reqProcess);

	if (reqProcess.priority == 1)
	{
		user_level.push_back(&reqProcess);
	}

	else if (reqProcess.priority == 2)
	{
		system_level.push_back(&reqProcess);
	}

	resource.waiting_list.push_back(std::pair<pcb*, int>(&reqProcess, counter));
}

void req_rsc(std::string &running, std::string resourceName, std::string counter,
	std::vector<pcb*> &rdy_list, std::vector<pcb*> &waitingList, std::vector<rcb*> &resourceList, 
	std::vector<pcb*> &user_level, std::vector<pcb*> &system_level, pcb &init)
{
	if (resourceName != "R1" && resourceName != "R2" && resourceName != "R3" && resourceName != "R4")
	{
		write_to_file("error");
		return;
	}
	
	pcb *reqProcess = find_pcb(running, rdy_list, waitingList); 
	rcb *reqrcb = find_rsc(resourceName, resourceList);
	int counterRequested = stoi(counter);

	if (!valid_req(*reqProcess, *reqrcb, counter))
	{
		write_to_file("error");
		return;
	}

	if (reqrcb -> counter.second >= counterRequested)
	{
		give_rsc(*reqProcess, *reqrcb, counterRequested);
	}

	else
	{
		block_rsc_req(*reqProcess, *reqrcb, counterRequested, rdy_list, waitingList, 
			user_level, system_level, resourceList);
	}

	scheduler(running, rdy_list, waitingList, user_level, system_level, init);
}

void timeout(std::string &running, std::vector<pcb*> &rdy_list, std::vector<pcb*> &waitingList, 
	std::vector<pcb*> &user_level, std::vector<pcb*> &system_level, pcb &init, std::vector<rcb*> &resourceList)
{
	pcb* runningProcess = find_pcb(running, rdy_list, waitingList);
	list_rmv(runningProcess -> id, rdy_list, waitingList, user_level, system_level, resourceList);
	runningProcess -> status.first = "ready";
	runningProcess -> status.second = "ready";
	rdy_list.push_back(runningProcess);

	if (runningProcess -> priority == 1)
	{
		user_level.push_back(runningProcess);
	}

	else if (runningProcess -> priority == 2)
	{
		system_level.push_back(runningProcess);
	}

	running = "nullhptr123erts";

	scheduler(running, rdy_list, waitingList, user_level, system_level, init);
}

void cr_pcb(std::string &running, std::string name, 
	std::string priority, std::vector<std::pair<pcb*, pcb*> > &creationTree, 
	std::vector<pcb*> &rdy_list, std::vector<pcb*> &waitingList, 
	std::vector<pcb*> &user_level, std::vector<pcb*> &system_level, pcb &init)
{
	if (! valid_cr(name, priority, rdy_list, waitingList))
	{
		write_to_file("error");
		return;
	}

	pcb* p = new pcb;
	p -> id = name;
	pcb* parent = find_pcb(running, rdy_list, waitingList);

	p -> parent = parent;
	p -> priority = std::stoi(priority);

	p -> status = std::pair<std::string, std::string>("ready", "ready");
	
	// add child to parent
	(parent -> children).push_back(p);

	rdy_list.push_back(p);

	creationTree.push_back(std::pair<pcb*, pcb*>(parent, p));
	if (p -> priority == 1)
	{
		user_level.push_back(p);
	}

	else if (p -> priority == 2)
	{
		system_level.push_back(p);
	}

	scheduler(running, rdy_list, waitingList, user_level, system_level, init);
}
 
void de_pcb(std::string &running, std::string name,
	std::vector<std::pair<pcb*, pcb*> > &creationTree, std::vector<pcb*> &rdy_list,
	std::vector<pcb*> &waitingList, std::vector<rcb*> &resourceList, 
	std::vector<pcb*> &user_level, std::vector<pcb*> &system_level, pcb &init)
{
	pcb* deletepcb = find_pcb(name, rdy_list, waitingList);
	if ( deletepcb == NULL)    // Process that user wants to destroy was not created at all.
	{
		write_to_file("error");
		return;
	}

	if (!valid_de(*deletepcb, running))
	{
		write_to_file("error");
		return;
	}
	del_tree(running, *deletepcb, creationTree, rdy_list, waitingList, resourceList, 
		user_level, system_level, init);
	scheduler(running, rdy_list, waitingList, user_level, system_level, init);
}

void shell(std::vector<pcb*> &readys, std::vector<pcb*> &waitings,
	std::string &running, std::vector<std::pair<pcb*, pcb*> > &creationTree, 
	std::vector<rcb*> &resources, std::vector<pcb*> &user_level, 
	std::vector<pcb*> &system_level, pcb &init, std::string fileName)
{
	std::string line;
	std::ifstream inputFile(fileName);
	bool firstLine = true;

	if (inputFile.is_open())
	{
		while (std::getline(inputFile, line))
		{
			if (firstLine)
			{
				write_to_file("init");
				firstLine = false;
			}

			std::vector<std::string> cmd;
            std::string delimiter =" ";
            std::string token;
            size_t pos = 0;
            while ((pos = line.find(delimiter)) != std::string::npos)
            {
                token = line.substr(0,pos);
                line.erase(0, pos + delimiter.length());
                cmd.push_back(token);
            }
            cmd.push_back(line);
        
			if (cmd[0] == "cr")
			{
				std::string pid = cmd[1];
				std::string priority = cmd[2];

				cr_pcb(running, pid, priority, creationTree, readys, waitings, 
					user_level, system_level, init);
			}

			else if (cmd[0] == "de")
			{
				std::string deletePid = cmd[1];
				de_pcb(running, deletePid, creationTree, readys, waitings, resources, 
					user_level, system_level, init);
			}

			else if (cmd[0] == "req")
			{
				std::string reqRid = cmd[1];
				std::string counter = cmd[2];

				req_rsc(running, reqRid, counter, readys, waitings, resources, 
					user_level, system_level, init);
			}

			else if (cmd[0] == "rel")
			{
				std::string relRid = cmd[1]; 
				std::string counter = cmd[2];

				rel_rsc(running, relRid, counter, readys, waitings, resources, 
					user_level, system_level, init);
			}

			else if (cmd[0] == "to")
			{
				timeout(running, readys, waitings, user_level, system_level, init, resources);
			}

			else if (cmd[0] == "init")
			{
				reset_sys(running, readys, waitings, user_level, system_level, 
					resources, creationTree, init, firstLine);
			}
		}
	}
	inputFile.close();
}


void write_to_file(std::string message, bool newLine)
{
	std::ofstream out;
	out.open("31238954.txt",std::ios::app);
    if (message == "\n")
        out<< message;
    else
		out<< message << " ";
	out.close();
}
