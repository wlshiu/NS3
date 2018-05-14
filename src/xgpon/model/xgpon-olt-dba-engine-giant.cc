/*
 * Copyright (c)  2013 The Provost, Fellows and Scholars of the
 * College of the Holy and Undivided Trinity of Queen Elizabeth near Dublin.
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
 * Authors: Pedro Alvarez <pinheirp@tcd.ie>
 *          Heather Crowley <crowleyh@tcd.ie>
 *
 */

#include "ns3/log.h"
#include "ns3/simulator.h"

#include "xgpon-olt-dba-engine-giant.h"
#include "xgpon-olt-dba-parameters-giant.h"
#include "xgpon-olt-net-device.h"
#include "xgpon-phy.h"
#include "xgpon-olt-ploam-engine.h"
#include "xgpon-link-info.h"
#include "xgpon-burst-profile.h"
#include "ns3/enum.h"
#include "ns3/uinteger.h"
#include "ns3/double.h"
#include <vector>


NS_LOG_COMPONENT_DEFINE ("XgponOltDbaEngineGiant");

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (XgponOltDbaEngineGiant);

TypeId
XgponOltDbaEngineGiant::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::XgponOltDbaEngineGiant")
    .SetParent<XgponOltDbaEngine> ()
    .AddConstructor<XgponOltDbaEngineGiant> ();
  return tid;
}

TypeId
XgponOltDbaEngineGiant::GetInstanceTypeId (void) const
{
  return GetTypeId ();
}



XgponOltDbaEngineGiant::XgponOltDbaEngineGiant () : XgponOltDbaEngine(),
m_lastScheduledAllocIndex(0),
m_nullTcont(0),
m_getNextNonAssuredTcontAtBeginning(false),
m_getNextBestEffortTcontAtBeginning(false),
m_stop(false)
{
  m_usAllTcons.clear();
}



XgponOltDbaEngineGiant::~XgponOltDbaEngineGiant ()
{
}

void
XgponOltDbaEngineGiant::DoInitialize()
{
}

//this function is called in AddAllocToDbaEngine
void XgponOltDbaEngineGiant::GenerateAllocOltGiantParameterPairs()
{
  std::vector< Ptr<XgponTcontOlt> >::iterator it;
  for (it = m_usAllTcons.begin(); it!=m_usAllTcons.end(); it++) {
    Ptr<XgponQosParameters> qos = (*it)->GetAggregatedQosParameters();

    Ptr<XgponOltDbaParametersGiant> giantParameter;

    //variables to hold qos parameters
    uint32_t serviceRateFixed, serviceRateAssured, serviceRateNonAssured, serviceRateBestEffort, serviceRateMax;
    uint64_t serviceIntervalMax, serviceIntervalMin;


    serviceRateFixed = qos->GetFixedBw();
    if(serviceRateFixed>0)
    {
      serviceIntervalMax = qos->GetMaxInterval();
      giantParameter = CreateObjectWithAttributes<XgponOltDbaParametersGiant>( "ServiceInterval",UintegerValue(serviceIntervalMax),
        "ServiceIntervalTimer",UintegerValue(serviceIntervalMax),  "AllocationBytes",UintegerValue(GetAllocationBytesFromRateAndServiceInterval(serviceRateFixed, serviceIntervalMax)),
        "BandwidthType",UintegerValue(XGPON_GIANT_BW_FIXED));
      m_allocOltFixedBandwidthPair.push_back (std::make_pair((*it), giantParameter)); //replaced allocOlt with (*it)
    }

    //assured
    serviceRateAssured = qos->GetAssuredBw();
    if(serviceRateAssured>0)
    {
      serviceIntervalMax = qos->GetMaxInterval();
      giantParameter = CreateObjectWithAttributes<XgponOltDbaParametersGiant>( "ServiceInterval",UintegerValue(serviceIntervalMax),
        "ServiceIntervalTimer",UintegerValue(serviceIntervalMax),  "AllocationBytes",UintegerValue(GetAllocationBytesFromRateAndServiceInterval(serviceRateAssured,serviceIntervalMax)),
        "BandwidthType",UintegerValue(XGPON_GIANT_BW_ASSURED));
      m_allocOltAssuredBandwidthPair.push_back (std::make_pair((*it), giantParameter));
    }

    //non-assured
    serviceRateNonAssured = qos->GetNonAssuredBw();
    if(serviceRateNonAssured>0)
    {
      serviceIntervalMin = qos->GetMinInterval();
      giantParameter = CreateObjectWithAttributes<XgponOltDbaParametersGiant>( "ServiceInterval",UintegerValue(serviceIntervalMin),
        "ServiceIntervalTimer",UintegerValue(serviceIntervalMin),  "AllocationBytes",UintegerValue(GetAllocationBytesFromRateAndServiceInterval(serviceRateNonAssured,serviceIntervalMin)),
        "BandwidthType",UintegerValue(XGPON_GIANT_BW_NON_ASSURED));
      m_allocOltNonAssuredBandwidthPair.push_back (std::make_pair((*it), giantParameter));
    }

    //best effort
    serviceRateMax = qos->GetMaxBw();
    serviceRateBestEffort = (serviceRateMax - serviceRateFixed - serviceRateAssured - serviceRateNonAssured);
    if(serviceRateBestEffort>0)
    {
      serviceIntervalMin = qos->GetMinInterval();
      giantParameter = CreateObjectWithAttributes<XgponOltDbaParametersGiant>( "ServiceInterval",UintegerValue(serviceIntervalMin),
        "ServiceIntervalTimer",UintegerValue(serviceIntervalMin),  "AllocationBytes",UintegerValue(GetAllocationBytesFromRateAndServiceInterval(serviceRateBestEffort, serviceIntervalMin)),
        "BandwidthType",UintegerValue(XGPON_GIANT_BW_BEST_EFFORT));
      m_allocOltBestEffortBandwidthPair.push_back (std::make_pair((*it), giantParameter));
    }

  }

  //Initialize Round Robin flags
  m_nonAssuredLastServedTcont=m_nonAssuredFirstServedTcont=m_allocOltNonAssuredBandwidthPair.begin();
  m_bestEffortLastServedTcont=m_bestEffortFirstServedTcont=m_allocOltBestEffortBandwidthPair.begin();

}






