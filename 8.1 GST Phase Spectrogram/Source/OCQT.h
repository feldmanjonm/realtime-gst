//
//  OCQT.h
//  Test_OCQT
//
//  Created by Jon Feldman on 2019-05-29.
//  Copyright © 2019 Jonathan Feldman. All rights reserved.
//

#ifndef OCQT_h
#define OCQT_h

#include <math.h>
#include <complex>


class OCQT
{

public:
    
    OCQT(size_t fs_in, double B_in, size_t N_in, size_t nOct, size_t Div)
    {
        fs = fs_in;
        B = B_in;
        N = N_in;
        
        M = nOct * Div;

        f = new std::complex<double>[M];
        ft = new std::complex<double>[M];
        
        freqs = new double[M];
        theta = new double[M];
        dtheta = new double[M];
        phi = new double[M];
        
        xbuf = new double[N];
        
        //
        // initialize the variables
        //
        
        for (int k = 0; k < M; k++)
        {
            freqs[k] = B * pow(2, k/Div);
            theta[k] = 0;
            dtheta[k] = -2 * M_PI * freqs[k] / fs;
            phi[k] = 0;
        }
    }

    std::complex<double>* processSample(double x, double xmN, size_t t)
    {
        const std::complex<double> j(0, 1);
        
        t = t + 1;
        
        if (t == 1)
        {
            for (int k = 0; k < M; k++)
            {
                f[k] = x;
            }
        }
        else if (t >= 2 && t <= N)
        {
            for (int k = 0; k < M; k++)
            {
                theta[k] = theta[k] + dtheta[k];
                
                f[k] = f[k] + std::exp(j * theta[k]) * x;
            }
        }
        else
        {
            for (int k = 0; k < M; k++)
            {
                theta[k] = theta[k] + dtheta[k];
                
                std::complex<double> a = std::exp(j * theta[k]);
                
                phi[k] = theta[k] - (dtheta[k] * N);
                
                std::complex<double> b = std::exp(j * phi[k]);
                
                ft[k] = f[k] + a * x - b * xmN;
                
                f[k] = ft[k];
            }
        }
        
        return f;
    }
    
private:

    std::complex<double>* f;
    std::complex<double>* ft;
    
    double* freqs;
    double* theta;
    double* dtheta;
    double* phi;
    
    size_t fs;
    size_t M;
    size_t N;
    double B;
    
    double* xbuf;
};


#endif /* OCQT_h */
