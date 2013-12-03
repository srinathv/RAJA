/*!
 ******************************************************************************
 *
 * \file
 *
 * \brief   Implementation file for hybrid index set classes
 *
 * \author  Rich Hornung, Center for Applied Scientific Computing, LLNL
 * \author  Jeff Keasler, Applications, Simulations And Quality, LLNL
 *
 ******************************************************************************
 */

#include "RAJA/HybridISet.hxx"

#include <cstdlib>
#include <iostream>


namespace RAJA {


/*
*************************************************************************
*
* Public HybridISet methods.
*
*************************************************************************
*/

HybridISet::HybridISet()
: m_len(0)
{
}

HybridISet::HybridISet(const HybridISet& other)
: m_len(0)
{
   copy(other); 
}

HybridISet& HybridISet::operator=(
   const HybridISet& rhs)
{
   if ( &rhs != this ) {
      HybridISet copy(rhs);
      this->swap(copy);
   }
   return *this;
}

HybridISet::~HybridISet()
{
   const int num_segs = m_segments.size();
   for ( int isi = 0; isi < num_segs; ++isi ) {
      Segment& seg = m_segments[isi];

      switch ( seg.m_type ) {

         case _Range_ : {
            if ( seg.m_segment ) {
               RangeISet* is =
                  const_cast<RangeISet*>(
                     static_cast<const RangeISet*>(seg.m_segment)
                  );
               delete is;
            }
            break;
         }

#if 0  // RDH RETHINK
         case _RangeStride_ : {
            if ( seg.m_segment ) {
               RangeStrideISet* is =
                  const_cast<RangeStrideISet*>(
                     static_cast<const RangeStrideISet*>(seg.m_segment)
                  );
               delete is;
            }
            break;
         }
#endif

         case _Unstructured_ : {
            if ( seg.m_segment ) {
               UnstructuredISet* is =
                  const_cast<UnstructuredISet*>(
                     static_cast<const UnstructuredISet*>(seg.m_segment)
                  );
               delete is;
            }
            break;
         }

         default : {
            std::cout << "\t HybridISet dtor: case not implemented!!\n";
         }

      } // iterate over segments of hybrid index set
   }
}

void HybridISet::swap(HybridISet& other)
{
   using std::swap;
   swap(m_len, other.m_len);
   swap(m_segments, other.m_segments);
}


/*
*************************************************************************
*
* Methods to add copies of index set objects to hybrid index set.
*
*************************************************************************
*/

template <>
void HybridISet::addISet(const RangeISet& index_set)
{
   RangeISet* new_is = new RangeISet(index_set);
   m_segments.push_back( Segment( _Range_, new_is ) );

   m_len += new_is->getLength();
}

#if 0  // RDH RETHINK
template <>
void HybridISet::addISet(const RangeStrideISet& index_set)
{
   RangeStrideISet* new_is = new RangeStrideISet(index_set);
   m_segments.push_back( Segment( _RangeStride_, new_is ) );

   m_len += new_is->getLength();
}
#endif

template <>
void HybridISet::addISet(const UnstructuredISet& index_set)
{
   UnstructuredISet* new_is = new UnstructuredISet(index_set);
   m_segments.push_back( Segment( _Unstructured_, new_is ) );

   m_len += new_is->getLength();
}


/*
*************************************************************************
*
* Methods to add index set objects to hybrid index set.
*
*************************************************************************
*/

void HybridISet::addRangeIndices(Index_type begin, Index_type end)
{
   RangeISet* new_is = new RangeISet(begin, end);
   m_segments.push_back( Segment( _Range_, new_is ) );

   m_len += new_is->getLength();
}

#if 0  // RDH RETHINK
void HybridISet::addRangeStrideIndices(Index_type begin, Index_type end,
                                       Index_type stride)
{
   RangeStrideISet* new_is = new RangeStrideISet(begin, end, stride);
   m_segments.push_back( Segment( _RangeStride_, new_is ) );

   m_len += new_is->getLength() / new_is->getStride();
}
#endif

void HybridISet::addUnstructuredIndices(const Index_type* indx, 
                                        Index_type len)
{
   UnstructuredISet* new_is = new UnstructuredISet(indx, len);
   m_segments.push_back( Segment( _Unstructured_, new_is ) );

   m_len += new_is->getLength();
}


/*
*************************************************************************
*
* Print contents of hybrid index set to given output stream.
*
*************************************************************************
*/

void HybridISet::print(std::ostream& os) const
{
   os << "HYBRID INDEX SET : " 
      << getLength() << " length..." << std::endl
      << getNumSegments() << " segments..." << std::endl;

   const int num_segs = m_segments.size();
   for ( int isi = 0; isi < num_segs; ++isi ) {
      const Segment& seg = m_segments[isi];

      switch ( seg.m_type ) {

         case _Range_ : {
            if ( seg.m_segment ) {
               const RangeISet* is =
                  static_cast<const RangeISet*>(seg.m_segment);
               is->print(os);
            } else {
               os << "_Range_ is null" << std::endl;
            }
            break;
         }

#if 0  // RDH RETHINK
         case _RangeStride_ : {
            if ( seg.m_segment ) {
               const RangeStrideISet* is =
                  static_cast<const RangeStrideISet*>(seg.m_segment);
               is->print(os);
            } else {
               os << "_RangeStride_ is null" << std::endl;
            }
            break;
         }
#endif

         case _Unstructured_ : {
            if ( seg.m_segment ) {
               const UnstructuredISet* is =
                  static_cast<const UnstructuredISet*>(seg.m_segment);
               is->print(os);
            } else {
               os << "_Unstructured_ is null" << std::endl;
            }
            break;
         }

         default : {
            os << "HybridISet print: case not implemented!!\n";
         }

      }  // switch ( is_pair.first )

   } // iterate over segments of hybrid index set
}


/*
*************************************************************************
*
* Private helper function to copy hybrid index set segments.
*
*************************************************************************
*/
void HybridISet::copy(const HybridISet& other)
{
   const int num_segs = other.m_segments.size();
   for ( int isi = 0; isi < num_segs; ++isi ) {
      const Segment& seg = other.m_segments[isi];

      switch ( seg.m_type ) {

         case _Range_ : {
            addISet(*static_cast<const RangeISet*>(seg.m_segment));
            break;
         }

#if 0  // RDH RETHINK
         case _RangeStride_ : {
            addISet(*static_cast<const RangeStrideISet*>(seg.m_segment));
            break;
         }
#endif

         case _Unstructured_ : {
            addISet(*static_cast<const UnstructuredISet*>(seg.m_segment));
            break;
         }

         default : {
            std::cout << "\t HybridISet copySegments: case not implemented!!\n";
         }

      } // iterate over segments of hybrid index set
   }
}



/*
*************************************************************************
*
* HybridISet builder methods.
*
*************************************************************************
*/

HybridISet* buildHybridISet(const Index_type* const indices_in, 
                                    Index_type length)
{
   HybridISet* hindex = new HybridISet();

   /* only transform relatively large */
   if (length > RANGE_MIN_LENGTH) {
      /* build a rindex array from an index array */
      Index_type docount = 0 ;
      Index_type inrange = -1 ;

      /****************************/
      /* first, gather statistics */
      /****************************/

      Index_type scanVal = indices_in[0] ;
      Index_type sliceCount = 0 ;
      for (Index_type ii=1; ii<length; ++ii) {
         Index_type lookAhead = indices_in[ii] ;

         if (inrange == -1) {
            if ( (lookAhead == scanVal+1) && 
                 ((scanVal % RANGE_ALIGN) == 0) ) {
              inrange = 1 ;
            }
            else {
              inrange = 0 ;
            }
         }

         if (lookAhead == scanVal+1) {
            if ( (inrange == 0) && ((scanVal % RANGE_ALIGN) == 0) ) {
               if (sliceCount != 0) {
                  docount += 1 + sliceCount ; /* length + singletons */
               }
               inrange = 1 ;
               sliceCount = 0 ;
            }
            ++sliceCount ;  /* account for scanVal */
         }
         else {
            if (inrange == 1) {
               /* we can tighten this up by schleping any trailing */
               /* sigletons off into the subsequent singleton */
               /* array.  We would then also need to recheck the */
               /* final length of the range to make sure it meets */
               /* our minimum length crietria.  If it doesnt, */
               /* we need to emit a random array instead of */
               /* a range array */
               ++sliceCount ;
               docount += 2 ; /* length + begin */
               inrange = 0 ;
               sliceCount = 0 ;
            }
            else {
              ++sliceCount ;  /* account for scanVal */
            }
         }

         scanVal = lookAhead ;
      }  // end loop to gather statistics

      if (inrange != -1) {
         if (inrange) {
            ++sliceCount ;
            docount += 2 ; /* length + begin */
         }
         else {
            ++sliceCount ;
            docount += 1 + sliceCount ; /* length + singletons */
         }
      }
      else if (scanVal != -1) {
         ++sliceCount ;
         docount += 2 ;
      }
      ++docount ; /* zero length termination */

      /* What is the cutoff criteria for generating the rindex array? */
      if (docount < (length*(RANGE_ALIGN-1))/RANGE_ALIGN) {
         /* The rindex array can either contain a pointer into the */
         /* original index array, *or* it can repack the data from the */
         /* original index array.  Benefits of repacking could include */
         /* better use of hardware prefetch streams, or guaranteeing */
         /* alignment of index array segments. */

         /*******************************/
         /* now, build the rindex array */
         /*******************************/

         Index_type dobegin ;
         inrange = -1 ;

         scanVal = indices_in[0] ;
         sliceCount = 0 ;
         dobegin = scanVal ;
         for (Index_type ii=1; ii < length; ++ii) {
            Index_type lookAhead = indices_in[ii] ;

            if (inrange == -1) {
               if ( (lookAhead == scanVal+1) && 
                    ((scanVal % RANGE_ALIGN) == 0) ) {
                 inrange = 1 ;
               }
               else {
                 inrange = 0 ;
                 dobegin = ii-1 ;
               }
            }
            if (lookAhead == scanVal+1) {
               if ( (inrange == 0) && 
                    ((scanVal % RANGE_ALIGN) == 0) ) {
                  if (sliceCount != 0) {
                     hindex->addUnstructuredIndices(&indices_in[dobegin], sliceCount);
                  }
                  inrange = 1 ;
                  dobegin = scanVal ;
                  sliceCount = 0 ;
               }
               ++sliceCount ;  /* account for scanVal */
            }
            else {
               if (inrange == 1) {
               /* we can tighten this up by schleping any trailing */
               /* sigletons off into the subsequent singleton */
               /* array.  We would then also need to recheck the */
               /* final length of the range to make sure it meets */
               /* our minimum length crietria.  If it doesnt, */
               /* we need to emit a random array instead of */
               /* a range array */
                  ++sliceCount ;
                  hindex->addRangeIndices(dobegin, dobegin+sliceCount);
                  inrange = 0 ;
                  sliceCount = 0 ;
                  dobegin = ii ;
               }
               else {
                 ++sliceCount ;  /* account for scanVal */
               }
            }

            scanVal = lookAhead ;
         }  // for (Index_type ii ...

         if (inrange != -1) {
            if (inrange) {
               ++sliceCount ;
               hindex->addRangeIndices(dobegin, dobegin+sliceCount);
            }
            else {
               ++sliceCount ;
               hindex->addUnstructuredIndices(&indices_in[dobegin], sliceCount);
            }
         }
         else if (scanVal != -1) {
            hindex->addUnstructuredIndices(&scanVal, 1);
         }
      }
      else {  // !(docount < (length*RANGE_ALIGN-1))/RANGE_ALIGN)
         hindex->addUnstructuredIndices(indices_in, length);
      }
   }
   else {  // else !(length > RANGE_MIN_LENGTH)
      hindex->addUnstructuredIndices(indices_in, length);
   }

   return( hindex );
}

}  // closing brace for namespace statement