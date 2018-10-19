
#ifndef OVS_POINT_TO_POINT_EPC_HELPER_H
#define OVS_POINT_TO_POINT_EPC_HELPER_H

#include <ns3/point-to-point-epc-helper.h>
#include <ns3/object.h>
#include <ns3/data-rate.h>
#include <ns3/epc-helper.h>
#include <ns3/epc-tft.h>
#include <ns3/eps-bearer.h>

using namespace ns3;

class OVSPointToPointEpcHelper : public PointToPointEpcHelper
{


public:
 OVSPointToPointEpcHelper ();
 virtual ~OVSPointToPointEpcHelper ();
 void AddEnb(Ptr<Node> enbNode, Ptr<NetDevice> lteEnbNetDevice, uint16_t cellId);

};



#endif