void
XgponOltDbaEngineGiant::AddTcontToDbaEngine (const ns3::Ptr<ns3::XgponTcontOlt>& alloc)
{
  NS_LOG_FUNCTION(this);
  m_usAllTcons.push_back(alloc);
  return;
}

const Ptr<XgponTcontOlt>&
XgponOltDbaEngineGiant::GetFirstTcontOlt ( )
{
  NS_LOG_FUNCTION(this);

  m_stop=false;

  if(!m_allocOltFixedBandwidthPair.empty())
    m_vectorPairIterator = m_allocOltFixedBandwidthPair.begin();
  else if(!m_allocOltAssuredBandwidthPair.empty())
    m_vectorPairIterator = m_allocOltAssuredBandwidthPair.begin();
  else if(!m_allocOltNonAssuredBandwidthPair.empty())
  {
    m_vectorPairIterator = m_nonAssuredLastServedTcont;
    if(m_getNextNonAssuredTcontAtBeginning)
    {
      m_vectorPairIterator++;
      m_getNextNonAssuredTcontAtBeginning=false;
      if(m_vectorPairIterator==m_allocOltNonAssuredBandwidthPair.end())
        m_vectorPairIterator=m_allocOltNonAssuredBandwidthPair.begin();
    }
    m_nonAssuredFirstServedTcont=m_vectorPairIterator;
  }
  else if(!m_allocOltBestEffortBandwidthPair.empty())
  {
    m_vectorPairIterator = m_bestEffortLastServedTcont;
    if(m_getNextBestEffortTcontAtBeginning)
    {
      m_vectorPairIterator++;
      m_getNextBestEffortTcontAtBeginning=false;
      if(m_vectorPairIterator==m_allocOltBestEffortBandwidthPair.end())
        m_vectorPairIterator=m_allocOltBestEffortBandwidthPair.begin();
    }
    m_bestEffortFirstServedTcont=m_vectorPairIterator;

    m_vectorPairIterator = m_allocOltBestEffortBandwidthPair.begin();
  }
  else
    return m_nullTcont;

  return m_vectorPairIterator->first;

}

const Ptr<XgponTcontOlt>&
XgponOltDbaEngineGiant::GetCurrentTcontOlt ( )
{
  NS_LOG_FUNCTION(this);
  return m_vectorPairIterator->first;
}



