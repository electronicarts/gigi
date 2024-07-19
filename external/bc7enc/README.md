bc7enc - Fast, single source file BC1-5 and BC7/BPTC GPU texture encoders.

Note: The latest version of this repo, supporting rate distortion optimization for BC1-7, is [here](https://github.com/richgel999/bc7enc_rdo). The BC7 encoder here does not support all the modes, and is not vectorized. See bc7e.ispc in the bc7enc_rdo repo, which is far better.

This is primarily a testbed for rgbcx.h, which is [one of the highest quality BC1 encoders available (if not the highest quality)](https://aras-p.info/blog/2020/12/08/Texture-Compression-in-2020/).

To compile: Use CMake, and run "cmake ." in the project's root directory to generate either a Visual Studio .SLN file or make files.

Features:
- BC1/3 encoder (in [rgbcx.h](https://github.com/richgel999/bc7enc/blob/master/rgbcx.h)) uses a new algorithm (which we've named "prioritized cluster fit") which is 3-4x faster than traditional cluster fit (as implemented in [libsquish](https://github.com/svn2github/libsquish) with SSE2) at the same or slightly higher average quality using scalar CPU instructions. This algorithm is suitable for GPU encoder implementations.

The BC1/BC3 encoder also implements [Castano's optimal endpoint rounding improvement](https://gist.github.com/castano/c92c7626f288f9e99e158520b14a61cf).

rgbcx's BC1 encoder is faster than both AMD Compressonator and libsquish at the same average quality.

- BC7 encoder (in bc7enc.c/.h) has perceptual colorspace metric support, and is very fast compared to ispc_texcomp (see below) for RGB textures. Important: The BC7 encoder included in this repo is still a work in progress. I took bc7enc16 and added more modes for better alpha support, but it needs more testing and development. My best BC7 encoder is BC7E, located here: https://github.com/BinomialLLC/bc7e

- Full decoders for BC1-5/7. BC7 decoder is in bc7decomp.cpp/.h, BC1-5 decoders in rgbcx.h.

This project is basically a demo of some of the techniques we use in Basis BC7,
which is Binomial's state of the art vectorized BC7 encoder. Basis BC7 is the
highest quality and fastest CPU BC7 encoder available (2-3x faster than
ispc_texcomp). It supports all modes and linear/perceptual colorspace metrics.
Licensees get full ISPC source code so they can customize the codec as needed.

bc7enc currently only supports modes 1 and 6 for RGB, and modes 1, 5, 6, and 7 for alpha. The plan is to add all the modes. See the [bc7enc16](https://github.com/richgel999/bc7enc16) project for the previous version (which only supports modes 1 and 6). Note this readme still refers to "bc7enc16", but bc7enc is the same encoder but with more alpha modes.

This codec supports a perceptual mode when encoding BC7, where it computes colorspace error in
weighted YCbCr space (like etc2comp), and it also supports weighted RGBA
metrics. It's particular strong in perceptual mode, beating the current state of
the art CPU encoder (Intel's ispc_texcomp) by a wide margin when measured by
Luma PSNR, even though it only supports 2 modes and isn't vectorized.

Why only modes 1 and 6 for opaque BC7?
Because with these two modes you have a complete encoder that supports both
opaque and transparent textures in a small amount (~1400 lines) of
understandable plain C code. Mode 6 excels on smooth blocks, and mode 1 is
strong with complex blocks, and a strong encoder that combines both modes can be
quite high quality. Fast mode 6-only encoders will have noticeable block
artifacts which this codec avoids by fully supporting mode 1.

Modes 1 and 6 are typically the most used modes on many textures using other
encoders. Mode 1 has two subsets, 64 possible partitions, and 3-bit indices,
while mode 6 has large 4-bit indices and high precision 7777.1 endpoints. This
codec produces output that is far higher quality than any BC1 encoder, and
approaches (or in perceptual mode exceeds!) the quality of other full BC7
encoders.

Why is bc7enc16 so fast in perceptual mode?
Computing error in YCbCr space is more expensive than in RGB space, yet bc7enc16
in perceptual mode is stronger than ispc_texcomp (see the benchmark below) -
even without SSE/AVX vectorization and with only 2 modes to work with!

Most BC7 encoders only support linear RGB colorspace metrics, which is a
fundamental weakness. Some support weighted RGB metrics, which is better. With
linear RGB metrics, encoding error is roughly balanced between each channel, and
encoders have to work *very* hard (examining large amounts of RGB search space)
to get overall quality up. With perceptual colorspace metrics, RGB error tends
to become a bit unbalanced, with green quality favored more highly than red and
blue, and blue quality favored the least. A perceptual encoder is tuned to
prefer exploring solutions along the luma axis, where it's much less work to find
solutions with less luma error. bc7enc16 is, as far as I know, the first BC7
codec to support computing error in weighted YCbCr colorspace.

Note: Most of the timings here (except for the ispc_texcomp "fast" mode timings at the very bottom)
are for the *original* release, before I added several more optimizations. The latest version of 
bc7enc16.c is around 8-27% faster than the initial release at same quality (when mode 1 is enabled - 
there's no change with just mode 6).

Some benchmarks across 31 images (kodim corpus+others):

Perceptual (average REC709 Luma PSNR - higher is better quality):
```
iscp_texcomp slow vs. bc7enc16 uber4/max_partitions 64
iscp_texcomp:   355.4 secs 48.6 dB
bc7enc16:       122.6 secs 50.0 dB

iscp_texcomp slow vs. bc7enc16 uber0/max_partitions 64
iscp_texcomp:   355.4 secs 48.6 dB
bc7enc16:       38.3 secs 49.6 dB

iscp_texcomp basic vs. bc7enc16 uber0/max_partitions 16
ispc_texcomp:   100.2 secs 48.3 dB
bc7enc16:       20.8 secs 49.3 dB 

iscp_texcomp fast vs. bc7enc16 uber0/max_partitions 16
iscp_texcomp:   41.5 secs 48.0 dB 
bc7enc16:       20.8 secs 49.3 dB

iscp_texcomp ultrafast vs. bc7enc16 uber0/max_partitions 0
iscp_texcomp:   1.9 secs 46.2 dB
bc7enc16:       8.9 secs 48.4 dB 

Non-perceptual (average RGB PSNR):

iscp_texcomp slow vs. bc7enc16 uber4/max_partitions 64
iscp_texcomp:   355.4 secs 46.8 dB 
bc7enc16:       51 secs 46.1 dB

iscp_texcomp slow vs. bc7enc16 uber0/max_partitions 64
iscp_texcomp:   355.4 secs 46.8 dB
bc7enc16:       29.3 secs 45.8 dB

iscp_texcomp basic vs. bc7enc16 uber4/max_partitions 64
iscp_texcomp:   99.9 secs 46.5 dB
bc7enc16:       51 secs 46.1 dB

iscp_texcomp fast vs. bc7enc16 uber1/max_partitions 16
ispc_texcomp:   41.5 secs 46.1 dB
bc7enc16:       19.8 secs 45.5 dB

iscp_texcomp fast vs. bc7enc16 uber0/max_partitions 8
ispc_texcomp:   41.5 secs 46.1 dB
bc7enc16:       10.46 secs 44.4 dB

iscp_texcomp ultrafast vs. bc7enc16 uber0/max_partitions 0
ispc_texcomp:   1.9 secs 42.7 dB 
bc7enc16:       3.8 secs 42.7 dB

DirectXTex CPU in "mode 6 only" mode vs. bc7enc16 uber1/max_partions 0 (mode 6 only), non-perceptual:

DirectXTex:     466.4 secs 41.9 dB 
bc7enc16:       6.7 secs 42.8 dB

DirectXTex CPU in (default - no 3 subset modes) vs. bc7enc16 uber1/max_partions 64, non-perceptual:

DirectXTex:     9485.1 secs 45.6 dB 
bc7enc16:       36 secs 46.0 dB
```
(Note this version of DirectXTex has a key pbit bugfix which I've submitted but
is still waiting to be accepted. Non-bugfixed versions will be slightly lower
quality.)

UPDATE: To illustrate how strong the mode 1+6 implementation is in bc7enc16, let's compare ispc_texcomp 
fast vs. the latest version of bc7enc16 uber4/max_partitions 64:

Without filterbank optimizations:
```
                Time       RGB PSNR   Y PSNR
ispc_texcomp:   41.45 secs 46.09 dB   48.0 dB
bc7enc16:       41.42 secs 46.03 dB   48.2 dB

With filterbank optimizations enabled:
bc7enc16:       38.78 secs 45.94 dB   48.12 dB
```
They both have virtually the same average RGB PSNR with these settings (.06 dB is basically noise), but 
bc7enc16 is just as fast as ispc_texcomp fast, even though it's not vectorized. Interestingly, our Y PSNR is better, 
although bc7enc16 wasn't using perceptual metrics in these benchmarks. 

This was a multithreaded benchmark (using OpenMP) on a dual Xeon workstation.
ispc_texcomp was called with 64-blocks at a time and used AVX instructions.
Timings are for encoding only.
