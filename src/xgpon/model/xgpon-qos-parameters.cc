/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2012 University College Cork (UCC), Ireland
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * Author: Xiuchao Wu <xw2@cs.ucc.ie>
 */

#include "ns3/log.h"

#include "ns3/enum.h"
#include "ns3/uinteger.h"

#include "xgpon-qos-parameters.h"



NS_LOG_COMPONENT_DEFINE ("XgponQosParameters");

namespace ns3 {
NS_OBJECT_ENSURE_REGISTERED (XgponQosParameters);


TypeId
XgponQosParameters::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::XgponQosParameters")
    .SetParent<Object> ()
    .AddConstructor<XgponQosParameters> ()
    .AddAttribute ("TcontType", 
                   "The T-CONT type that this connection belongs to.",
                   EnumValue (XGPON_TCONT_TYPE_4),
                   MakeEnumAccessor (&XgponQosParameters::SetTcontType),
                   MakeEnumChecker (XGPON_TCONT_TYPE_1, "T-CONT Type 1 with fixed bandwidth allocation",
                                    XGPON_TCONT_TYPE_2, "T-CONT Type 2 with assured bandwidth allocation",
                                    XGPON_TCONT_TYPE_3, "T-CONT Type 3 with assured and nonassured bandwidth allocation",
                                    XGPON_TCONT_TYPE_4, "T-CONT Type 4 with best effort service",
                                    XGPON_TCONT_TYPE_5, "T-CONT Type 5 with general bandwidth requirement."))
    .AddAttribute ("FixedBandwidth", 
                   "The fixed bandwidth that is pre-allocated to this connection.",
                   UintegerValue (20000),
                   MakeUintegerAccessor (&XgponQosParameters::m_fixedBw),
                   MakeUintegerChecker<uint32_t> ())
    .AddAttribute ("AssuredBandwidth", 
                   "The assured bandwidth that is dynamically allocated to this connection.",
                   UintegerValue (20000),
                   MakeUintegerAccessor (&XgponQosParameters::m_assuredBw),
                   MakeUintegerChecker<uint32_t> ())
    .AddAttribute ("NonAssuredBandwidth", 
                   "The non-assured bandwidth that is dynamically allocated to this connection.",
                   UintegerValue (20000),
                   MakeUintegerAccessor (&XgponQosParameters::m_nonAssuredBw),
                   MakeUintegerChecker<uint32_t> ())
    .AddAttribute ("MaxBandwidth", 
                   "The maximal bandwidth that could be allocated to this connection (best effort service).",
                   UintegerValue (20000),
                   MakeUintegerAccessor (&XgponQosParameters::m_maxBw),
                   MakeUintegerChecker<uint32_t> ())
    .AddAttribute ("MaxServiceInterval", 
                   "The maximal interval between the consecutive intervals os a connection. unit: nanosecond",
                   UintegerValue (10000000),    //10ms
                   MakeUintegerAccessor (&XgponQosParameters::m_maxInterval),
                   MakeUintegerChecker<uint32_t> ())
    /*
     * Added minInterval to help with GIANT scheduler.
     */
    .AddAttribute ("MinServiceInterval",
                   "The minimum interval between the consecutive intervals of a connection. unit: nanosecond",
                   UintegerValue (10000000),    //10ms
                   MakeUintegerAccessor (&XgponQosParameters::m_minInterval),
                   MakeUintegerChecker<uint32_t> ())
  ;

  return tid;
}
TypeId
XgponQosParameters::GetInstanceTypeId (void) const
{
  return GetTypeId ();
}



XgponQosParameters::XgponQosParameters ()
{
}
XgponQosParameters::~XgponQosParameters ()
{
}






void 
XgponQosParameters::AggregateQosParameters(const Ptr<XgponQosParameters>& qosParameters)
{
  NS_LOG_FUNCTION (this);

  if(m_tcontType != qosParameters->GetTcontType()) 
  {
    NS_LOG_ERROR ("ERROR!!! Connections of the same TCONT should have the same T-CONT type.");
  }

  m_fixedBw += qosParameters->GetFixedBw ();
  m_assuredBw += qosParameters->GetAssuredBw ();
  m_nonAssuredBw += qosParameters->GetNonAssuredBw ();
  m_maxBw += qosParameters->GetMaxBw ();

  if(m_maxInterval <= 0) m_maxInterval = qosParameters->GetMaxInterval ();
  else
  {
    if(m_maxInterval > qosParameters->GetMaxInterval ()) m_maxInterval = qosParameters->GetMaxInterval ();
  }
  /*
   * Added minInterval To Help with GIANT scheduler
   */
  if(m_minInterval <= 0) m_minInterval = qosParameters->GetMinInterval ();
  else
  {
    if(m_minInterval < qosParameters->GetMinInterval ()) m_minInterval = qosParameters->GetMinInterval ();
  }

}


void 
XgponQosParameters::DeepCopy(const Ptr<XgponQosParameters>& qosParameters)
{
  NS_LOG_FUNCTION (this);

  m_tcontType = qosParameters->GetTcontType();
  m_fixedBw = qosParameters->GetFixedBw ();
  m_assuredBw = qosParameters->GetAssuredBw ();
  m_nonAssuredBw = qosParameters->GetNonAssuredBw ();
  m_maxBw = qosParameters->GetMaxBw ();
  m_maxInterval = qosParameters->GetMaxInterval ();
  /*
   * Added minInterval To Help with GIANT scheduler
   */
  m_minInterval = qosParameters->GetMinInterval ();
}



}; // namespace ns3