const Ptr<XgponTcontOlt>&
XgponOltDbaEngineGiant::GetNextTcontOlt ( )
{
  m_vectorPairIterator++;

  //Loop round robin vectors
  if (m_vectorPairIterator == (m_allocOltNonAssuredBandwidthPair.end()))
    m_vectorPairIterator = m_allocOltNonAssuredBandwidthPair.begin();
  if (m_vectorPairIterator == (m_allocOltBestEffortBandwidthPair.end()))
    m_vectorPairIterator = m_allocOltBestEffortBandwidthPair.begin();


  //All fixed bandwidth was served, move on to assured.
  if(m_vectorPairIterator == m_allocOltFixedBandwidthPair.end())
  {
    if(m_allocOltAssuredBandwidthPair.empty() && m_allocOltNonAssuredBandwidthPair.empty() && m_allocOltBestEffortBandwidthPair.empty())
      {m_stop=true; return m_nullTcont;}
    else
      m_vectorPairIterator = m_allocOltAssuredBandwidthPair.begin();
  }
  //All assured bandwidth was served, move on to non-assured or best effort if non-assured is empty.
  if (m_vectorPairIterator == m_allocOltAssuredBandwidthPair.end())
  {
    if(m_allocOltNonAssuredBandwidthPair.empty() && m_allocOltBestEffortBandwidthPair.empty())
      {m_stop=true; return m_nullTcont;}

    else if(!m_allocOltNonAssuredBandwidthPair.empty())
    {
      m_vectorPairIterator = m_nonAssuredLastServedTcont;
      if(m_getNextNonAssuredTcontAtBeginning)
      {
        m_vectorPairIterator++;
      }
      m_nonAssuredFirstServedTcont=m_vectorPairIterator;
      m_getNextNonAssuredTcontAtBeginning=false;
      return m_vectorPairIterator->first;
      //The function needs to return here, otherwise when checking for the all non-assured served
      //the loop could break prematurely if (m_vectorPairIterator == m_nonAssuredFirstServedTcont)
    }
    else
    {
      m_vectorPairIterator = m_bestEffortLastServedTcont;
      if(m_getNextBestEffortTcontAtBeginning)
      {
        m_vectorPairIterator++;
      }
      m_bestEffortFirstServedTcont=m_vectorPairIterator;
      m_getNextBestEffortTcontAtBeginning=false;
      return m_vectorPairIterator->first;
      //The function needs to return here, otherwise when checking for the all non-assured served
      //the loop could break prematurely at if (m_vectorPairIterator == m_besEffortFirstServedTcont)
    }
  }

  //All non-assured bandwidth was served, move on best-effort.
  if (m_vectorPairIterator == m_nonAssuredFirstServedTcont)
  {
    if(m_allocOltBestEffortBandwidthPair.empty())
      {m_stop=true; return m_nullTcont;}
    else
    {
      m_vectorPairIterator = m_bestEffortLastServedTcont;
      if(m_getNextBestEffortTcontAtBeginning)
      {
        m_vectorPairIterator++;
      }
      m_bestEffortFirstServedTcont=m_vectorPairIterator;
      m_getNextBestEffortTcontAtBeginning=false;
      return m_vectorPairIterator->first;
    }
  }
  //All best-effort bandwidth was served. The cycle is over.
  if(m_vectorPairIterator == m_bestEffortFirstServedTcont)
    {m_stop=true; return m_nullTcont;}

  return m_vectorPairIterator->first;
}


bool
XgponOltDbaEngineGiant::CheckAllTcontsServed ()
{
  return m_stop;
}




