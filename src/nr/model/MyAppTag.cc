#include "ns3/tag.h"
#include "ns3/uinteger.h"
#include "MyAppTag.h"

namespace ns3 {




MyAppTag::MyAppTag ()
  {

  }

 MyAppTag::MyAppTag (Time sendTs) : m_sendTs (sendTs)
  {
	// std::cout << m_sendTs << std::endl;
  }
MyAppTag::~MyAppTag(){}
  TypeId
  MyAppTag::GetTypeId (void)
  {
    static TypeId tid = TypeId ("ns3::MyAppTag")
      .SetParent<Tag> ()
      .AddConstructor<MyAppTag> ();
    return tid;
  }

 TypeId
 MyAppTag::GetInstanceTypeId (void) const
  {
    return GetTypeId ();
  }

void
MyAppTag::Serialize (TagBuffer i) const
  {

	i.WriteU64 (m_sendTs.GetNanoSeconds());
  }

   void
   MyAppTag::Deserialize (TagBuffer i)
  {

	   m_sendTs = NanoSeconds (i.ReadU64 ());
  }

  uint32_t
  MyAppTag::GetSerializedSize () const
  {
    return sizeof (m_sendTs);
  }

   void
   MyAppTag::Print (std::ostream &os) const
  {
    std::cout << m_sendTs;
  }
void
MyAppTag::SetTime(Time t){
	m_sendTs = t;

}
Time
MyAppTag::GetTime(){
		return m_sendTs;
}


}
