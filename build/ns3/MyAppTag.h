/*
 * MyAppTag.h
 *
 *  Created on: 2017. 12. 30.
 *      Author: netlab
 */

#ifndef NS3_DC_SRC_NR_MODEL_MYAPPTAG_H_
#define NS3_DC_SRC_NR_MODEL_MYAPPTAG_H_

#include "ns3/tag.h"
#include "ns3/uinteger.h"
#include "ns3/packet.h"
#include "ns3/nstime.h"

namespace ns3 {
class Tag;

class MyAppTag : public Tag
{
public:
  MyAppTag ();
  ~MyAppTag();

  MyAppTag (Time sendTs) ;

  static TypeId GetTypeId (void);


  virtual TypeId  GetInstanceTypeId (void) const;


  virtual void  Serialize (TagBuffer i) const;


  virtual void  Deserialize (TagBuffer i);

  virtual uint32_t  GetSerializedSize () const;
  virtual void Print (std::ostream &os) const;
   void SetTime(Time t);
   Time GetTime();
  Time m_sendTs;
};

}
#endif /* NS3_DC_SRC_NR_MODEL_MYAPPTAG_H_ */