void
XgponOltDbaEngineGiant::UpdateAllTimers( )
{
  std::vector< std::pair<Ptr<XgponTcontOlt>,Ptr<XgponOltDbaParametersGiant> > >::iterator it;

  for (it =m_allocOltFixedBandwidthPair.begin(); it !=m_allocOltFixedBandwidthPair.end(); it++)
  {
    if((it->second->GetTimerValue())==XgponOltDbaEngineGiant::TIMER_EXPIRE_VALUE)
      it->second->ResetTimer();
    else
      it->second->UpdateTimer();
  }
  for (it =m_allocOltAssuredBandwidthPair.begin(); it !=m_allocOltAssuredBandwidthPair.end(); it++)
  {
    if((it->second->GetTimerValue())==XgponOltDbaEngineGiant::TIMER_EXPIRE_VALUE)
      it->second->ResetTimer();
    else
      it->second->UpdateTimer();
  }
  for (it =m_allocOltNonAssuredBandwidthPair.begin(); it !=m_allocOltNonAssuredBandwidthPair.end(); it++)
  {
    if((it->second->GetTimerValue())==XgponOltDbaEngineGiant::TIMER_EXPIRE_VALUE && m_bursts.CheckServedTcont(it->first->GetAllocId()))
      it->second->ResetTimer();
    else
      it->second->UpdateTimer();
  }
  for (it =m_allocOltBestEffortBandwidthPair.begin(); it !=m_allocOltBestEffortBandwidthPair.end(); it++)
  {
    if((it->second->GetTimerValue())==XgponOltDbaEngineGiant::TIMER_EXPIRE_VALUE && m_bursts.CheckServedTcont(it->first->GetAllocId()))
      it->second->ResetTimer();
    else
      it->second->UpdateTimer();
  }
  return;
}


void
XgponOltDbaEngineGiant::Prepare2ProduceBwmap ( )
{
}


void
XgponOltDbaEngineGiant::FinalizeBwmapProduction ( ){
  UpdateAllTimers();
}



