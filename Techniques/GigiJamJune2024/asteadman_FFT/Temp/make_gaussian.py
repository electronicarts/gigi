import numpy as np
from scipy import signal
import math

# Generate a standard 2d gaussian kernel
# credit: thomasaarholt - https://gist.github.com/thomasaarholt/267ec4fff40ca9dff1106490ea3b7567
def gaussian_kernel(n, std, normalised=False):
    '''
    Generates a n x n matrix with a centered gaussian 
    of standard deviation std centered on it. If normalised,
    its volume equals 1.'''
    gaussian1D = signal.windows.gaussian(n, std)
    gaussian2D = np.outer(gaussian1D, gaussian1D)
    if normalised:
        gaussian2D /= (2*np.pi*(std**2))
    return gaussian2D

G=gaussian_kernel(257,math.sqrt(257),True)
N=4096
FG=np.fft.fft2(G,(N,N))
FGR=np.real(FG)
FGI=np.imag(FG)
Q = np.hstack( (FGR.reshape(N*N,1), FGI.reshape(N*N,1)) )
np.savetxt('gauss_fft_4k.csv',Q,delimiter=',')
#print(Q.shape)
