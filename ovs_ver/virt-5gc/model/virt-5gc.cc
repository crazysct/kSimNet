/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

#include "ns3/log.h"

#include "virt-5gc.h"

namespace ns3 {

	NS_LOG_COMPONENT_DEFINE("Virt5gc");

	NS_OBJECT_ENSURE_REGISTERED (Virt5gc);

	TypeId Virt5gc::GetTypeId(void)
	{
		static TypeId tid = TypeId("ns3::Virt5gc")
			.SetParent<Object> ()
			.SetGroupName("Virt5gc")
			.AddTraceSource ("ScalingDelay", 
					"pass scaling delay", 
					MakeTraceSourceAccessor (&Virt5gc::m_scalingTrace),
					"ns3::TracedValueCallback::Uint32");
		return tid;
	}

	Virt5gc::Virt5gc()
	{
		InitVirt5gc();
		NS_LOG_FUNCTION (this);
	}


	void
	Virt5gc::InitVirt5gc ()
	{
		totAttr = 4;
		pgwN = 0;
		mmeN = 0;
		ueN = 0;
		enbN = 0;
		loadStd = 0;
		allocDelay = 0;
		scaleInRate = 0;
		scaleOutRate = 0;
		mmeVmN = 0;
		pgwVmN = 0;
	}


	void 
	Virt5gc::SetInputFile (const std::string &fileName)
	{
		m_inputFile = fileName;
	}

	void
	Virt5gc::SetTopoFile (const std::string &fileName)
	{
		m_topoFile = fileName;
	}


	std::string
	Virt5gc::GetInputFile (void)
	{
		return m_inputFile;
	}

	void
	Virt5gc::ReadVm (void)
	{
		std::ifstream inputgen;
		inputgen.open (GetInputFile().c_str());

		if (!inputgen.is_open())
		{
			NS_LOG_WARN("Cannot open an Virt5gc input file");
		}


		std::istringstream lineBuffer;
		std::string line;

		//NS_LOG_INFO ("Virt5gc input should have " << totAttr << "attributes");
		int vmId, torId, pmId, cSize, cUtil, mSize, mUtil, dSize, dUtil, bSize, bUtil;
		while (true) {

			// get ToR
			this->getline (inputgen, line);
			lineBuffer.clear();
			lineBuffer.str(line);
			lineBuffer >> vmId;

			if (inputgen.eof())
				break;

			this->getline (inputgen, line);
			lineBuffer.clear();
			lineBuffer.str(line);
			lineBuffer >> torId;

			this->getline (inputgen, line);
			lineBuffer.clear();
			lineBuffer.str(line);
			lineBuffer >> pmId;

			Virt5gcVm tempVm (vmId, torId, pmId);

			// get Cpu info
			this->getline (inputgen, line);
			lineBuffer.clear();
			lineBuffer.str(line);

			lineBuffer >> cSize;
			lineBuffer >> cUtil;
			tempVm.SetCpuInfo(cSize, cUtil);

			// get Mem info
			this->getline (inputgen, line);
			lineBuffer.clear();
			lineBuffer.str(line);

			lineBuffer >> mSize;
			lineBuffer >> mUtil;
			tempVm.SetMemInfo(mSize, mUtil);

			// get Disk info
			this->getline (inputgen, line);
			lineBuffer.clear();
			lineBuffer.str(line);

			lineBuffer >> dSize;
			lineBuffer >> dUtil;
			tempVm.SetDiskInfo(dSize, dUtil);

			// get B/W info
			this->getline (inputgen, line);
			lineBuffer.clear();
			lineBuffer.str(line);

			lineBuffer >> bSize;
			lineBuffer >> bUtil;
			tempVm.SetBwInfo(bSize, bUtil);
	
			// Set nodeID
			std::list<Virt5gcNode>::iterator itor;
			for (itor = nodeList.begin(); itor != nodeList.end(); itor++) {
				if ((*itor).FindVm(vmId)) {
					tempVm.SetNodeId((*itor).GetId());

					(*itor).SetCpuInfo(cSize, cUtil);
					(*itor).SetMemInfo(mSize, mUtil);
					(*itor).SetDiskInfo(dSize, dUtil);
					(*itor).SetBwInfo(bSize, bUtil);
					break;
				}
			}
			vmList.push_back(tempVm);
		}
	}