uint32_t
XgponOltDbaEngineGiant::CalculateAmountData2Upload (const Ptr<XgponTcontOlt>& tcontOlt, uint32_t allocatedSize, uint64_t nowNano)
{
  NS_LOG_FUNCTION(this);
  NS_ASSERT_MSG(GetExtraInLastBwmap()==0, "GIANT DBA: More than what was supposed to was assigned in the Last BwMap");
  uint32_t size2Assign = 0;
  uint32_t sizeRemaining = 0;


  Ptr<XgponPhy> phy = m_device->GetXgponPhy();
  uint32_t usPhyFrameSize = phy->GetUsPhyFrameSizeInWord();
  Ptr<XgponBurstProfile> burstProfile = m_device->GetPloamEngine()->GetLinkInfo(tcontOlt->GetOnuId())->GetCurrentProfile();


  if((m_vectorPairIterator->second->GetBandwidthType()) == XGPON_GIANT_BW_FIXED)
  {
    if((m_vectorPairIterator->second->GetTimerValue()) == XgponOltDbaEngineGiant::TIMER_EXPIRE_VALUE)
      size2Assign = m_vectorPairIterator->second->GetAllocationBytes();
  }
  else if ((m_vectorPairIterator->second->GetBandwidthType()) == XGPON_GIANT_BW_ASSURED)
  {
    if((m_vectorPairIterator->second->GetTimerValue()) == XgponOltDbaEngineGiant::TIMER_EXPIRE_VALUE)
    {
      size2Assign = m_vectorPairIterator->first->CalculateRemainingDataToServe(GetRtt(), GetFrameSlotSize()); //requested bandwidth?
      if(size2Assign>0)
      {
        if (size2Assign<4)
          size2Assign =4; //smallest allocation for receiving data from ONU
        if(!CheckServedTcont(m_vectorPairIterator->first->GetAllocId()))
          size2Assign+=1; //This T-CONT was not served before in this bwMap, add one word for queue status report
        if(size2Assign>(m_vectorPairIterator->second->GetAllocationBytes()))
          size2Assign = m_vectorPairIterator->second->GetAllocationBytes();
      }
      else //size2assign <0, ensure the allocOlt is polled
      {
        uint64_t lastPollingTime = tcontOlt->GetLatestPollingTime();
        if((nowNano - lastPollingTime) > MAX_POLLING_INTERVAL) size2Assign = 1;
      }
    }
  }
   else
   {
     if((m_vectorPairIterator->second->GetTimerValue()) == XgponOltDbaEngineGiant::TIMER_EXPIRE_VALUE)
     {
       size2Assign = m_vectorPairIterator->first->CalculateRemainingDataToServe(GetRtt(), GetFrameSlotSize()); //requested bandwidth?
       if(size2Assign>0)
       {
         size2Assign+=1;
         if(size2Assign>(m_vectorPairIterator->second->GetAllocationBytes()))
           size2Assign = m_vectorPairIterator->second->GetAllocationBytes();
         else if (size2Assign<4)
           size2Assign =4;
       }
       else
       {
         uint64_t lastPollingTime = tcontOlt->GetLatestPollingTime();
         if((nowNano - lastPollingTime) > MAX_POLLING_INTERVAL) size2Assign = 1;
       }
     }
   }

  bool isNewBurstNecessary=m_bursts.IsNewBurstNecessary(tcontOlt);


  /**************************************
   *
   * Get the remaining size in a frame
   *
   **************************************/

  if(isNewBurstNecessary)
  {

    if(burstProfile->GetFec())
    {
      //Get the remaining size available for XGTC data when a new burst is needed and FEC is enabled.
      uint32_t fecBlocks=(2+size2Assign)/(phy->GetUsFecBlockDataSize()/4); //Divide by four to convert to words; Add two to take XGTC header and trailer into account
      if((2+size2Assign)%(phy->GetUsFecBlockDataSize()/4)!=0)
        fecBlocks++;
      uint32_t codeWords=fecBlocks*(phy->GetUsFecBlockSize()-phy->GetUsFecBlockDataSize())/4;
      sizeRemaining = usPhyFrameSize-allocatedSize-phy->GetUsMinimumGuardTime()-(burstProfile->GetPreambleLen()+burstProfile->GetDelimiterLen())/4-codeWords-2; //Two words for XGTC headers.

    }
    else
    {
      //Get the remaining size available for XGTC data when a new burst is needed and FEC is disabled.
      sizeRemaining = usPhyFrameSize-allocatedSize-phy->GetUsMinimumGuardTime()-(burstProfile->GetPreambleLen()+burstProfile->GetDelimiterLen())/4-2;
    }
  }
  else
  {
    if(burstProfile->GetFec())
    {
      //Get the remaining size available for XGTC data when the ONU has been served before and FEC is enabled.
      Ptr<XgponOltDbaPerBurstInfo> burstInfo=m_bursts.GetBurstInfo4TcontOlt(tcontOlt);
      uint32_t oldDataWords=burstInfo->GetHeaderTrailerDataSize()/phy->GetUsFecBlockDataSize()/4;
      uint32_t oldFecBlocks=oldDataWords/(phy->GetUsFecBlockDataSize()/4); //Divide by four to convert to words
      if(oldDataWords%(phy->GetUsFecBlockDataSize()/4)!=0)
        oldFecBlocks++;
      uint32_t oldCodeWords=oldFecBlocks*(phy->GetUsFecBlockSize()-phy->GetUsFecBlockDataSize())/4;


      uint32_t newFecBlocks=(2+size2Assign+oldDataWords)/(phy->GetUsFecBlockDataSize()/4); //Divide by four to convert to words; Add two to take XGTC header and trailer into account
      if((2+size2Assign+oldDataWords)%(phy->GetUsFecBlockDataSize()/4)!=0)
        newFecBlocks++;
      uint32_t newCodeWords=newFecBlocks*(phy->GetUsFecBlockSize()-phy->GetUsFecBlockDataSize())/4;
      sizeRemaining = usPhyFrameSize-(allocatedSize-oldCodeWords)-phy->GetUsMinimumGuardTime()-(burstProfile->GetPreambleLen()+burstProfile->GetDelimiterLen())/4-newCodeWords-2; //Two words for XGTC headers.

      /*
      Ptr<XgponOltDbaPerBurstInfo> burstInfo=m_bursts.GetBurstInfo4TcontOlt(tcontOlt);
      uint32_t oldDataWords=burstInfo->GetHeaderTrailerDataSize()/phy->GetUsFecBlockDataSize()/4;
      uint32_t oldFecBlocks=oldDataWords/(phy->GetUsFecBlockDataSize()/4); //Divide by four to convert to words
      if(oldDataWords%(phy->GetUsFecBlockDataSize()/4)!=0)
        oldFecBlocks++;
      uint32_t oldCodeWords=oldFecBlocks*(phy->GetUsFecBlockSize()-phy->GetUsFecBlockDataSize())/4;

      uint32_t newDataWords=oldDataWords+size2Assign; //The new burst should fill up the frame completely
      uint32_t newFecBlocks=newDataWords/(phy->GetUsFecBlockDataSize()/4); //Divide by four to convert to words
      uint32_t newCodeWords=newFecBlocks*(phy->GetUsFecBlockSize()-phy->GetUsFecBlockDataSize())/4;
      if(newDataWords%(phy->GetUsFecBlockDataSize()/4)!=0)
      {
        newFecBlocks++;
        uint32_t shortenedDataBlockSize=newDataWords%(phy->GetUsFecBlockDataSize()/4);
        newCodeWords=newCodeWords+shortenedDataBlockSize+(phy->GetUsFecBlockSize()-phy->GetUsFecBlockDataSize())/4;
      }
      */
      sizeRemaining = usPhyFrameSize-(allocatedSize-oldCodeWords);
    }
    else
    {
      //Get the remaining size available for XGTC data when the ONU has been served before and FEC is disabled.
      sizeRemaining = usPhyFrameSize-allocatedSize;
    }
  }


  /**************************************
   *
   * Prevent the bwMap from granting more than
   * what is allowed by the US PHY frame.
   *
   **************************************/
  if(size2Assign>sizeRemaining)
  {
    //There is not enough space to send all that the user requested
    if(burstProfile->GetFec())
    {
      //FEC is enabled. We need to compute what is the maximum amount of words we can transmit,
      //considering the FEC overhead will be smaller if the transmitted data is smaller.
      if(isNewBurstNecessary)
      {
        //Computing the maximum data transmission possible, bearing in mind that a new burst is necessary to serve the T-CONT.
        uint32_t sizeRemainingNoFec=usPhyFrameSize-allocatedSize-phy->GetUsMinimumGuardTime()-(burstProfile->GetPreambleLen()+burstProfile->GetDelimiterLen())/4;

        uint32_t fecBlocs=sizeRemainingNoFec/(phy->GetUsFecBlockSize()/4);
        uint32_t dataRemainder=0;
        if(sizeRemainingNoFec%(phy->GetUsFecBlockSize()/4)!=0)
        {
          dataRemainder=sizeRemainingNoFec-fecBlocs*(phy->GetUsFecBlockSize()/4)-(phy->GetUsFecBlockSize()-phy->GetUsFecBlockDataSize())/4;
        }
        sizeRemaining=fecBlocs*phy->GetUsFecBlockDataSize()/4+dataRemainder-2;
      }
      else
      {
        //Computing the maximum possible data transmission, bearing in mind that the ONU was served before.
        Ptr<XgponOltDbaPerBurstInfo> burstInfo=m_bursts.GetBurstInfo4TcontOlt(tcontOlt);
        uint32_t oldDataWords=burstInfo->GetHeaderTrailerDataSize()/phy->GetUsFecBlockDataSize()/4;
        uint32_t oldFecBlocks=oldDataWords/(phy->GetUsFecBlockDataSize()/4); //Divide by four to convert to words
        if(oldDataWords%(phy->GetUsFecBlockDataSize()/4)!=0)
          oldFecBlocks++;
        uint32_t oldCodeWords=oldFecBlocks*(phy->GetUsFecBlockSize()-phy->GetUsFecBlockDataSize())/4;

        uint32_t newCodedBurstSize=usPhyFrameSize-(allocatedSize-oldCodeWords-oldDataWords);
        uint32_t newFecBlocks = newCodedBurstSize/(phy->GetUsFecBlockSize()/4);
        uint32_t newDataWords = newFecBlocks*(phy->GetUsFecBlockDataSize()/4);
        if(newCodedBurstSize%(phy->GetUsFecBlockSize()/4)!=0)
        {
          newFecBlocks++;
          uint32_t shortenedCodedBlockSize=newCodedBurstSize%(phy->GetUsFecBlockSize()/4);
          newDataWords=newDataWords+shortenedCodedBlockSize-(phy->GetUsFecBlockSize()-phy->GetUsFecBlockDataSize())/4;
        }
        sizeRemaining=newDataWords-oldDataWords;
      }
    }
    return sizeRemaining;
  }
  else
  {
      return size2Assign;
  }

}




