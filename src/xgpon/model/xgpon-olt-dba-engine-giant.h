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
* Authors: Heather Crowley <crowleyh@tcd.ie>
*          Pedro Alvarez <pinheirp@tcd.ie>
*/

#ifndef XGPON_OLT_DBA_ENGINE_GIANT_H_
#define XGPON_OLT_DBA_ENGINE_GIANT_H_

#include "ns3/object.h"
#include "xgpon-olt-dba-engine.h"
#include "xgpon-olt-dba-per-burst-info.h"
#include "xgpon-olt-dba-parameters-giant.h"

namespace ns3 {

class XgponOltDbaEngineGiant : public XgponOltDbaEngine
{
public:
  const static uint32_t ALLOC_PER_SERVICE_MAX_SIZE=1000;    //1K words (4Kbytes). TODO: replace with one attribute
  const static uint32_t MAX_POLLING_INTERVAL=10000000;      //10ms. Unit: nanosecond
  const static uint32_t TIMER_EXPIRE_VALUE=0;               //Value at which the timer expires. Unit: frames.


  /**
   * \brief Constructor
   */
  XgponOltDbaEngineGiant ();
  virtual ~XgponOltDbaEngineGiant ();



  /**
    * \brief Creates vector pairs linking AllocOlts to GiantParameters. The GiantParameters are
    * obtained from the qos parameters for each AllocOlt, taking TCont Type into account.
    * \param void
    */
  void GenerateAllocOltGiantParameterPairs();

  /**
    * \brief sets the initial value of the serviceIntervalTimer of a specific alloc
    * \param takes 3 parameters: AllocId, bandwidth type and timer start value.
    *
    */
  void SetTimerStartValue(uint16_t allocId, XgponGiantBandwidthType type, uint32_t initialValue);


  /**
    * \brief function is called in AllTConts served
    * \param
    */
  void UpdateAllTimers( );


  /**
      * \brief sets serviceInterval of a specific alloc
      * \param takes 3 parameters: AllocId, bandwidth type and serviceInterval.
      *
      */
  void SetServiceInterval(uint16_t allocId, XgponGiantBandwidthType type, uint32_t serviceInterval);

  /**
   * \brief Add Alloc-Id info into the DBA Engine
   * \param the Alloc-Id to be added to the engine
   */
  virtual void  AddTcontToDbaEngine (const ns3::Ptr<ns3::XgponTcontOlt>&);

  //returns m_vectorpair iterator to the beginning
  virtual void Prepare2ProduceBwmap();

  /**
   *  \breif Decrements or resets the SImin and SImax timers in the vector pairs
   */
  virtual void FinalizeBwmapProduction();

  //checks if all tconts have been served
  virtual bool CheckAllTcontsServed ();

  //Functions required by NS-3
  static TypeId GetTypeId (void);
  virtual TypeId GetInstanceTypeId (void) const;

  void DoInitialize(void);

private:

  /**
   * \brief return the next Alloc-ID to be considered. Note that the scheduled AllocType and AllocIndex are updated.
   * m_vectorPairIterator is are also updated
   * \return the T-CONT to be considered. 0 means that there is no T-CONT in this XG-PON network
   */
  virtual const Ptr<XgponTcontOlt>& GetNextTcontOlt ( );


  const Ptr<XgponTcontOlt>& GetCurrentTcontOlt ( );

  /**
   * \brief return the first T-CONT to be considered. m_vectorPairIterator and m_vectorTypeKey are also updated
   * \return the T-CONT to be considered.
   */
  virtual const Ptr<XgponTcontOlt>& GetFirstTcontOlt ( );

  //Calculate the amount of data to be sent for the AllocOlt
  virtual uint32_t CalculateAmountData2Upload (const Ptr<XgponTcontOlt>& allocOlt,uint32_t allocatedSize, uint64_t nowNano);

  /**
   *\brief checks if all tconts have been served i.e if the iterator has returned to the
   *\ beginning of m_allocOltFixedBandwidthPair
   *\ return true if all served
   */

  uint32_t GetAllocationBytesFromRateAndServiceInterval(uint32_t rate, uint16_t si);

private:
  uint16_t m_lastScheduledAllocIndex;  //the index in this alloc-type list that has been scheduled most recently
  std::vector< Ptr<XgponTcontOlt> > m_usAllTcons;
  Ptr<XgponTcontOlt> m_nullTcont;      // Pointer used to return a null T-CONT

  //vector pairs linking AllocOlt to GiantParameter
  std::vector<std::pair<Ptr<XgponTcontOlt>,Ptr<XgponOltDbaParametersGiant> > >  m_allocOltFixedBandwidthPair;
  std::vector<std::pair<Ptr<XgponTcontOlt>,Ptr<XgponOltDbaParametersGiant> > >  m_allocOltAssuredBandwidthPair;
  std::vector<std::pair<Ptr<XgponTcontOlt>,Ptr<XgponOltDbaParametersGiant> > >  m_allocOltNonAssuredBandwidthPair;
  std::vector<std::pair<Ptr<XgponTcontOlt>,Ptr<XgponOltDbaParametersGiant> > >  m_allocOltBestEffortBandwidthPair;

  /**
    * member variable used to iterate through the vector pairs in CalculateAmountData2Upload
    * it is updated in GetNextTcontOlt
    */
  typedef std::vector<std::pair<Ptr<XgponTcontOlt>,Ptr<XgponOltDbaParametersGiant> > >  vector_type;
  vector_type::iterator m_vectorPairIterator;
  vector_type::iterator m_nonAssuredFirstServedTcont;
  vector_type::iterator m_bestEffortFirstServedTcont;
  vector_type::iterator m_nonAssuredLastServedTcont;
  vector_type::iterator m_bestEffortLastServedTcont;

  bool m_getNextNonAssuredTcontAtBeginning;
  bool m_getNextBestEffortTcontAtBeginning;

  bool m_stop;

};


#endif /* XGPON_OLT_DBA_ENGINE_GIANT_H_ */

} // namespace ns3