	std::string
	Virt5gc::GetTopoFile (void)
	{
		return m_topoFile;
	}

	void
	Virt5gc::Read (void)
	{
		std::ifstream topgen;
		topgen.open (GetTopoFile().c_str());
		NodeContainer nodes;
		std::vector<std::pair<int, int>> ueCoords;
		std::vector<std::pair<int, int>> enbCoords;

		if (!topgen.is_open())
		{
			NS_LOG_WARN("Cannot open an Virt5gc topology file");
			//Time time = Simulator::Now();
		}

		int totNode = 0;
		int totLink = 0;

		std::istringstream lineBuffer;
		std::string line;

		this->getline (topgen, line);
		lineBuffer.str (line);

		lineBuffer >> totNode;
		lineBuffer >> totLink;
		NS_LOG_INFO ("Virt5gc topology should have " << totNode << " node and " << totLink << "links");

		int x, y, component, nodeIdx, vm;
		uint16_t i;
		std::string tmp;

		lteHelper = CreateObject<LteHelper> ();
		epcHelper = CreateObject<OvsPointToPointEpcHelper> ();
		lteHelper->SetEpcHelper(epcHelper);

		/* Generate Lte components (PGW/SGW, eNB, UE)
		 * MME is not implemented yet
		 */
		for (i = 0; i < totNode && !topgen.eof(); i++)
		{
			this->getline (topgen, line);
			lineBuffer.clear();
			lineBuffer.str(line);

			/* If a component is
			 * 0: MME
			 * 1: SGW/PGW
			 * 2: eNB
			 * 3: UE
			 */

			lineBuffer >> nodeIdx;
			lineBuffer >> x;
			lineBuffer >> y;
			lineBuffer >> component;

			std::pair<int, int> coordPair(x, y);

			Virt5gcNode tmpNode(nodeIdx, x, y, component);

			// If component is MME or SGW/PGW, then read Vm, PM info
			if (component == 0 || component == 1) {
				lineBuffer >> vm;
				std::pair<int, int> tmpPair(vm, nodeIdx);
				tmpNode.SetVm(vm);
			}

			nodeList.push_back(tmpNode);

			// categorization
			if (component == 0) {
				NS_LOG_INFO ("Create MME Node");
				mmeN++;
				mmeVmN++;
			}
			else if (component == 1) {
				pgwN++;
				pgwVmN++;
			}
			else if (component == 2) {
				Ptr<Node> tmpNode = CreateObject<Node> ();
				enbNodes.Add(tmpNode);
				enbCoords.push_back(coordPair);
				enbN++;
			}
			else if (component == 3) {
				Ptr<Node> tmpNode = CreateObject<Node> ();
				ueNodes.Add(tmpNode);
				ueCoords.push_back(coordPair);
				ueN++;
			}
			else {
				NS_LOG_WARN ("Node component setting error");
			}
		}

		AllocNodesPosition(ueCoords, enbCoords);

		enbDevs = lteHelper->InstallEnbDevice (enbNodes);
		ueDevs = lteHelper->InstallUeDevice (ueNodes);

		/* Read linking information in the topology file
		 * and map the Ue nodes and Enb nodes
		 */
		int ueId, enbId, bw;
		int ueSub = pgwN + mmeN + enbN + 1;
		int enbSub = pgwN + mmeN + 1;
		for (i = 0; i < totLink && !topgen.eof(); i++)
		{
			this->getline (topgen, line);
			lineBuffer.clear();
			lineBuffer.str(line);

			lineBuffer >> ueId;
			lineBuffer >> enbId;
			lineBuffer >> bw;

			std::pair<int, int> tempPair(ueId - ueSub, enbId - enbSub);
			nodeMapping.push_back(tempPair);
		}

		ReadVm();
	}

	void
	Virt5gc::DynamicLoadInit (double std)
	{
		loadStd = std;

		std::string m_loadFile = "Virt5gc-load.data";
		AsciiTraceHelper ascii;
		loadStream = ascii.CreateFileStream (m_loadFile.c_str());
		*loadStream->GetStream() << "#Time Node Cpu Memory Disk" << std::endl;

		std::string m_scaleFile = "Virt5gc-scaling.data";
		scalingStream = ascii.CreateFileStream (m_scaleFile.c_str());
		*scalingStream->GetStream() << "#Time Node In/Out Delay" << std::endl;

		Simulator::Schedule(Seconds(1.2), &Virt5gc::DynamicLoad, this);
	}