void
XgponOltDbaEngineGiant::SetTimerStartValue(uint16_t allocId, XgponGiantBandwidthType type, uint32_t initialValue)
{
  vector_type::iterator tempIterator;
  if (type==XGPON_GIANT_BW_FIXED)
  {
    for (tempIterator =m_allocOltFixedBandwidthPair.begin(); tempIterator !=m_allocOltFixedBandwidthPair.end();
    tempIterator++)
    {
      if(tempIterator->first->GetAllocId()==allocId)
        tempIterator->second->SetTimerValue(initialValue);
    }
  }
  else if (type==XGPON_GIANT_BW_ASSURED)
  {
    for (tempIterator =m_allocOltAssuredBandwidthPair.begin(); tempIterator !=m_allocOltAssuredBandwidthPair.end();
    tempIterator++)
    {
      if(tempIterator->first->GetAllocId()==allocId)
        tempIterator->second->SetTimerValue(initialValue);
    }
  }
  else if (type==XGPON_GIANT_BW_NON_ASSURED)
  {
    for (tempIterator =m_allocOltNonAssuredBandwidthPair.begin(); tempIterator !=m_allocOltNonAssuredBandwidthPair.end();
    tempIterator++)
    {
      if(tempIterator->first->GetAllocId()==allocId)
        tempIterator->second->SetTimerValue(initialValue);
    }
  }
  else if (type==XGPON_GIANT_BW_BEST_EFFORT)
  {
    for (tempIterator =m_allocOltBestEffortBandwidthPair.begin(); tempIterator !=m_allocOltBestEffortBandwidthPair.end();
    tempIterator++)
    {
      if(tempIterator->first->GetAllocId()==allocId)
        tempIterator->second->SetTimerValue(initialValue);
    }
  }
}

