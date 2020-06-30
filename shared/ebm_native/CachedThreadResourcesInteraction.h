// Copyright (c) 2018 Microsoft Corporation
// Licensed under the MIT license.
// Author: Paul Koch <code@koch.ninja>

#ifndef CACHED_INTERACTION_THREAD_RESOURCES_H
#define CACHED_INTERACTION_THREAD_RESOURCES_H

#include <stdlib.h> // free
#include <stddef.h> // size_t, ptrdiff_t

#include "EbmInternal.h" // EBM_INLINE
#include "Logging.h" // EBM_ASSERT & LOG

struct HistogramBucketBase;

class CachedInteractionThreadResources final {
   HistogramBucketBase * m_aThreadByteBuffer1;
   size_t m_cThreadByteBufferCapacity1;

public:

   CachedInteractionThreadResources() = default; // preserve our POD status
   ~CachedInteractionThreadResources() = default; // preserve our POD status
   void * operator new(std::size_t) = delete; // we only use malloc/free in this library
   void operator delete (void *) = delete; // we only use malloc/free in this library

   EBM_INLINE void InitializeZero() {
      m_aThreadByteBuffer1 = nullptr;
      m_cThreadByteBufferCapacity1 = 0;
   }

   INLINE_RELEASE void Free() {
      LOG_0(TraceLevelInfo, "Entered CachedInteractionThreadResources::Free");

      free(m_aThreadByteBuffer1);

      free(this);

      LOG_0(TraceLevelInfo, "Exited CachedInteractionThreadResources::Free");
   }

   INLINE_RELEASE static CachedInteractionThreadResources * Allocate() {
      LOG_0(TraceLevelInfo, "Entered CachedInteractionThreadResources::Allocate");

      CachedInteractionThreadResources * const pNew = EbmMalloc<CachedInteractionThreadResources>();
      if(nullptr != pNew) {
         pNew->InitializeZero();
      }

      LOG_0(TraceLevelInfo, "Exited CachedInteractionThreadResources::Allocate");

      return pNew;
   }

   INLINE_RELEASE HistogramBucketBase * GetThreadByteBuffer1(const size_t cBytesRequired) {
      HistogramBucketBase * aBuffer = m_aThreadByteBuffer1;
      if(UNLIKELY(m_cThreadByteBufferCapacity1 < cBytesRequired)) {
         m_cThreadByteBufferCapacity1 = cBytesRequired << 1;
         LOG_N(TraceLevelInfo, "Growing CachedInteractionThreadResources::ThreadByteBuffer1 to %zu", m_cThreadByteBufferCapacity1);

         free(aBuffer);
         aBuffer = static_cast<HistogramBucketBase *>(EbmMalloc<void>(m_cThreadByteBufferCapacity1));
         m_aThreadByteBuffer1 = aBuffer;
      }
      return aBuffer;
   }
};
static_assert(std::is_standard_layout<CachedInteractionThreadResources>::value,
   "We use the struct hack in several places, so disallow non-standard_layout types in general");
static_assert(std::is_trivial<CachedInteractionThreadResources>::value,
   "We use memcpy in several places, so disallow non-trivial types in general");
static_assert(std::is_pod<CachedInteractionThreadResources>::value,
   "We use a lot of C constructs, so disallow non-POD types in general");

#endif // CACHED_INTERACTION_THREAD_RESOURCES_H
