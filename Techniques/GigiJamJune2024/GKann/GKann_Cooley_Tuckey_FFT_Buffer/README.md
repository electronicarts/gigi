# Cooley Tuckey FFT
This is an implementation of the Cooley Tuckey FFT algorithm with Radix-2 and Radix-4 in Compute Shaders. I converted it from this a [WebGPU implementation](https://compute.toys/view/1187) to Gigi HLSL nodes.

# Usage Samples
There are two samples in the 'Usages' folder. One is a bloom filter, the other is a collection of blur filters (Circular bokeh, Gaussian blur and Box blur).

| Technique | Input | Output |
| --------- | ----- | ------ |
| Bloom | ![Bloom Input](./ReadMe%20Imgs/Bloom%20Input.png) | ![Bloom Output](./ReadMe%20Imgs/Bloom%20Output.png) |
| Blur | ![Blur Input](./ReadMe%20Imgs/Blur%20Input.png) | ![Blur Output](./ReadMe%20Imgs/Blur%20Output.png) |

# Sample Images
The images provided are high dynamic range (.exr) files. Their sources are highlighted below:
1. From thunderpoly.gumroad.com
    1. [BloomKernel](https://thunderpoly.gumroad.com/l/bloomfree)
2. From Polyhaven
    1. [leadenhall_market_2k](https://polyhaven.com/a/leadenhall_market)
3. From [OpenEXR](https://openexr.com/en/latest/test_images/index.html)
    1. CandleGlass
    2. PrismsLenses
    3. StillLife