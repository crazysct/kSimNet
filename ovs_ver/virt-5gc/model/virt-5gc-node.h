#ifndef VIRT_5GC_NODE_H
#define VIRT_5GC_NODE_H

#include "ns3/object.h"

namespace ns3 {

	class Virt5gcNode : public Object
	{
		public:
			static TypeId GetTypeId (void);
			Virt5gcNode (int id, int x, int y, int component);
			//bool operator<(const Virt5gcNode& rhs) const; 

			int GetId (void);
			std::pair<int, int> GetCoord (void);
			int GetComponent (void);
			std::list<int> GetVms(void);
			std::pair<int, int> GetCpuInfo(void);
			std::pair<int, int> GetMemInfo(void);
			std::pair<int, int> GetDiskInfo(void);
			std::pair<int, int> GetBwInfo(void);

			bool FindVm(int vm);
			void DeleteVm(int vm);

			void SetVm (int vm);
			void SetCpuInfo(int size, int util);
			void SetMemInfo(int size, int util);
			void SetDiskInfo(int size, int util);
			void SetBwInfo(int size, int util);
			void ChangeCpuLoad(int util);
			void ChangeMemLoad(int util);
			void ChangeDiskLoad(int util);

		private:
			int node_id;
			int node_x;
			int node_y;
			int node_component;
			std::list<int> node_vms;

			int cpuSize;
			int cpuUtil;
			int memSize;
			int memUtil;
			int diskSize;
			int diskUtil;
			int bwSize;
			int bwUtil;
	};
};

#endif