	void
	Virt5gc::DynamicLoad (void)
	{
		unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
		std::default_random_engine generator(seed);
		Time time = Simulator::Now();

		int cpuLoad, memLoad, diskLoad;
		int comp;
		std::list<Virt5gcNode>::iterator itor;
		for (itor = nodeList.begin(); itor != nodeList.end(); itor++)
		{
			comp = (*itor).GetComponent();
			if (comp == 0 || comp == 1) {
				cpuLoad = ((*itor).GetCpuInfo()).second;
				memLoad = ((*itor).GetMemInfo()).second;
				diskLoad = ((*itor).GetDiskInfo()).second;

				std::normal_distribution<double> distribution(cpuLoad, loadStd);
				cpuLoad = distribution(generator);
				while (cpuLoad < 0)
					cpuLoad = distribution(generator);
				(*itor).ChangeCpuLoad(cpuLoad);

				std::normal_distribution<double> distribution2(memLoad, loadStd);
				memLoad = distribution2(generator);
				while (memLoad < 0)
					memLoad = distribution(generator);
				(*itor).ChangeMemLoad(memLoad);

				std::normal_distribution<double> distribution3(diskLoad, loadStd);
				diskLoad = distribution3(generator);
				while (diskLoad < 0)
					diskLoad = distribution(generator);
				(*itor).ChangeDiskLoad(diskLoad);

				*loadStream->GetStream() << time.GetSeconds() << " " << (*itor).GetId() << " " << cpuLoad << " " << memLoad << " " << diskLoad << std::endl;
			}
		}

		Scaling();
		Simulator::Schedule(Seconds(1.0), &Virt5gc::DynamicLoad, this);
	}

	void
	Virt5gc::SetMigrationRate (double scaleIn, double scaleOut)
	{
		scaleInRate = scaleIn;
		scaleOutRate = scaleOut;
	}

	void
	Virt5gc::SetAllocationDelay (double delay)
	{
		allocDelay = delay;
	}

	double
	Virt5gc::ScalingDelay(bool in, int migratedLoad, int bw, int comp)
	{
		double delay;
		if (!in){
			// calc scale out delay
			if (comp == 0) {
				delay = allocDelay + (migratedLoad / (double)bw);
			}
		}
		else {
			if (comp == 0) {
				// calc scale in delay
				delay = migratedLoad / (double)bw;
				//delay = (workload/vmCount)/(double)bw;
			}
		}
		//std::cout << migratedLoad << " " << bw << std::endl; 
		//std::cout << "Delay: " << delay << std::endl;
		return delay;
	}

	/* Find a VM list using VM IDs */
	std::list<Virt5gcVm*>
	Virt5gc::GetNodeVms(std::list<int> vms)
	{
		std::list<Virt5gcVm*> tempVms;
		std::list<Virt5gcVm>::iterator itor;
		std::list<int>::iterator itor2;
		for (itor = vmList.begin(); itor != vmList.end(); itor++) {
			for (itor2 = vms.begin(); itor2 != vms.end(); itor2++) {
				if (*itor2 == (*itor).GetVmId())
					tempVms.push_back(&(*itor));
			}
		}
		return tempVms;
	}

