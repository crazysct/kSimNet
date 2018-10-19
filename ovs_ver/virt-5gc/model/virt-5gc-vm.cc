/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

#include "ns3/log.h"

#include "virt-5gc-vm.h"

namespace ns3 {

	NS_LOG_COMPONENT_DEFINE("Virt5gcVm");

	NS_OBJECT_ENSURE_REGISTERED (Virt5gcVm);

	TypeId Virt5gcVm::GetTypeId(void)
	{
		static TypeId tid = TypeId("ns3::Virt5gcVm")
			.SetParent<Object> ()
			.SetGroupName("Virt5gc");

		return tid;
	}

	Virt5gcVm::Virt5gcVm(int vmId, int torId, int pmId)
	{
		id = vmId;
		ToR = torId;
		pm = pmId;
		NS_LOG_FUNCTION (this);
	}

	Virt5gcVm::Virt5gcVm(const Virt5gcVm& oldVm)
	{
		id = oldVm.id;
		ToR = oldVm.ToR;
		pm = oldVm.pm;
		node = oldVm.node;
		cpuSize = oldVm.cpuSize;
		cpuUtil = oldVm.cpuUtil;
		memSize = oldVm.memSize;
		memUtil = oldVm.memUtil;
		diskSize = oldVm.diskSize;
		diskUtil = oldVm.diskUtil;
		bwSize = oldVm.bwSize;
		bwUtil = oldVm.bwUtil;
	}

	
	bool
	Virt5gcVm::operator==(const Virt5gcVm& rhs) const
	{
		return id == rhs.id;
	}

	void
	Virt5gcVm::SetId (int vmId)
	{
		id = vmId;
	}

	void
	Virt5gcVm::SetNodeId (int nodeId)
	{
		node = nodeId;
	}

	void 
	Virt5gcVm::SetCpuInfo (int size, int util)
	{
		cpuSize = size;
		cpuUtil = util;
	}

	void 
	Virt5gcVm::SetMemInfo (int size, int util)
	{
		memSize = size;
		memUtil = util;
	}
	
	void 
	Virt5gcVm::SetDiskInfo (int size, int util)
	{
		diskSize = size;
		diskUtil = util;
	}

	void 
	Virt5gcVm::SetBwInfo (int size, int util)
	{
		bwSize = size;
		bwUtil = util;
	}

	void 
	Virt5gcVm::ChangeCpuLoad (int util)
	{
		cpuUtil = util;
	}

	void 
	Virt5gcVm::ChangeMemLoad (int util)
	{
		memUtil = util;
	}
	
	void 
	Virt5gcVm::ChangeDiskLoad (int util)
	{
		diskUtil = util;
	}

	void
	Virt5gcVm::ChangeToR (int tor)
	{
		ToR = tor;
	}

	void
	Virt5gcVm::ChangePm (int pmId)
	{
		pm = pmId;
	}

	int
	Virt5gcVm::GetVmId (void)
	{
		return id;
	}

	int
	Virt5gcVm::GetToRId (void)
	{
		return ToR;
	}

	int
	Virt5gcVm::GetPmId (void)
	{
		return pm;
	}

	int
	Virt5gcVm::GetNodeId (void)
	{
		return node;
	}

	std::pair<int, int>
	Virt5gcVm::GetCpuInfo (void)
	{
		std::pair<int, int> info (cpuSize, cpuUtil);
		return info;
	}
	
	std::pair<int, int>
	Virt5gcVm::GetMemInfo (void)
	{
		std::pair<int, int> info (memSize, memUtil);
		return info;
	}
	
	std::pair<int, int>
	Virt5gcVm::GetDiskInfo (void)
	{
		std::pair<int, int> info (diskSize, diskUtil);
		return info;
	}

	std::pair<int, int>
	Virt5gcVm::GetBwInfo (void)
	{
		std::pair<int, int> info (bwSize, bwUtil);
		return info;
	}
}
