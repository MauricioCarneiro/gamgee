#ifndef __gamgee__sam_pair_iterator__
#define __gamgee__sam_pair_iterator__

#include "sam.h"

#include "htslib/sam.h"

#include <fstream>
#include <queue>
#include <memory>

namespace gamgee {

  /**
   * @brief private namespace to contain htslib specific wrappers
   */
  namespace hts {
    /** 
     * @brief a functor object to delete a bam1 pointer 
     * 
     * used by the unique_ptr queue used in the paired reader
     */
    struct BamDeleter {
      void operator()(bam1_t* p) const { bam_destroy1(p); }
    };
  }

/**
 * @brief Utility class to enable for-each style iteration by pairs in the SamReader class
 */
class SamPairIterator {
  public:

    /**
     * @brief creates an empty iterator (used for the end() method) 
     */
    SamPairIterator();

    /**
     * @brief initializes a new iterator based on an input stream (e.g. sam/a file, stdin, ...)
     *
     * @param sam_file_ptr   pointer to a sam file opened via the sam_open() macro from htslib
     * @param sam_header_ptr pointer to a sam file header created with the sam_hdr_read() macro from htslib
     */
    SamPairIterator(samFile * sam_file_ptr, bam_hdr_t * sam_header_ptr);

    /**
     * @brief a SamPairIterator move constructor guarantees all objects will have the same state.
     */
    SamPairIterator(SamPairIterator&&);
    
    /**
     * @brief inequality operator (needed by for-each loop)
     *
     * @param rhs the other SamPairIterator to compare to
     *
     * @return whether or not the two iterators are the same (e.g. have the same input stream on the same
     * status)
     */
    bool operator!=(const SamPairIterator& rhs);

    /**
     * @brief dereference operator (needed by for-each loop)
     *
     * @return a persistent Sam object independent from the iterator (a copy of the iterator's object)
     */
    std::pair<Sam,Sam> operator*();

    /**
     * @brief pre-fetches the next record and tests for end of file
     *
     * @return a reference to the object (it can be const& because this return value should only be used 
     *         by the for-each loop to check for the eof)
     */
    std::pair<Sam,Sam> operator++();

    /**
     * @brief takes care of all the memory allocations of the htslib sam reader interface
     */
    ~SamPairIterator();
    
  private:

    using SamPtrQueue = std::queue<std::unique_ptr<bam1_t, hts::BamDeleter>>;

    SamPtrQueue supp_alignments;          ///< queue to hold the supplementary alignments temporarily while processing the pairs
    samFile * sam_file_ptr;               ///< pointer to the sam file
    bam_hdr_t * sam_header_ptr;           ///< pointer to the sam header
    bam1_t * sam_record_ptr;              ///< pointer to the internal structure of the sam record. Useful to only allocate it once.
    std::pair<Sam,Sam> sam_records;       ///< temporary record to hold between fetch (operator++) and serve (operator*)

    std::pair<Sam,Sam> fetch_next_pair(); ///< makes a new (through copy) pair of Sam objects that the user is free to use/keep without having to worry about memory management
    bool read_sam();                      ///< reads a sam record and checks for the end-of-file invalidating the file and header pointers if necessary
    Sam make_sam();                       ///< creates a sam record from the internal data
    Sam next_primary_alignment();
    std::pair<Sam,Sam> next_supplementary_alignment();
    bool not_primary() const;
};

}  // end namespace gamgee

#endif