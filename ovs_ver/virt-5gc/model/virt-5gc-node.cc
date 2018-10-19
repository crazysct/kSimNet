/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

#include "ns3/log.h"

#include "virt-5gc-node.h"

namespace ns3 {

	NS_LOG_COMPONENT_DEFINE("Virt5gcNode");

	NS_OBJECT_ENSURE_REGISTERED (Virt5gcNode);

	TypeId Virt5gcNode::GetTypeId(void)
	{
		static TypeId tid = TypeId("ns3::Virt5gcNode")
			.SetParent<Object> ()
			.SetGroupName("Virt5gc");

		return tid;
	}

	Virt5gcNode::Virt5gcNode(int id, int x, int y, int component)
	{
		node_id = id;
		node_x = x;
		node_y = y;
		node_component = component;
		cpuSize = 0;
		cpuUtil = 0;
		memSize = 0;
		memUtil = 0;
		diskSize = 0;
		diskUtil = 0;
		bwSize = 0;
		bwUtil = 0;
		//node_vms = NULL;
		NS_LOG_FUNCTION (this);
	}

	/*
	bool
	Virt5gcNode::operator<(const Virt5gcNode& rhs) const
	{
		return node_id < rhs.node_id;
	}
	*/

	int
	Virt5gcNode::GetId (void)
	{
		return node_id;
	}
	
	std::pair<int, int>
	Virt5gcNode::GetCoord (void)
	{
		std::pair<int, int> tempPair(node_x, node_y);
		return tempPair;
	}

	int
	Virt5gcNode::GetComponent (void)
	{
		return node_component;
	}

	std::list<int>
	Virt5gcNode::GetVms(void)
	{
		return node_vms;
	}

	std::pair<int, int>
	Virt5gcNode::GetCpuInfo(void)
	{
		std::pair<int, int> tempPair(cpuSize, cpuUtil);
		return tempPair;
	}

	std::pair<int, int>
	Virt5gcNode::GetMemInfo(void)
	{
		std::pair<int, int> tempPair(memSize, memUtil);
		return tempPair;
	}

	std::pair<int, int>
	Virt5gcNode::GetDiskInfo(void)
	{
		std::pair<int, int> tempPair(diskSize, diskUtil);
		return tempPair;
	}

	std::pair<int, int>
	Virt5gcNode::GetBwInfo(void)
	{
		std::pair<int, int> tempPair(bwSize, bwUtil);
		return tempPair;
	}

	void
	Virt5gcNode::SetVm (int vm)
	{
		node_vms.push_back(vm);
	}

	void
	Virt5gcNode::DeleteVm (int vm)
	{
		node_vms.remove(vm);
	}

	bool
	Virt5gcNode::FindVm (int vm)
	{
		std::list<int>::iterator itor;
		for (itor = node_vms.begin(); itor != node_vms.end(); itor++) {
			if (*itor == vm)
				return true;
		}
		return false;
	}

	void
	Virt5gcNode::SetCpuInfo(int size, int util)
	{
		cpuSize = size;
		cpuUtil = util;
	}

	void
	Virt5gcNode::SetMemInfo(int size, int util)
	{
		memSize = size;
		memUtil = util;
	}

	void
	Virt5gcNode::SetDiskInfo(int size, int util)
	{
		diskSize = size;
		diskUtil = util;
	}

	void
	Virt5gcNode::SetBwInfo(int size, int util)
	{
		bwSize = size;
		bwUtil = util;
	}

	void
	Virt5gcNode::ChangeCpuLoad(int util)
	{
		cpuUtil = util;
	}

	void
	Virt5gcNode::ChangeMemLoad(int util)
	{
		memUtil = util;
	}

	void
	Virt5gcNode::ChangeDiskLoad(int util)
	{
		diskUtil = util;
	}


}
