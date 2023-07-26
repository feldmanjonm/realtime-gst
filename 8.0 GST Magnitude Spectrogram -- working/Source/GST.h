//
//  GST.h
//  Test_GST
//
//  Created by Jon Feldman on 2019-05-29.
//  Copyright © 2019 Jonathan Feldman. All rights reserved.
//

#ifndef GST_h
#define GST_h

#include <math.h>
#include <complex>


class GST
{

public:
    
    explicit GST(size_t fs_in, double B_in, size_t N_in, size_t nOct, size_t Div):
    fs(fs_in),B(B_in),N(N_in),M(nOct*Div)
    {
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
            freqs[k] = B * pow(2, (double)k / (double)Div);
            theta[k] = 0;
            dtheta[k] = -2 * M_PI * freqs[k] / fs;
            phi[k] = 0;
            f[k] = 0;
            ft[k] = 0;
        }
    }
    
    ~GST()
    {
        delete[] f;
        delete[] ft;
        
        delete[] freqs;
        delete[] theta;
        delete[] dtheta;
        delete[] phi;
        
        delete[] xbuf;
    }

    std::complex<double>* processSample(double x, double xmN, size_t t_)
    {        
        const std::complex<double> j(0, 1);
        
        size_t t = t_ + 1;
        
        if (t == 1)
        {
            for (int k = 0; k < M; k++)
            {
                f[k] = x;
            }
            
            xbuf[t-1] = x;
        }
        else if (t >= 2 && t <= N)
        {
            for (int k = 0; k < M; k++)
            {
                theta[k] = theta[k] + dtheta[k];
                
                f[k] = f[k] + std::exp(j * theta[k]) * x;
            }
            
            xbuf[t-1] = x;
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
    
    double* getFreqs()
    {
        return freqs;
    }
    
    size_t getM()
    {
        return M;
    }
    
    size_t getN()
    {
        return N;
    }
    
    void printFreqs()
    {
        for (int k = 0; k < M; k++)
        {
            std::cout << freqs[k] <<  " ";
        }
        
        std::cout << "\n";
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


#endif /* GST_h */
