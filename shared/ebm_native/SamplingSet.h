// Copyright (c) 2018 Microsoft Corporation
// Licensed under the MIT license.
// Author: Paul Koch <code@koch.ninja>

#ifndef SAMPLING_SET_H
#define SAMPLING_SET_H

#include <stddef.h> // size_t, ptrdiff_t

#include "EbmInternal.h" // EBM_INLINE
#include "Logging.h" // EBM_ASSERT & LOG

class RandomStream;
class DataSetByFeatureCombination;

class SamplingSet final {
   // Sampling with replacement is the more theoretically correct method of sampling, but it has the drawback that 
   // we need to keep a count of the number of times each instance is selected in the dataset.  
   // Sampling without replacement would require 1 bit per case, so it can be faster.

   const DataSetByFeatureCombination * m_pOriginDataSet;

   // TODO : make this a struct of FractionalType and size_t counts and use MACROS to have either size_t or 
   // FractionalType or both, and perf how this changes things.  We don't get a benefit anywhere by storing 
   // the raw data in both formats since it is never converted anyways, but this count is!
   size_t * m_aCountOccurrences;

   // we take owernship of the aCounts array.  We do not take ownership of the pOriginDataSet since many 
   // SamplingSet objects will refer to the original one
   static SamplingSet * GenerateSingleSamplingSet(
      RandomStream * const pRandomStream, 
      const DataSetByFeatureCombination * const pOriginDataSet
   );
   static SamplingSet * GenerateFlatSamplingSet(const DataSetByFeatureCombination * const pOriginDataSet);

public:

   SamplingSet() = default; // preserve our POD status
   ~SamplingSet() = default; // preserve our POD status
   void * operator new(std::size_t) = delete; // we only use malloc/free in this library
   void operator delete (void *) = delete; // we only use malloc/free in this library

   size_t GetTotalCountInstanceOccurrences() const {
      // for SamplingSet (bootstrap sampling), we have the same number of instances as our original dataset
      size_t cTotalCountInstanceOccurrences = m_pOriginDataSet->GetCountInstances();
#ifndef NDEBUG
      size_t cTotalCountInstanceOccurrencesDebug = 0;
      for(size_t i = 0; i < m_pOriginDataSet->GetCountInstances(); ++i) {
         cTotalCountInstanceOccurrencesDebug += m_aCountOccurrences[i];
      }
      EBM_ASSERT(cTotalCountInstanceOccurrencesDebug == cTotalCountInstanceOccurrences);
#endif // NDEBUG
      return cTotalCountInstanceOccurrences;
   }

   const DataSetByFeatureCombination * GetDataSetByFeatureCombination() const {
      return m_pOriginDataSet;
   }

   const size_t * GetCountOccurrences() const {
      return m_aCountOccurrences;
   }

   static void FreeSamplingSets(const size_t cSamplingSets, SamplingSet ** const apSamplingSets);
   static SamplingSet ** GenerateSamplingSets(
      RandomStream * const pRandomStream, 
      const DataSetByFeatureCombination * const pOriginDataSet, 
      const size_t cSamplingSets
   );
};
static_assert(std::is_standard_layout<SamplingSet>::value,
   "We use the struct hack in several places, so disallow non-standard_layout types in general");
static_assert(std::is_trivial<SamplingSet>::value,
   "We use memcpy in several places, so disallow non-trivial types in general");
static_assert(std::is_pod<SamplingSet>::value,
   "We use a lot of C constructs, so disallow non-POD types in general");

#endif // SAMPLING_SET_H