//this function is not used at the moment- it's there in case we need to set SImin timer manually (take it out of qos parameters)
void
XgponOltDbaEngineGiant::SetServiceInterval(uint16_t allocId, XgponGiantBandwidthType type, uint32_t serviceInterval)
{
  vector_type::iterator tempIterator;
  if (type==XGPON_GIANT_BW_FIXED)
  {
    for (tempIterator =m_allocOltFixedBandwidthPair.begin(); tempIterator !=m_allocOltFixedBandwidthPair.end();
    tempIterator++)
    {
      if(tempIterator->first->GetAllocId()==allocId)
        tempIterator->second->SetServiceInterval(serviceInterval);
    }
  }
  if (type==XGPON_GIANT_BW_ASSURED)
  {
    for (tempIterator =m_allocOltAssuredBandwidthPair.begin(); tempIterator !=m_allocOltAssuredBandwidthPair.end();
    tempIterator++)
    {
      if(tempIterator->first->GetAllocId()==allocId)
        tempIterator->second->SetTimerValue(serviceInterval);
    }
  }
  if (type==XGPON_GIANT_BW_NON_ASSURED)
  {
    for (tempIterator =m_allocOltNonAssuredBandwidthPair.begin(); tempIterator !=m_allocOltNonAssuredBandwidthPair.end();
    tempIterator++)
    {
      if(tempIterator->first->GetAllocId()==allocId)
        tempIterator->second->SetTimerValue(serviceInterval);
    }
  }
  if (type==XGPON_GIANT_BW_BEST_EFFORT)
  {
    for (tempIterator =m_allocOltBestEffortBandwidthPair.begin(); tempIterator !=m_allocOltBestEffortBandwidthPair.end();
    tempIterator++)
    {
      if(tempIterator->first->GetAllocId()==allocId)
        tempIterator->second->SetTimerValue(serviceInterval);
    }
  }
}

uint32_t
XgponOltDbaEngineGiant::GetAllocationBytesFromRateAndServiceInterval(uint32_t rate, uint16_t si)
{
  //uint128_t tmp128;
  uint64_t tmp64;
  uint32_t tmp32;


  tmp64=(uint64_t)rate*(uint64_t)GetFrameSlotSize();
  tmp64=tmp64*(uint64_t)si;                           //rate is in bps and frame slot size is in nanoseconds
  tmp64=tmp64/1000000000;                   //Get value in bits
  NS_ASSERT_MSG(tmp64%(32)==0,"Cannot assign that rate to the connection since it will not be a multiple of 4 bytes (one word).");
  tmp32=tmp64/32;                            //Convert bits to words

  return tmp32;

}




}//namespace ns3
