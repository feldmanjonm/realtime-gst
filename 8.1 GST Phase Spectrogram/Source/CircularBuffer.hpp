//
//  CircularBuffer.hpp
//  ProcessingAudioInputTutorial - App
//
//  Created by Jon Feldman on 2019-05-29.
//  Copyright © 2019 JUCE. All rights reserved.
//

#ifndef CircularBuffer_hpp
#define CircularBuffer_hpp

#include <stdio.h>
#include <mutex>

class circular_buffer
{
public:
    
    circular_buffer()
    {
        initialized_ = false;
    }
    
    void init(size_t size)
    {
        if (!initialized_)
        {
            buf_ = new float[size];
        
            max_size_ = size;
        
            reset();
        
            initWithZeros();
        }
        
        initialized_ = true;
    }
    
    void initWithZeros()
    {
        for (int i = 0; i < max_size_; i++)
        {
            put(0);
        }
    }

    void reset()
    {
        std::lock_guard<std::mutex> lock(mutex_);
        
        head_ = tail_;
        
        full_ = false;
    }
        
    bool empty() const
    {
        // if head and tail are equal, we are empty
        
        return (!full_ && (head_ == tail_));
    }
    
    bool full() const
    {
        // if tail is ahead of the head by 1, we are full
        
        return full_;
    }

    size_t capacity() const
    {
        return max_size_;
    }

    size_t size() const
    {
        size_t size = max_size_;
        
        if (!full_)
        {
            if (head_ >= tail_)
            {
                size = head_ - tail_;
            }
            else
            {
                size = max_size_ + head_ - tail_;
            }
        }
        
        return size;
    }
    
    void put (float item)
    {
        std::lock_guard<std::mutex> lock(mutex_);
            
        buf_[head_] = item;
            
        if (full_)
        {
            tail_ = (tail_ + 1) % max_size_;
        }
            
        head_ = (head_ + 1) % max_size_;
            
        full_ = (head_ == tail_);
    }
        
    float get()
    {
        std::lock_guard<std::mutex> lock(mutex_);
        
        if (empty())
        {
            return 0.0;
        }
        
        // read data and advance the tail (we now have a free space)
        
        auto val = buf_[tail_];
        
        full_ = false;
        
        tail_ = (tail_ + 1) % max_size_;
        
        return val;
    }

    
private:
    
    bool initialized_;
    
    std::mutex mutex_;
    
    float* buf_;
    
    size_t head_ = 0;
    
    size_t tail_ = 0;
    
    size_t max_size_;
    
    bool full_ = 0;

};



#endif /* CircularBuffer_hpp */
