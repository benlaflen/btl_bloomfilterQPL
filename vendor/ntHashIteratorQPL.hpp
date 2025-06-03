#ifndef NTHASH__ITERATOR_QPL_H
#define NTHASH__ITERATOR_QPL_H 1

#include <string>
#include <limits>
#include <chrono>
#include "nthash.hpp"


/**
 * Iterate over hash values for k-mers in a
 * given DNA sequence.
 *
 * This implementation uses ntHash
 * function to efficiently calculate
 * hash values for successive k-mers.
 */

class ntHashIteratorQPL
{

public:

    /**
     * Default constructor. Creates an iterator pointing to
     * the end of the iterator range.
    */
    ntHashIteratorQPL():
        m_hVec(NULL),
        m_pos(std::numeric_limits<std::size_t>::max())
    {}

    /**
     * Constructor.
     * @param seq address of DNA sequence to be hashed
     * @param k k-mer size
     * @param h number of hashes
     * @param s size of filter in bytes
    */
    ntHashIteratorQPL(const std::string& seq, unsigned h, unsigned k, unsigned s, size_t pos = 0):
        m_seq(seq), m_h(h), m_s(s), m_k(k), m_hVec(new uint64_t[s/8]), m_pos(pos)
    {
        init();
    }
    
    /** Copy constructor */
    ntHashIteratorQPL(const ntHashIteratorQPL &nth)
    {
        m_seq = nth.m_seq;
        m_h = nth.m_h;
        m_s = nth.m_s;
        m_k = nth.m_k;
        m_hVec = nth.m_hVec;
        for (unsigned i=0; i<m_h; i++) m_hVec[i] = nth.m_hVec[i];
        m_pos = nth.m_pos;
        m_fhVal = nth.m_fhVal;
        m_rhVal = nth.m_rhVal;
    }
    

    /** Initialize internal state of iterator */
    void init()
    {
        if (m_k > m_seq.length()) {
            m_pos = std::numeric_limits<std::size_t>::max();
            return;
        }
        unsigned locN=0;
        while (m_pos<m_seq.length()-m_k+1 && !NTMC64QPL(m_seq.data()+m_pos, m_k, m_h, m_s, m_fhVal, m_rhVal, locN, m_hVec))
            m_pos+=locN+1;
        if (m_pos >= m_seq.length()-m_k+1)
            m_pos = std::numeric_limits<std::size_t>::max();
    }

    /** Advance iterator right to the next valid k-mer */
    void next()
    {
        ++m_pos;
        if (m_pos >= m_seq.length()-m_k+1) {
            m_pos = std::numeric_limits<std::size_t>::max();
            return;
        }
        if(seedTab[(unsigned char)(m_seq.at(m_pos+m_k-1))]==seedN) {
            m_pos+=m_k;
            init();
        }
        else
            auto start = std::chrono::high_resolution_clock::now();
            NTMC64QPL(m_seq.at(m_pos-1), m_seq.at(m_pos-1+m_k), m_k, m_h, m_s, m_fhVal, m_rhVal, m_hVec);
            auto end = std::chrono::high_resolution_clock::now();
            std::chrono::duration<double, std::micro> elapsed = end - start;
            std::cout<<"Hashing took: " << elapsed.count() << "us\n\n";
    }
    
    size_t pos() const{
    	return m_pos;
    }

    /** get pointer to hash values for current k-mer */
    const uint64_t* operator*() const
    {
        return m_hVec;
    }

    /** test equality with another iterator */
    bool operator==(const ntHashIteratorQPL& it) const
    {
        return m_pos == it.m_pos;
    }

    /** test inequality with another iterator */
    bool operator!=(const ntHashIteratorQPL& it) const
    {
        return !(*this == it);
    }

    /** pre-increment operator */
    ntHashIteratorQPL& operator++()
    {
        next();
        return *this;
    }

    /** iterator pointing to one past last element */
    static const ntHashIteratorQPL end()
    {
        return ntHashIteratorQPL();
    }

    /** destructor */
    ~ntHashIteratorQPL() {
        if(m_hVec!=NULL)
            delete [] m_hVec;
    }

private:

    /** DNA sequence */
    std::string m_seq;

    /** number of hashes */
    unsigned m_h;

    /** size of filter */
    unsigned m_s;

    /** k-mer size */
    unsigned m_k;

    /** hash values */
    uint64_t *m_hVec;

    /** position of current k-mer */
    size_t m_pos;

    /** forward-strand k-mer hash value */
    uint64_t m_fhVal;

    /** reverse-complement k-mer hash value */
    uint64_t m_rhVal;
};

#endif