	double
	Virt5gc::scaleIn(std::list<Virt5gcVm*> *vms, std::pair<int, int> cpuInfo, std::pair<int, int> memInfo, std::pair<int, int> diskInfo) {
		
		double delay;
		int evenMemLoad, memCapacity, currMemLoad, memRemain, migratedLoad;
		int evenCpuLoad, cpuCapacity, currCpuLoad, cpuRemain;
		int evenDiskLoad, diskCapacity, currDiskLoad, diskRemain;
		std::list<Virt5gcVm*>::iterator itor;
		int vmsSize = vms->size();

		memCapacity = memInfo.first / (vmsSize + 1);
		cpuCapacity = cpuInfo.first / (vmsSize + 1);
		diskCapacity = diskInfo.first / (vmsSize + 1);
		migratedLoad = 0;

		// even distribution
		if (scaleInRate == 0.0) {
			evenMemLoad = memInfo.second / vmsSize;
			evenCpuLoad = cpuInfo.second / vmsSize;
			evenDiskLoad = diskInfo.second / vmsSize;

			for (itor = vms->begin(); itor != vms->end(); itor++) {
				currMemLoad = (**itor).GetMemInfo().second;
				migratedLoad += abs(evenMemLoad - currMemLoad);
				(**itor).ChangeMemLoad(evenMemLoad);
				(**itor).ChangeCpuLoad(evenCpuLoad);
				(**itor).ChangeDiskLoad(evenDiskLoad);
			}
		}
		else { // distribute a load from first VM in a VM list
			memRemain = memInfo.second;
			cpuRemain = cpuInfo.second;
			diskRemain = diskInfo.second;

			for (itor = vms->begin(); itor != vms->end(); itor++) {
				currMemLoad = (**itor).GetMemInfo().second;
				memRemain -= currMemLoad;
				currCpuLoad = (**itor).GetCpuInfo().second;
				cpuRemain -= currCpuLoad;
				currDiskLoad = (**itor).GetDiskInfo().second;
				diskRemain -= currDiskLoad;

				if (memRemain != 0) {
					if (memRemain < (memCapacity - currMemLoad)) {
						migratedLoad += memRemain;
						(**itor).ChangeMemLoad(currMemLoad + memRemain);
						memRemain = 0;
					}
					else {
						memRemain -= (memCapacity - currMemLoad);
						migratedLoad += (memCapacity - currMemLoad);
						(**itor).ChangeMemLoad(memCapacity);
					}
				}
				if (cpuRemain != 0) {
					if (cpuRemain < (cpuCapacity - currCpuLoad)) {
						(**itor).ChangeCpuLoad(currCpuLoad + cpuRemain);
						cpuRemain = 0;
					}
					else {
						cpuRemain -= (cpuCapacity - currCpuLoad);
						(**itor).ChangeCpuLoad(cpuCapacity);
					}
				}
				if (diskRemain != 0) {
					if (diskRemain < (diskCapacity - currDiskLoad)) {
						(**itor).ChangeDiskLoad(currDiskLoad + diskRemain);
						diskRemain = 0;
					}
					else {
						diskRemain -= (diskCapacity - currDiskLoad);
						(**itor).ChangeDiskLoad(diskCapacity);
					}
				}
			}
		}

		if (itor == vms->end())
			--itor;
		delay = ScalingDelay(true, migratedLoad, (**itor).GetBwInfo().second, 0);

		return delay;
	}
	
	
	double
	Virt5gc::scaleOut(std::list<Virt5gcVm*> *vms, std::pair<int, int> cpuInfo, std::pair<int, int> memInfo, std::pair<int, int> diskInfo) {

		double delay;
		int evenMemLoad, memCapacity, migratedLoad;
		int evenCpuLoad, cpuCapacity; 
		int evenDiskLoad, diskCapacity;
		std::list<Virt5gcVm*>::iterator itor;
		int vmsSize = vms->size();
	
		int newMemLoad, newCpuLoad, newDiskLoad;
		int cpuFlag, memFlag, diskFlag;

		memCapacity = memInfo.first / (vmsSize - 1);
		cpuCapacity = cpuInfo.first / (vmsSize - 1);
		diskCapacity = diskInfo.first / (vmsSize - 1);

		memFlag = memInfo.second - memInfo.first;
		cpuFlag = cpuInfo.second - cpuInfo.first;
		diskFlag = diskInfo.second - diskInfo.first;

		newMemLoad = memInfo.second / vmsSize;
		newCpuLoad = cpuInfo.second / vmsSize;
		newDiskLoad = diskInfo.second / vmsSize;

		if (scaleOutRate > 0.0) {
			if (memFlag > 0)
				newMemLoad = round(scaleOutRate * memCapacity);
			if (cpuFlag > 0)
				newCpuLoad = round(scaleOutRate * cpuCapacity);
			if (diskFlag > 0)
				newDiskLoad = round(scaleOutRate * diskCapacity);
		}
		else if (scaleOutRate == 0.0) {
			if (newMemLoad > 0) 
				newMemLoad = memFlag;
			
			if (newCpuLoad > 0) 
				newCpuLoad = cpuFlag;
			
			if (newDiskLoad > 0) 
				newDiskLoad = diskFlag;
		}
		else {
			std::cout << "Error!! scale-out rate is negative\n";
		}
		migratedLoad = newMemLoad;

		itor = vms->end();
		itor--;
		(**itor).ChangeMemLoad(newMemLoad);
		(**itor).ChangeCpuLoad(newCpuLoad);
		(**itor).ChangeDiskLoad(newDiskLoad);

		std::list<Virt5gcVm*>::iterator itor2;
		evenMemLoad = (memInfo.second - newMemLoad) / (vmsSize - 1);
		evenCpuLoad = (cpuInfo.second - newCpuLoad) / (vmsSize - 1);
		evenDiskLoad = (diskInfo.second - newDiskLoad) / (vmsSize - 1);
		for (itor2 = vms->begin(); itor2 != itor; itor2++) {
			(**itor2).ChangeMemLoad(evenMemLoad);
			(**itor2).ChangeCpuLoad(evenCpuLoad);
			(**itor2).ChangeDiskLoad(evenDiskLoad);
		}

		delay = ScalingDelay(false, migratedLoad, (**itor).GetBwInfo().second, 0);

		return delay;
	}
	 

