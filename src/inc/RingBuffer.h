#if !defined(__RingBuffer_hdr__)
#define __RingBuffer_hdr__

#include <cassert>
#include <algorithm>
#include <cmath>
#include <cstring>

/*! \brief implement a circular buffer of type T
*/
template <class T>
class CRingBuffer
{
public:
    explicit CRingBuffer(int iBufferLengthInSamples) :
        m_iBuffLength(iBufferLengthInSamples),
        m_iReadIdx(0),
        m_iWriteIdx(0),
        m_ptBuff(0)
    {
        assert(iBufferLengthInSamples > 0);

        m_ptBuff = new T[m_iBuffLength];
        reset();
    }

    virtual ~CRingBuffer()
    {
        delete[] m_ptBuff;
        m_ptBuff = 0;
    }

    /*! add a new value of type T to write index and increment write index
    \param tNewValue the new value
    \return void
    */
    void putPostInc(T tNewValue)
    {
        put(tNewValue);
        incIdx(m_iWriteIdx);
    }

    /*! add a new value of type T to write index
    \param tNewValue the new value
    \return void
    */
    void put(T tNewValue)
    {
        m_ptBuff[m_iWriteIdx] = tNewValue;
    }

    /*! return the value at the current read index and increment the read pointer
    \return float the value from the read index
    */
    T getPostInc()
    {
        T tValue = get();
        incIdx(m_iReadIdx);
        return tValue;
    }

    /*! return the value at the current read index
    \param fOffset: read at offset from read index
    \return float the value from the read index
    */
    T get(float fOffset = 0) const
    {
        float fRawIndex = m_iReadIdx + fOffset;
        int iIndex0 = static_cast<int>(floor(fRawIndex));
        int iIndex1 = static_cast<int>(ceil(fRawIndex));
        float fValue0 = m_ptBuff[wrapAround(iIndex0)];
        float fValue1 = m_ptBuff[wrapAround(iIndex1)];

        float fInterpValue = fValue0 + ((fRawIndex - iIndex0) * (fValue1 - fValue0));

        return fInterpValue;
    }

    //Noel RingBuffFractional
/*\
    T get(float fOffset = 0) const
    {
        if (fOffset)
            return m_ptBuff[m_iReadIdx];
        else
        {
            // Get Fractional Part
            int IntOffset = static_cast<int>(floor(fOffset));
            float fracOffset = fOffset-IntOffset;

            int new_Read = incIdx(m_iReadIdx+IntOffset,0);
//          Check Negative Values and return
            new_Read = AllPos(new_Read);
            float cur_BuffVal = m_ptBuff[new_Read];
            float nex_BuffVal = m_ptBuff[incIdx(new_Read+1,0)];
            //Weighted sum
            return (1-fracOffset)*cur_BuffVal + fracOffset*(nex_BuffVal);
        }
    }
*/

    /*! set buffer content and indices to 0
    \return void
    */
    void reset()
    {
        std::memset (m_ptBuff, 0, sizeof(T)*m_iBuffLength);
        m_iReadIdx  = 0;
        m_iWriteIdx = 0;
    }

    /*! return the current index for writing/put
    \return int
    */
    int getWriteIdx() const
    {
        return m_iWriteIdx;
    }

    /*! move the write index to a new position
    \param iNewWriteIdx: new position
    \return void
    */
    void setWriteIdx(int iNewWriteIdx)
    {
        incIdx(m_iWriteIdx, iNewWriteIdx - m_iWriteIdx);
    }

    /*! return the current index for reading/get
    \return int
    */
    int getReadIdx() const
    {
        return m_iReadIdx;
    }

    /*! move the read index to a new position
    \param iNewReadIdx: new position
    \return void
    */
    void setReadIdx(int iNewReadIdx)
    {
        incIdx(m_iReadIdx, iNewReadIdx - m_iReadIdx);
    }

    /*! returns the number of values currently buffered (note: 0 could also mean the buffer is full!)
    \return int
    */
    int getNumValuesInBuffer() const
    {
        return (m_iWriteIdx - m_iReadIdx + m_iBuffLength) % m_iBuffLength;
    }

    /*! returns the length of the internal buffer
    \return int
    */
    int getLength() const
    {
        return m_iBuffLength;
    }
private:
    CRingBuffer();
    CRingBuffer(const CRingBuffer& that);

    void incIdx(int& iIdx, int iOffset = 1)
    {
        while ((iIdx + iOffset) < 0)
        {
            // avoid negative buffer indices
            iOffset += m_iBuffLength;
        }
        iIdx = (iIdx + iOffset) % m_iBuffLength;
    };

    int wrapAround(int iIdx) const
    {
        while (iIdx < 0)
            iIdx += m_iBuffLength;
        iIdx %= m_iBuffLength;
        return iIdx;
    }
    int AllPos(int ReadIdx)
    {
        if(ReadIdx<0) {
            return AllPos(incIdx(ReadIdx,m_iBuffLength));
        }
        else if(ReadIdx>m_iBuffLength-1) {
            return AllPos(incIdx(ReadIdx,-m_iBuffLength));
        }
        else {
            return incIdx(ReadIdx, 0);
        }
    }

    int m_iBuffLength = 0,      //!< length of the internal buffer
        m_iReadIdx = 0,         //!< current read index
        m_iWriteIdx = 0;        //!< current write index

    T* m_ptBuff = 0;            //!< data buffer
};
#endif // __RingBuffer_hdr__
