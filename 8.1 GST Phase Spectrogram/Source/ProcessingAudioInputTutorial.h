/*
  ==============================================================================

   This file is part of the JUCE tutorials.
   Copyright (c) 2017 - ROLI Ltd.

   The code included in this file is provided under the terms of the ISC license
   http://www.isc.org/downloads/software-support-policy/isc-license. Permission
   To use, copy, modify, and/or distribute this software for any purpose with or
   without fee is hereby granted provided that the above copyright notice and
   this permission notice appear in all copies.

   THE SOFTWARE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES,
   WHETHER EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR
   PURPOSE, ARE DISCLAIMED.

  ==============================================================================
*/

/*******************************************************************************
 The block below describes the properties of this PIP. A PIP is a short snippet
 of code that can be read by the Projucer and used to generate a JUCE project.

 BEGIN_JUCE_PIP_METADATA

 name:             ProcessingAudioInputTutorial
 version:          1.0.0
 vendor:           JUCE
 website:          http://juce.com
 description:      Performs processing on an input signal.

 dependencies:     juce_audio_basics, juce_audio_devices, juce_audio_formats,
                   juce_audio_processors, juce_audio_utils, juce_core,
                   juce_data_structures, juce_events, juce_graphics,
                   juce_gui_basics, juce_gui_extra
 exporters:        xcode_mac, vs2017, linux_make

 type:             Component
 mainClass:        MainContentComponent

 useLocalCopy:     1

 END_JUCE_PIP_METADATA

*******************************************************************************
 
 Jonathan Feldman, May 2019
 
 This version has the basical spectral drive algorithm working.
 
 The incoming guitar gain is very low
 
 The spectral components are being boosted but there is no distortion.
 
 */



#pragma once
#include <iostream>
#include <math.h>

#include "GST.h"
#include "CircularBuffer.hpp"

using namespace std;


//==============================================================================
class MainContentComponent   : public AudioAppComponent, private Timer
{
public:
    //==============================================================================
    MainContentComponent() : gst(44100, 110, 4096, 5, 12),
                             spectrogramFFT(fftOrder),
                             spectrogramImage (Image::RGB, 512, 512, true)
    {
        
        setSize (600, 100);
        
        //
        // change the audio buffer size
        //
        
        AudioDeviceManager::AudioDeviceSetup currentAudioSetup;
        deviceManager.getAudioDeviceSetup (currentAudioSetup);
        currentAudioSetup.bufferSize = 128;
        deviceManager.setAudioDeviceSetup (currentAudioSetup, true);
        
        // set for mono input, mono output
        //
        // the guitar is coming in the left channel and going out the headphones
        // which are connected to the left channel
        //
        // we will worry about stereo later
        //
        
        setAudioChannels (1, 1);
        
        //
        // set Spectrogram components
        //
        
        setOpaque (true);
        startTimerHz (60);
        setSize (700, 500);
        
        //
        // initialize the x buffer
        //
        
        xDelayN.init(4096);
        
        // ocqt.printFreqs();
        std::cout << "M = " << gst.getM() << "\n";
    }

    ~MainContentComponent()
    {
        shutdownAudio();
    }

    void prepareToPlay (int, double) override {}

    void getNextAudioBlock (const AudioSourceChannelInfo& bufferToFill) override
    {
        auto* device = deviceManager.getCurrentAudioDevice();
        auto activeInputChannels  = device->getActiveInputChannels();
        auto activeOutputChannels = device->getActiveOutputChannels();
        auto maxInputChannels  = activeInputChannels .getHighestBit() + 1;
        auto maxOutputChannels = activeOutputChannels.getHighestBit() + 1;
        
        for (auto channel = 0; channel < maxOutputChannels; ++channel)
        {
            if ((! activeOutputChannels[channel]) || maxInputChannels == 0)
            {
                bufferToFill.buffer->clear (channel, bufferToFill.startSample, bufferToFill.numSamples);
            }
            else
            {
                auto actualInputChannel = channel % maxInputChannels;
                
                if (! activeInputChannels[channel])
                {
                    bufferToFill.buffer->clear (channel, bufferToFill.startSample, bufferToFill.numSamples);
                }
                else //
                {
                    auto* inBuffer = bufferToFill.buffer->getReadPointer (actualInputChannel, bufferToFill.startSample);
                    
                    //
                    //  we send the samples to the fifo to reverse the order of the samples
                    //  for the spectrogram
                    //
                    
                     for (auto sample = 0; sample < bufferToFill.numSamples; ++sample)
                     {
                         float xin = inBuffer[sample];
                         
                         float xmN = xDelayN.get();
                         
                         f = gst.processSample(static_cast<double>(xin), static_cast<double>(xmN), t);
                         
                         drawNextLineOfSpectrogram();
                         
                         xDelayN.put(xin);
                         
                         t++;
                     }
                    
                    //
                    // Add effect here
                    //
                    
                    auto* outBuffer = bufferToFill.buffer->getWritePointer (channel, bufferToFill.startSample);
                                                                          
                    for (auto sample = 0; sample < bufferToFill.numSamples; ++sample)
                    {
                        outBuffer[sample] = outBuffer[sample];
                    }
                }
            }
        }
    }
    
    void releaseResources() override {}

    void resized() override
    {
        //
    }
    
    void paint (Graphics& g) override
    {
        g.fillAll (Colours::black);
        g.setOpacity (1.0f);
        g.drawImage (spectrogramImage, getLocalBounds().toFloat());
    }
    
    void timerCallback() override
    {
        repaint();
    }

    void drawNextLineOfSpectrogram()
    {
        auto rightHandEdge = spectrogramImage.getWidth() - 1;
        auto imageHeight   = spectrogramImage.getHeight();
        
        // first, shuffle our image leftwards by 1 pixel..
        spectrogramImage.moveImageSection (0, 0, 1, 0, rightHandEdge, imageHeight);

        int M = (int) gst.getM();
        
        for (int k = 0; k < M; k++)
        {
            phase[k] = atan2(imag(f[k]), real(f[k]));
        }
        
        // find the range of values produced, so we can scale our rendering to
        // show up the detail clearly
  
        auto maxLevel = FloatVectorOperations::findMinAndMax (magnitude, maxMagnitude);
        
        for (auto y = 1; y < imageHeight; ++y)
        {
            auto proportion = M - y / (float) imageHeight * (float) M;
            
            auto index = jlimit<int> (0, M, (int) (proportion));
            
            // std::cout << "index: " << index << "\n";
            
            auto level = jmap<double> (magnitude[index], 0.0f, jmax<double> (maxLevel.getEnd(), 1e-5f), 0.0f, 1.0f);
            
            spectrogramImage.setPixelAt (rightHandEdge, y, Colour::fromHSV (magnitude[index], 0.5f, level, 1.0f));
        }
    }

    enum
    {
        fftOrder = 9,
        fftSize = 1 << fftOrder
    };


private:

    //
    // OCQT
    //
    
    
    GST gst;
    long int t = 0;
    circular_buffer xDelayN;
    std::complex<double>* f;
    
    //
    // signal processing variables
    //
    
    Random random;

    dsp::FFT spectrogramFFT;
    
    complex<float> queuedSampleBuffer[fftSize];
    float outputSampleBuffer[fftSize];
    
    //
    // this buffer size should be of length M
    // consider computing magnitude and phase inside OCQT
    //
    double phase[fftSize];
    
    //
    // Spectrogram Image
    //
    
    Image spectrogramImage;

    complex<float> fifo[fftSize];
    complex<float> fftData[2*fftSize];
    int fifoIndex = 0;
    bool nextFFTBlockReady = false;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainContentComponent)
};
