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

#include "ns3/simulator.h"
#include "ns3/nstime.h"
#include "ns3/ptr.h"
#include "ns3/log.h"

#include "xgpon-tcont-olt.h"
#include "xgpon-net-device.h"



NS_LOG_COMPONENT_DEFINE ("XgponTcontOlt");

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (XgponTcontOlt);

TypeId 
XgponTcontOlt::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::XgponTcontOlt")
    .SetParent<XgponTcont> ()
    .AddConstructor<XgponTcontOlt> ()
  ;
  return tid;
}
TypeId 
XgponTcontOlt::GetInstanceTypeId (void) const
{
  return GetTypeId ();
}




XgponTcontOlt::XgponTcontOlt (): XgponTcont (),
  m_connections(0),
  m_pkt4Reassemble(0),
  m_qosParameters(0),
  m_lastPollingTime(0)
{
}
XgponTcontOlt::~XgponTcontOlt ()
{
}




void 
XgponTcontOlt::ReceiveStatusReport (const Ptr<XgponXgtcDbru>& report, uint64_t time)
{
  NS_LOG_FUNCTION(this);

  report->SetReceiveTime(time);
  AddNewBufOccupancyReport (report);

  int64_t timeTh = time - XgponNetDevice::HISTORY_2_MAINTAIN;
  if(timeTh > 0) ClearOldBufOccupancyReports (time - XgponNetDevice::HISTORY_2_MAINTAIN);
}

void 
XgponTcontOlt::AddNewBwAllocation2ServiceHistory (const Ptr<XgponXgtcBwAllocation>& allocation, uint64_t time)
{
  NS_LOG_FUNCTION(this);

  allocation->SetCreateTime(time);
  AddNewBwAllocation (allocation);

  int64_t timeTh = time - XgponNetDevice::HISTORY_2_MAINTAIN;
  if(timeTh > 0) ClearOldBandwidthAllocations(timeTh);

  if(allocation->GetDbruFlag() != 0) { m_lastPollingTime = time; }
}








void 
XgponTcontOlt::AddOneConnection (const Ptr<XgponConnectionReceiver>& conn)
{
  NS_LOG_FUNCTION(this);

  if(m_qosParameters==0) 
  {
    m_qosParameters = CreateObject<XgponQosParameters>();
    m_qosParameters->DeepCopy(conn->GetQosParameters ());
  }
  else
  {
    m_qosParameters->AggregateQosParameters(conn->GetQosParameters ());
  }

  m_connections.push_back(conn);
}












uint32_t 
XgponTcontOlt::CalculateRemainingDataToServe (uint64_t rtt, uint64_t slotSize)
{
  NS_LOG_FUNCTION(this);

  const Ptr<XgponXgtcDbru>& dbru = GetLatestBufOccupancyReport ();
  if(dbru==0) return 0;

  uint32_t latestOccupancy = dbru->GetBufOcc ();
  uint64_t lastReportTime = dbru->GetReceiveTime ();

  if(m_bwAllocations.size()==0) return latestOccupancy;


  //since m_bwAllocations is maintained according to creation time (ascend order), we should start from the tail to consider the newer bwallocs.
  std::deque < Ptr<XgponXgtcBwAllocation> >::reverse_iterator rit, rend;
  rend = m_bwAllocations.rend();
  rit = m_bwAllocations.rbegin();
  uint32_t assignedSize = 0;
  while(rit!=rend)
  {
    //the status report in one burst includes the data transmitted in that burst. Thus, slotSize / 2 is added to include the corresponding bwalloc.
    uint64_t timeTh = (*rit)->GetCreateTime() + rtt + slotSize / 2;
    if(timeTh > lastReportTime)   
    {
      assignedSize += (*rit)->GetGrantSize();
      if((*rit)->GetDbruFlag()) assignedSize -= 1;   //ocuupancy report occupies one word;    
      
      rit++;  
    } else break;
  } 
  
  int remain = latestOccupancy - assignedSize;
  if(remain < 0) remain = 0;

  return remain;
}










void 
XgponTcontOlt::ClearOldBufOccupancyReports (uint64_t time)
{
  NS_LOG_FUNCTION(this);

  while(m_bufOccupancyReports.size() > 0)
  {
    const Ptr<XgponXgtcDbru>& dbru = m_bufOccupancyReports.front();
    if(dbru->GetReceiveTime() < time) m_bufOccupancyReports.pop_front();
    else break;
  }

  return;
}


void 
XgponTcontOlt::ClearOldBandwidthAllocations (uint64_t time)
{
  NS_LOG_FUNCTION(this);

  while(m_bwAllocations.size() > 0)
  {
    const Ptr<XgponXgtcBwAllocation>& bwAlloc = m_bwAllocations.front();
    if(bwAlloc->GetCreateTime () < time) m_bwAllocations.pop_front();
    else break;
  }

  return;
}




}; // namespace ns3

