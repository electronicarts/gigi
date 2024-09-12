// The size of the image to FFT: a power of 2 between 256 and 2048
#define N 2048

#define PI 3.14159265359

// The radix to use for the FFT: 2 or 4
static const uint RADIX = 4;

static const uint WORKGROUP_SIZE = min(N / RADIX, 256);

static const uint LOG2_N = firstbithigh(uint(N));
static const uint LOG4_N = LOG2_N / 2;
