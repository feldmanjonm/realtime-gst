//
//  xbuf.h
//  ProcessingAudioInputTutorial - App
//
//  Created by Jon Feldman on 2019-05-30.
//  Copyright © 2019 JUCE. All rights reserved.
//

#ifndef xbuf_h
#define xbuf_h

class XBUF
{
    
public:
    
    explicit(size_t N_in):(N = N_in)
    {
        x = new float[N];
    }
    
    void addSample(float s, size_t t)
    {
        x[t] = s;
    }
    
private:
    
    float* x;
    
    size_t N;
}

#endif /* xbuf_h */