	static GlobalValue g_delay = GlobalValue ("scalingDelay", "scaling delay", DoubleValue (-1.0), MakeDoubleChecker<double> ());
	static GlobalValue g_time = GlobalValue ("scalingTime", "scaling time", TimeValue(Time(0)), MakeTimeChecker());

	void
	Virt5gc::Scaling (void)
	{
		std::list<Virt5gcVm*>::iterator itor2;
		std::list<Virt5gcVm>::iterator it;
		it = vmList.end();
		--it;
		int lastVmId = (*it).GetVmId();

		bool outFlag = false;
		int inFlag, comp;
		std::list<Virt5gcNode>::iterator itor;
		double mme_delay = 0, pgw_delay = 0;

		Time time = Simulator::Now();
		for (itor = nodeList.begin(); itor != nodeList.end(); itor++) {
			comp = (*itor).GetComponent();
			//mme_delay = 0.0;
			//pgw_delay = 0.0;
			inFlag = 0;
			outFlag = false;

			// mme
			if (comp == 0) {

				std::pair<int, int> cpuInfo = (*itor).GetCpuInfo();
				std::pair<int, int> diskInfo = (*itor).GetDiskInfo();
				std::pair<int, int> memInfo = (*itor).GetMemInfo();

				//std::cout << "mme " << cpuInfo.first << " " << cpuInfo.second << " " << diskInfo.first << " " << diskInfo.second << " " << memInfo.first << " " << memInfo.second << "\n";

				// check scale out of memory or cpu or disk 
				if ((memInfo.first < memInfo.second) || (cpuInfo.first < cpuInfo.second) || (diskInfo.first < diskInfo.second)) {
					outFlag = true;
				}
				else {
					if ((memInfo.first-memInfo.second) >= (memInfo.first/mmeVmN))
						inFlag++;
					if ((cpuInfo.first-cpuInfo.second) >= (cpuInfo.first/mmeVmN)) 
						inFlag++;
					if ((diskInfo.first-diskInfo.second) >= (diskInfo.first/mmeVmN))
						inFlag++;
				}
				
				// do scale out
				if (outFlag) {
					std::list<int> vms = (*itor).GetVms();
					std::list<Virt5gcVm*> tempVms = GetNodeVms(vms);
					itor2 = tempVms.begin();
					Virt5gcVm newVm = **itor2;
					newVm.SetId(++lastVmId);
					tempVms.push_back(&newVm);
					
					mme_delay = scaleOut(&tempVms, cpuInfo, memInfo, diskInfo);

					vmList.push_back(newVm);
					
					(*itor).SetVm(lastVmId);
					(*itor).SetMemInfo(memInfo.first + (memInfo.first/mmeVmN), memInfo.second);
					(*itor).SetCpuInfo(cpuInfo.first + (cpuInfo.first/mmeVmN), cpuInfo.second);
					(*itor).SetDiskInfo(diskInfo.first + (diskInfo.first/mmeVmN), diskInfo.second);
					mmeVmN++;

					*scalingStream->GetStream() << time.GetSeconds() << " " << (*itor).GetId() << " Scale Out " << mme_delay << std::endl;

				}
				
				// do scale in
				if (inFlag == 3) {
					std::list<int> vms = (*itor).GetVms();
					std::list<Virt5gcVm*> tempVms = GetNodeVms(vms);
					itor2 = tempVms.end();
					--itor2;

					(*itor).DeleteVm((**itor2).GetVmId());
					vmList.remove(**itor2);
					tempVms.remove(*itor2);

					mme_delay = scaleIn(&tempVms, cpuInfo, memInfo, diskInfo);
					//g_delay.SetValue(DoubleValue(mme_delay));
					//g_time.SetValue(TimeValue(time));

					(*itor).SetMemInfo(memInfo.first - (memInfo.first/mmeVmN), memInfo.second);
					(*itor).SetCpuInfo(cpuInfo.first - (cpuInfo.first/mmeVmN), cpuInfo.second);
					(*itor).SetDiskInfo(diskInfo.first - (diskInfo.first/mmeVmN), diskInfo.second);

					mmeVmN--;
					*scalingStream->GetStream() << time.GetSeconds() << " " << (*itor).GetId() << " Scale In " << mme_delay << std::endl;

				}

/*
				std::list<Virt5gcVm>::iterator it;
				for (it = vmList.begin(); it != vmList.end(); it++)
				{
					std::cout << "MME! " << "ID: " << (*it).GetVmId() << ", ToR: " << (*it).GetToRId() << ", Pm: " << (*it).GetPmId() << ", NodeId: " << (*it).GetNodeId() << std::endl;
					std::cout << "Cpu Cap: " << (*it).GetCpuInfo().first << ", load: " << (*it).GetCpuInfo().second << ", Mem Cap: " << (*it).GetMemInfo().first << ", load: " << (*it).GetMemInfo().second << ", Disk Cap: " << (*it).GetDiskInfo().first << ", load: " << (*it).GetDiskInfo().second << ", Bw Cap: " << (*it).GetBwInfo().first  << "\n" << std::endl;
				}
*/

			}

			// pgw/sgw
			else if (comp == 1) {

				std::pair<int, int> cpuInfo = (*itor).GetCpuInfo();
				std::pair<int, int> diskInfo = (*itor).GetDiskInfo();
				std::pair<int, int> memInfo = (*itor).GetMemInfo();

				//std::cout << "pgw " << cpuInfo.first << " " << cpuInfo.second << " " << diskInfo.first << " " << diskInfo.second << " " << memInfo.first << " " << memInfo.second << "\n";

				
				if ((memInfo.first < memInfo.second) || (cpuInfo.first < cpuInfo.second) || (diskInfo.first < diskInfo.second)) {
					outFlag = true;
				}
				else {
					if ((memInfo.first-memInfo.second) >= (memInfo.first/mmeVmN))
						inFlag++;
					if ((cpuInfo.first-cpuInfo.second) >= (cpuInfo.first/mmeVmN)) 
						inFlag++;
					if ((diskInfo.first-diskInfo.second) >= (diskInfo.first/mmeVmN))
						inFlag++;
				}
	
				if (outFlag) {
					std::list<int> vms = (*itor).GetVms();
					std::list<Virt5gcVm*> tempVms = GetNodeVms(vms);
					itor2 = tempVms.begin();

					Virt5gcVm newVm = **itor2;
					newVm.SetId(++lastVmId);
					tempVms.push_back(&newVm);
					
					pgw_delay = scaleOut(&tempVms, cpuInfo, memInfo, diskInfo);

					vmList.push_back(newVm);
					(*itor).SetVm(lastVmId);
					(*itor).SetMemInfo(memInfo.first + (memInfo.first/pgwVmN), memInfo.second);
					(*itor).SetCpuInfo(cpuInfo.first + (cpuInfo.first/pgwVmN), cpuInfo.second);
					(*itor).SetDiskInfo(diskInfo.first + (diskInfo.first/pgwVmN), diskInfo.second);
					pgwVmN++;

					*scalingStream->GetStream() << time.GetSeconds() << " " << (*itor).GetId() << " Scale Out " << pgw_delay << std::endl;
				}

				// do scale in
				if (inFlag == 3) {
					std::list<int> vms = (*itor).GetVms();
					std::list<Virt5gcVm*> tempVms = GetNodeVms(vms);
					itor2 = tempVms.end();
					--itor2;

					(*itor).DeleteVm((**itor2).GetVmId());
					vmList.remove(**itor2);
					tempVms.remove(*itor2);

					pgw_delay = scaleIn(&tempVms, cpuInfo, memInfo, diskInfo);
					//g_delay.SetValue(DoubleValue(pgw_delay));
					//g_time.SetValue(TimeValue(time));

					(*itor).SetMemInfo(memInfo.first - (memInfo.first/pgwVmN), memInfo.second);
					(*itor).SetCpuInfo(cpuInfo.first - (cpuInfo.first/pgwVmN), cpuInfo.second);
					(*itor).SetDiskInfo(diskInfo.first - (diskInfo.first/pgwVmN), diskInfo.second);

					pgwVmN--;

					*scalingStream->GetStream() << time.GetSeconds() << " " << (*itor).GetId() << " Scale In " << pgw_delay << std::endl;	
				}

				
							/*
				std::list<Virt5gcVm>::iterator it;
				for (it = vmList.begin(); it != vmList.end(); it++)
				{
					std::cout << "ID: " << (*it).GetVmId() << ", ToR: " << (*it).GetToRId() << ", Pm: " << (*it).GetPmId() << ", NodeId: " << (*it).GetNodeId() << std::endl;
					std::cout << "Cpu Cap: " << (*it).GetCpuInfo().first << ", load: " << (*it).GetCpuInfo().second << ", Mem Cap: " << (*it).GetMemInfo().first << ", load: " << (*it).GetMemInfo().second << ", Disk Cap: " << (*it).GetDiskInfo().first << ", load: " << (*it).GetDiskInfo().second << ", Bw Cap: " << (*it).GetBwInfo().first << "\n" << std::endl;
				}
*/				
			}
		}

		
		if (mme_delay > pgw_delay) {
			g_delay.SetValue(DoubleValue(mme_delay));
			g_time.SetValue(TimeValue(time));
		}
		else {
			g_delay.SetValue(DoubleValue(pgw_delay));
			g_time.SetValue(TimeValue(time));
		}
	}


	void
	Virt5gc::AllocNodesPosition (std::vector<std::pair<int, int>> ueCoords, std::vector<std::pair<int, int>> enbCoords)
	{
		uint16_t i;

		/* Allocate initial position of nodes */
		Ptr<ListPositionAllocator> uePosition = CreateObject<ListPositionAllocator> ();
		for (i = 0; i < ueNodes.GetN(); i++) 
		{
			uePosition->Add (Vector(ueCoords[i].first, ueCoords[i].second, 0));
		}
		MobilityHelper ueMobility;
		ueMobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");
		ueMobility.SetPositionAllocator(uePosition);
		ueMobility.Install(ueNodes);
		
		Ptr<ListPositionAllocator> enbPosition = CreateObject<ListPositionAllocator> ();
		for (i = 0; i < enbNodes.GetN(); i++) 
		{
			enbPosition->Add (Vector(enbCoords[i].first, enbCoords[i].second, 0));
		}
		MobilityHelper enbMobility;
		enbMobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");
		enbMobility.SetPositionAllocator(enbPosition);
		enbMobility.Install(enbNodes);
	}


	// <Ue, eNB> pair
	std::list<std::pair<int, int>>
	Virt5gc::GetNodeMap (void)
	{
		return nodeMapping;
	}

	NodeContainer
	Virt5gc::GetEnbNodes (void)
	{
		return enbNodes;
	}
	
	NodeContainer
	Virt5gc::GetUeNodes (void)
	{
		return ueNodes;
	}

	Ptr<LteHelper>
	Virt5gc::GetLteHelper (void)
	{
		return lteHelper;
	}

	Ptr<OvsPointToPointEpcHelper>
	Virt5gc::GetEpcHelper (void)
	{
		return epcHelper;
	}

	NetDeviceContainer
	Virt5gc::GetEnbDevs (void)
	{
		return enbDevs;
	}

	NetDeviceContainer
	Virt5gc::GetUeDevs (void)
	{
		return ueDevs;
	}

	std::list<Virt5gcVm>
	Virt5gc::GetVmList (void)
	{
		return vmList; 
	}

	std::istream&
	Virt5gc::getline (std::istream& is, std::string &str)
	{
		std::string::size_type index = 0;
		do {
			std::getline(is, str);
			index = str.find_first_not_of(" \t\r\n");
		} while(index < 0 || str[index] == Virt5gc::COMMENT_HEADER);
		return is;
	}


}
