// test.cpp - Command line example/test app
#ifdef _MSC_VER
#define _CRT_SECURE_NO_WARNINGS
#endif

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <algorithm>
#include <assert.h>
#include <time.h>

#include "bc7enc.h"
#include "lodepng.h"
#include "dds_defs.h"
#include "bc7decomp.h"

#define RGBCX_IMPLEMENTATION
#include "rgbcx.h"

const int MAX_UBER_LEVEL = 5;

inline int iabs(int i) { if (i < 0) i = -i; return i; }
inline uint8_t clamp255(int32_t i) { return (uint8_t)((i & 0xFFFFFF00U) ? (~(i >> 31)) : i); }
template <typename S> inline S clamp(S value, S low, S high) { return (value < low) ? low : ((value > high) ? high : value); }

static int print_usage()
{
	fprintf(stderr, "bc7enc\n");
	fprintf(stderr, "Reads PNG files (with or without alpha channels) and packs them to BC1-5 or BC7/BPTC (default) using\nmodes 1, 6 (opaque blocks) or modes 1, 5, 6, and 7 (alpha blocks).\n");
	fprintf(stderr, "By default, a DX10 DDS file and a unpacked PNG file will be written to the current\ndirectory with the .dds/_unpacked.png/_unpacked_alpha.png suffixes.\n\n");
	fprintf(stderr, "Usage: bc7enc [-apng_filename] [options] input_filename.png [compressed_output.dds] [unpacked_output.png]\n\n");
	fprintf(stderr, "-apng_filename Load G channel of PNG file into alpha channel of source image\n");
	fprintf(stderr, "-g Don't write unpacked output PNG files (this disables PSNR metrics too).\n");
	fprintf(stderr, "-y Flip source image along Y axis before packing\n");
	fprintf(stderr, "-o Write output files to the current directory\n");
	fprintf(stderr, "-1 Encode to BC1. -u[0,5] controls quality vs. perf. tradeoff for RGB.\n");
	fprintf(stderr, "-3 Encode to BC3. -u[0,5] controls quality vs. perf. tradeoff for RGB.\n");
	fprintf(stderr, "-4 Encode to BC4\n");
	fprintf(stderr, "-5 Encode to BC5\n");
	fprintf(stderr, "\n");
	fprintf(stderr, "-X# BC4/5: Set first color channel (defaults to 0 or red)\n");
	fprintf(stderr, "-Y# BC4/5: Set second color channel (defaults to 1 or green)\n");
	fprintf(stderr, "\n");
	fprintf(stderr, "-l BC7: Use linear colorspace metrics instead of perceptual\n");
	fprintf(stderr, "-uX BC1/3/7: Higher quality levels, X ranges from [0,4] for BC7, or [0,5] for BC1-3\n");
	fprintf(stderr, "-pX BC7: Scan X partitions in mode 1, X ranges from [0,64], use 0 to disable mode 1 entirely (faster)\n");
	fprintf(stderr, "\n");
	fprintf(stderr, "-b BC1: Enable 3-color mode for blocks containing black or very dark pixels. (Important: engine/shader MUST ignore decoded texture alpha if this flag is enabled!)\n");
	fprintf(stderr, "-c BC1: Disable 3-color mode for solid color blocks\n");
	fprintf(stderr, "-n BC1: Encode/decode for NVidia GPU's\n");
	fprintf(stderr, "-m BC1: Encode/decode for AMD GPU's\n");
	fprintf(stderr, "-r BC1: Encode/decode using ideal BC1 formulas with rounding for 4-color block colors 2,3 (same as AMD Compressonator)\n");
	fprintf(stderr, "-LX BC1: Set encoding level, where 0=fastest and 19=slowest but highest quality\n");
	fprintf(stderr, "-f Force writing DX10-style DDS files (otherwise for BC1-5 it uses DX9-style DDS files)\n");
	fprintf(stderr, "\nBy default, this tool encodes to BC1 without rounding 4-color block colors 2,3, which may not match the output of some software decoders.\n");
	fprintf(stderr, "\nFor BC4 and BC5: Not all tools support reading DX9-style BC4/BC5 format files (or BC4/5 files at all). AMD Compressonator does.\n");
			
	return EXIT_FAILURE;
}

struct color_quad_u8
{
	uint8_t m_c[4];
	
	inline color_quad_u8(uint8_t r, uint8_t g, uint8_t b, uint8_t a)
	{
		set(r, g, b, a);
	}

	inline color_quad_u8(uint8_t y = 0, uint8_t a = 255)
	{
		set(y, a);
	}

	inline color_quad_u8 &set(uint8_t y, uint8_t a = 255)
	{
		m_c[0] = y;
		m_c[1] = y;
		m_c[2] = y;
		m_c[3] = a;
		return *this;
	}
	
	inline color_quad_u8 &set(uint8_t r, uint8_t g, uint8_t b, uint8_t a)
	{
		m_c[0] = r;
		m_c[1] = g;
		m_c[2] = b;
		m_c[3] = a;
		return *this;
	}

	inline uint8_t &operator[] (uint32_t i) { assert(i < 4);  return m_c[i]; }
	inline uint8_t operator[] (uint32_t i) const { assert(i < 4); return m_c[i]; }

	inline int get_luma() const { return (13938U * m_c[0] + 46869U * m_c[1] + 4729U * m_c[2] + 32768U) >> 16U; } // REC709 weightings
};
typedef std::vector<color_quad_u8> color_quad_u8_vec;

class image_u8
{
public:
	image_u8() : 
		m_width(0), m_height(0)
	{
	}

	image_u8(uint32_t width, uint32_t height) :
		m_width(width), m_height(height)
	{
		m_pixels.resize(width * height);
	}

	inline const color_quad_u8_vec &get_pixels() const { return m_pixels; }
	inline color_quad_u8_vec &get_pixels() { return m_pixels; }

	inline uint32_t width() const { return m_width; }
	inline uint32_t height() const { return m_height; }
	inline uint32_t total_pixels() const { return m_width * m_height; }

	inline color_quad_u8 &operator()(uint32_t x, uint32_t y) { assert(x < m_width && y < m_height);  return m_pixels[x + m_width * y]; }
	inline const color_quad_u8 &operator()(uint32_t x, uint32_t y) const { assert(x < m_width && y < m_height);  return m_pixels[x + m_width * y]; }

	image_u8& clear()
	{
		m_width = m_height = 0;
		m_pixels.clear();
		return *this;
	}

	image_u8& init(uint32_t width, uint32_t height)
	{
		clear();

		m_width = width;
		m_height = height;
		m_pixels.resize(width * height);
		return *this;
	}

	image_u8& set_all(const color_quad_u8 &p)
	{
		for (uint32_t i = 0; i < m_pixels.size(); i++)
			m_pixels[i] = p;
		return *this;
	}

	image_u8& crop(uint32_t new_width, uint32_t new_height)
	{
		if ((m_width == new_width) && (m_height == new_height))
			return *this;

		image_u8 new_image(new_width, new_height);

		const uint32_t w = std::min(m_width, new_width);
		const uint32_t h = std::min(m_height, new_height);

		for (uint32_t y = 0; y < h; y++)
			for (uint32_t x = 0; x < w; x++)
				new_image(x, y) = (*this)(x, y);

		return swap(new_image);
	}

	image_u8 &swap(image_u8 &other)
	{
		std::swap(m_width, other.m_width);
		std::swap(m_height, other.m_height);
		std::swap(m_pixels, other.m_pixels);
		return *this;
	}

	inline void get_block(uint32_t bx, uint32_t by, uint32_t width, uint32_t height, color_quad_u8 *pPixels)
	{
		assert((bx * width + width) <= m_width);
		assert((by * height + height) <= m_height);

		for (uint32_t y = 0; y < height; y++)
			memcpy(pPixels + y * width, &(*this)(bx * width, by * height + y), width * sizeof(color_quad_u8));
	}

	inline void set_block(uint32_t bx, uint32_t by, uint32_t width, uint32_t height, const color_quad_u8 *pPixels)
	{
		assert((bx * width + width) <= m_width);
		assert((by * height + height) <= m_height);

		for (uint32_t y = 0; y < height; y++)
			memcpy(&(*this)(bx * width, by * height + y), pPixels + y * width, width * sizeof(color_quad_u8));
	}

	image_u8 &swizzle(uint32_t r, uint32_t g, uint32_t b, uint32_t a)
	{
		assert((r | g | b | a) <= 3);
		for (uint32_t y = 0; y < m_height; y++)
		{
			for (uint32_t x = 0; x < m_width; x++)
			{
				color_quad_u8 tmp((*this)(x, y));
				(*this)(x, y).set(tmp[r], tmp[g], tmp[b], tmp[a]);
			}
		}

		return *this;
	}
		
private:
	color_quad_u8_vec m_pixels;
	uint32_t m_width, m_height;
};

static bool load_png(const char *pFilename, image_u8 &img)
{
	img.clear();

	std::vector<unsigned char> pixels;
	unsigned int w = 0, h = 0;
	unsigned int e = lodepng::decode(pixels, w, h, pFilename);
	if (e != 0)
	{
		fprintf(stderr, "Failed loading PNG file %s\n", pFilename);
		return false;
	}

	img.init(w, h);
	memcpy(&img.get_pixels()[0], &pixels[0], w * h * sizeof(uint32_t));
	
	return true;
}

static bool save_png(const char *pFilename, const image_u8 &img, bool save_alpha)
{
	const uint32_t w = img.width();
	const uint32_t h = img.height();

	std::vector<unsigned char> pixels;
	if (save_alpha)
	{
		pixels.resize(w * h * sizeof(color_quad_u8));
		memcpy(&pixels[0], &img.get_pixels()[0], w * h * sizeof(color_quad_u8));
	}
	else
	{
		pixels.resize(w * h * 3);
		unsigned char *pDst = &pixels[0];
		for (uint32_t y = 0; y < h; y++)
			for (uint32_t x = 0; x < w; x++, pDst += 3)
				pDst[0] = img(x, y)[0], pDst[1] = img(x, y)[1], pDst[2] = img(x, y)[2];
	}
	
	return lodepng::encode(pFilename, pixels, w, h, save_alpha ? LCT_RGBA : LCT_RGB) == 0;
}

class image_metrics
{
public:
	double m_max, m_mean, m_mean_squared, m_root_mean_squared, m_peak_snr;

	image_metrics()
	{
		clear();
	}

	void clear()
	{
		memset(this, 0, sizeof(*this));
	}

	void compute(const image_u8 &a, const image_u8 &b, uint32_t first_channel, uint32_t num_channels)
	{
		const bool average_component_error = true;

		const uint32_t width = std::min(a.width(), b.width());
		const uint32_t height = std::min(a.height(), b.height());

		assert((first_channel < 4U) && (first_channel + num_channels <= 4U));

		// Histogram approach originally due to Charles Bloom.
		double hist[256];
		memset(hist, 0, sizeof(hist));

		for (uint32_t y = 0; y < height; y++)
		{
			for (uint32_t x = 0; x < width; x++)
			{
				const color_quad_u8 &ca = a(x, y);
				const color_quad_u8 &cb = b(x, y);

				if (!num_channels)
					hist[iabs(ca.get_luma() - cb.get_luma())]++;
				else
				{
					for (uint32_t c = 0; c < num_channels; c++)
						hist[iabs(ca[first_channel + c] - cb[first_channel + c])]++;
				}
			}
		}

		m_max = 0;
		double sum = 0.0f, sum2 = 0.0f;
		for (uint32_t i = 0; i < 256; i++)
		{
			if (!hist[i])
				continue;

			m_max = std::max<double>(m_max, i);

			double x = i * hist[i];

			sum += x;
			sum2 += i * x;
		}

		// See http://richg42.blogspot.com/2016/09/how-to-compute-psnr-from-old-berkeley.html
		double total_values = width * height;

		if (average_component_error)
			total_values *= clamp<uint32_t>(num_channels, 1, 4);

		m_mean = clamp<double>(sum / total_values, 0.0f, 255.0f);
		m_mean_squared = clamp<double>(sum2 / total_values, 0.0f, 255.0f * 255.0f);

		m_root_mean_squared = sqrt(m_mean_squared);

		if (!m_root_mean_squared)
			m_peak_snr = 100.0f;
		else
			m_peak_snr = clamp<double>(log10(255.0f / m_root_mean_squared) * 20.0f, 0.0f, 100.0f);
	}
};

struct block8
{
	uint64_t m_vals[1];
};

typedef std::vector<block8> block8_vec;

struct block16
{
	uint64_t m_vals[2];
};

typedef std::vector<block16> block16_vec;

static bool save_dds(const char *pFilename, uint32_t width, uint32_t height, const void *pBlocks, uint32_t pixel_format_bpp, DXGI_FORMAT dxgi_format, bool srgb, bool force_dx10_header)
{
	(void)srgb;

	FILE *pFile = NULL;
	pFile = fopen(pFilename, "wb");
	if (!pFile)
	{
		fprintf(stderr, "Failed creating file %s!\n", pFilename);
		return false;
	}

	fwrite("DDS ", 4, 1, pFile);

	DDSURFACEDESC2 desc;
	memset(&desc, 0, sizeof(desc));

	desc.dwSize = sizeof(desc);
	desc.dwFlags = DDSD_WIDTH | DDSD_HEIGHT | DDSD_PIXELFORMAT | DDSD_CAPS;

	desc.dwWidth = width;
	desc.dwHeight = height;

	desc.ddsCaps.dwCaps = DDSCAPS_TEXTURE;
	desc.ddpfPixelFormat.dwSize = sizeof(desc.ddpfPixelFormat);
				
	desc.ddpfPixelFormat.dwFlags |= DDPF_FOURCC;

	desc.lPitch = (((desc.dwWidth + 3) & ~3) * ((desc.dwHeight + 3) & ~3) * pixel_format_bpp) >> 3;
	desc.dwFlags |= DDSD_LINEARSIZE;
	
	desc.ddpfPixelFormat.dwRGBBitCount = 0;

	if ( (!force_dx10_header) &&
		 ((dxgi_format == DXGI_FORMAT_BC1_UNORM) ||
		 (dxgi_format == DXGI_FORMAT_BC3_UNORM) ||
		 (dxgi_format == DXGI_FORMAT_BC4_UNORM) ||
		 (dxgi_format == DXGI_FORMAT_BC5_UNORM)) )
	{
		if (dxgi_format == DXGI_FORMAT_BC1_UNORM)
			desc.ddpfPixelFormat.dwFourCC = (uint32_t)PIXEL_FMT_FOURCC('D', 'X', 'T', '1');
		else if (dxgi_format == DXGI_FORMAT_BC3_UNORM)
			desc.ddpfPixelFormat.dwFourCC = (uint32_t)PIXEL_FMT_FOURCC('D', 'X', 'T', '5');
		else if (dxgi_format == DXGI_FORMAT_BC4_UNORM)
			desc.ddpfPixelFormat.dwFourCC = (uint32_t)PIXEL_FMT_FOURCC('A', 'T', 'I', '1');
		else if (dxgi_format == DXGI_FORMAT_BC5_UNORM)
			desc.ddpfPixelFormat.dwFourCC = (uint32_t)PIXEL_FMT_FOURCC('A', 'T', 'I', '2');

		fwrite(&desc, sizeof(desc), 1, pFile);
	}
	else
	{
		desc.ddpfPixelFormat.dwFourCC = (uint32_t)PIXEL_FMT_FOURCC('D', 'X', '1', '0');
						
		fwrite(&desc, sizeof(desc), 1, pFile);

		DDS_HEADER_DXT10 hdr10;
		memset(&hdr10, 0, sizeof(hdr10));

		// Not all tools support DXGI_FORMAT_BC7_UNORM_SRGB (like NVTT), but ddsview in DirectXTex pays attention to it. So not sure what to do here.
		// For best compatibility just write DXGI_FORMAT_BC7_UNORM.
		//hdr10.dxgiFormat = srgb ? DXGI_FORMAT_BC7_UNORM_SRGB : DXGI_FORMAT_BC7_UNORM;
		hdr10.dxgiFormat = dxgi_format; // DXGI_FORMAT_BC7_UNORM;
		hdr10.resourceDimension = D3D10_RESOURCE_DIMENSION_TEXTURE2D;
		hdr10.arraySize = 1;

		fwrite(&hdr10, sizeof(hdr10), 1, pFile);
	}

	fwrite(pBlocks, desc.lPitch, 1, pFile);

	if (fclose(pFile) == EOF)
	{
		fprintf(stderr, "Failed writing to DDS file %s!\n", pFilename);
		return false;
	}

	return true;
}

static void strip_extension(std::string &s)
{
	for (int32_t i = (int32_t)s.size() - 1; i >= 0; i--)
	{
		if (s[i] == '.')
		{
			s.resize(i);
			break;
		}
	}
}

static void strip_path(std::string& s)
{
	for (int32_t i = (int32_t)s.size() - 1; i >= 0; i--)
	{
		if ((s[i] == '/') || (s[i] == ':') || (s[i] == '\\'))
		{
			s.erase(0, i + 1);
			break;
		}
	}
}

int main(int argc, char *argv[])
{
	if (argc < 2)
		return print_usage();

	std::string src_filename;
	std::string src_alpha_filename;
	std::string dds_output_filename;
	std::string png_output_filename;
	std::string png_alpha_output_filename;

	bool no_output_png = false;
	bool out_cur_dir = false;

	int uber_level = 0;
	int max_partitions_to_scan = BC7ENC_MAX_PARTITIONS1;
	bool perceptual = true;
	bool y_flip = false;
	uint32_t bc45_channel0 = 0;
	uint32_t bc45_channel1 = 1;
	
	rgbcx::bc1_approx_mode bc1_mode = rgbcx::bc1_approx_mode::cBC1Ideal;
	bool use_bc1_3color_mode = true;
	bool use_bc1_3color_mode_for_black = false;
	int bc1_quality_level = 2;

	DXGI_FORMAT dxgi_format = DXGI_FORMAT_BC7_UNORM;
	uint32_t pixel_format_bpp = 8;
	bool force_dx10_dds = false;
	
	for (int i = 1; i < argc; i++)
	{
		const char *pArg = argv[i];
		if (pArg[0] == '-')
		{
			switch (pArg[1])
			{
				case '1':
				{
					dxgi_format = DXGI_FORMAT_BC1_UNORM;
					pixel_format_bpp = 4;
					printf("Compressing to BC1\n");
					break;
				}
				case '3':
				{
					dxgi_format = DXGI_FORMAT_BC3_UNORM;
					pixel_format_bpp = 8;
					printf("Compressing to BC3\n");
					break;
				}
				case '4':
				{
					dxgi_format = DXGI_FORMAT_BC4_UNORM;
					pixel_format_bpp = 4;
					printf("Compressing to BC4\n");
					break;
				}
				case '5':
				{
					dxgi_format = DXGI_FORMAT_BC5_UNORM;
					pixel_format_bpp = 8;
					printf("Compressing to BC5\n");
					break;
				}
				case 'y':
				{
					y_flip = true;
					break;
				}
				case 'a':
				{
					src_alpha_filename = pArg + 2;
					break;
				}
				case 'X':
				{
					bc45_channel0 = atoi(pArg + 2);
					if ((bc45_channel0 < 0) || (bc45_channel0 > 3))
					{
						fprintf(stderr, "Invalid argument: %s\n", pArg);
						return EXIT_FAILURE;
					}
					break;
				}
				case 'Y':
				{
					bc45_channel1 = atoi(pArg + 2);
					if ((bc45_channel1 < 0) || (bc45_channel1 > 3))
					{
						fprintf(stderr, "Invalid argument: %s\n", pArg);
						return EXIT_FAILURE;
					}
					break;
				}
				case 'f':
				{
					force_dx10_dds = true;
					break;
				}
				case 'u':
				{
					uber_level = atoi(pArg + 2);
					if ((uber_level < 0) || (uber_level > MAX_UBER_LEVEL))
					{
						fprintf(stderr, "Invalid argument: %s\n", pArg);
						return EXIT_FAILURE;
					}
					break;

				}
				case 'L':
				{
					bc1_quality_level = atoi(pArg + 2);
					if (((int)bc1_quality_level < (int)rgbcx::MIN_LEVEL) || ((int)bc1_quality_level > (int)(rgbcx::MAX_LEVEL + 1)))
					{
						fprintf(stderr, "Invalid argument: %s\n", pArg);
						return EXIT_FAILURE;
					}
					break;

				}
				case 'g':
				{
					no_output_png = true;
					break;
				}
				case 'l':
				{
					perceptual = false;
					break;
				}
				case 'p':
				{
					max_partitions_to_scan = atoi(pArg + 2);
					if ((max_partitions_to_scan < 0) || (max_partitions_to_scan > BC7ENC_MAX_PARTITIONS1))
					{
						fprintf(stderr, "Invalid argument: %s\n", pArg);
						return EXIT_FAILURE;
					}
					break;
				}
				case 'n':
				{
					bc1_mode = rgbcx::bc1_approx_mode::cBC1NVidia;
					break;
				}
				case 'm':
				{
					bc1_mode = rgbcx::bc1_approx_mode::cBC1AMD;
					break;
				}
				case 'r':
				{
					bc1_mode = rgbcx::bc1_approx_mode::cBC1IdealRound4;
					break;
				}
				case 'o':
				{
					out_cur_dir = true;
					break;
				}
				case 'b':
				{
					use_bc1_3color_mode_for_black = true;
					break;
				}
				case 'c':
				{
					use_bc1_3color_mode = false;
					break;
				}
				default:
				{
					fprintf(stderr, "Invalid argument: %s\n", pArg);
					return EXIT_FAILURE;
				}
			}
		}
		else
		{
			if (!src_filename.size())
				src_filename = pArg;
			else if (!dds_output_filename.size())
				dds_output_filename = pArg;
			else if (!png_output_filename.size())
				png_output_filename = pArg;
			else
			{
				fprintf(stderr, "Invalid argument: %s\n", pArg);
				return EXIT_FAILURE;
			}
		}
	}

	const uint32_t bytes_per_block = (16 * pixel_format_bpp) / 8;
	assert(bytes_per_block == 8 || bytes_per_block == 16);

	if (!src_filename.size())
	{
		fprintf(stderr, "No source filename specified!\n");
		return EXIT_FAILURE;
	}

	if (!dds_output_filename.size())
	{
		dds_output_filename = src_filename;
		strip_extension(dds_output_filename);
		if (out_cur_dir)
			strip_path(dds_output_filename);
		dds_output_filename += ".dds";
	}

	if (!png_output_filename.size())
	{
		png_output_filename = src_filename;
		strip_extension(png_output_filename);
		if (out_cur_dir)
			strip_path(png_output_filename);
		png_output_filename += "_unpacked.png";
	}

	png_alpha_output_filename = png_output_filename;
	strip_extension(png_alpha_output_filename);
	png_alpha_output_filename += "_alpha.png";
		
	image_u8 source_image;
	if (!load_png(src_filename.c_str(), source_image))
		return EXIT_FAILURE;

	printf("Source image: %s %ux%u\n", src_filename.c_str(), source_image.width(), source_image.height());

	if (src_alpha_filename.size())
	{
		image_u8 source_alpha_image;
		if (!load_png(src_alpha_filename.c_str(), source_alpha_image))
			return EXIT_FAILURE;

		printf("Source alpha image: %s %ux%u\n", src_alpha_filename.c_str(), source_alpha_image.width(), source_alpha_image.height());

		const uint32_t w = std::min(source_alpha_image.width(), source_image.width());
		const uint32_t h = std::min(source_alpha_image.height(), source_image.height());
		
		for (uint32_t y = 0; y < h; y++)
			for (uint32_t x = 0; x < w; x++)
				source_image(x, y)[3] = source_alpha_image(x, y)[1];
	}

#if 0
	// HACK HACK
	for (uint32_t y = 0; y < source_image.height(); y++)
		for (uint32_t x = 0; x < source_image.width(); x++)
			source_image(x, y)[3] = 254;
#endif

	const uint32_t orig_width = source_image.width();
	const uint32_t orig_height = source_image.height();

	if (y_flip)
	{
		image_u8 temp;
		temp.init(orig_width, orig_height);

		for (uint32_t y = 0; y < orig_height; y++)
			for (uint32_t x = 0; x < orig_width; x++)
				temp(x, (orig_height - 1) - y) = source_image(x, y);

		temp.swap(source_image);
	}

	source_image.crop((source_image.width() + 3) & ~3, (source_image.height() + 3) & ~3);
		
	const uint32_t blocks_x = source_image.width() / 4;
	const uint32_t blocks_y = source_image.height() / 4;

	block16_vec packed_image16(blocks_x * blocks_y);
	block8_vec packed_image8(blocks_x * blocks_y);

	bc7enc_compress_block_params pack_params;
	bc7enc_compress_block_params_init(&pack_params);
	if (!perceptual)
		bc7enc_compress_block_params_init_linear_weights(&pack_params);
	pack_params.m_max_partitions_mode = max_partitions_to_scan;
	pack_params.m_uber_level = std::min(BC7ENC_MAX_UBER_LEVEL, uber_level);

	if (dxgi_format == DXGI_FORMAT_BC7_UNORM)
	{
		printf("Max mode 1 partitions: %u, uber level: %u, perceptual: %u\n", pack_params.m_max_partitions_mode, pack_params.m_uber_level, perceptual);
	}
	else
	{
		printf("Level: %u, use 3-color mode: %u, use 3-color mode for black: %u, bc1_mode: %u\n", 
			bc1_quality_level, use_bc1_3color_mode, use_bc1_3color_mode_for_black, (int)bc1_mode);
	}

	bc7enc_compress_block_init();
	rgbcx::init(bc1_mode);

	bool has_alpha = false;

	clock_t start_t = clock();
	uint32_t bc7_mode_hist[8];
	memset(bc7_mode_hist, 0, sizeof(bc7_mode_hist));

	for (uint32_t by = 0; by < blocks_y; by++)
	{
		for (uint32_t bx = 0; bx < blocks_x; bx++)
		{
			color_quad_u8 pixels[16];

			source_image.get_block(bx, by, 4, 4, pixels);
			if (!has_alpha)
			{
				for (uint32_t i = 0; i < 16; i++)
				{
					if (pixels[i].m_c[3] < 255)
					{
						has_alpha = true;
						break;
					}
				}
			}
						
			switch (dxgi_format)
			{
			case DXGI_FORMAT_BC1_UNORM:
			{
				block8* pBlock = &packed_image8[bx + by * blocks_x];

				rgbcx::encode_bc1(bc1_quality_level, pBlock, &pixels[0].m_c[0], use_bc1_3color_mode, use_bc1_3color_mode_for_black);
				break;
			}
			case DXGI_FORMAT_BC3_UNORM:
			{
				block16* pBlock = &packed_image16[bx + by * blocks_x];

				rgbcx::encode_bc3(bc1_quality_level, pBlock, &pixels[0].m_c[0]);
				break;
			}
			case DXGI_FORMAT_BC4_UNORM:
			{
				block8* pBlock = &packed_image8[bx + by * blocks_x];

				rgbcx::encode_bc4(pBlock, &pixels[0].m_c[bc45_channel0], 4);
				break;
			}
			case DXGI_FORMAT_BC5_UNORM:
			{
				block16* pBlock = &packed_image16[bx + by * blocks_x];

				rgbcx::encode_bc5(pBlock, &pixels[0].m_c[0], bc45_channel0, bc45_channel1, 4);
				break;
			}
			case DXGI_FORMAT_BC7_UNORM:
			{
				block16* pBlock = &packed_image16[bx + by * blocks_x];
																
				bc7enc_compress_block(pBlock, pixels, &pack_params);

				uint32_t mode = ((uint8_t *)pBlock)[0];
				for (uint32_t m = 0; m <= 7; m++)
				{
					if (mode & (1 << m)) 
					{
						bc7_mode_hist[m]++;
						break;
					}
				}
				break;
			}
			default:
			{
				assert(0);
				break;
			}
			}
		}

		if ((by & 127) == 0)
			printf(".");
	}
	
	clock_t end_t = clock();
	
	printf("\nTotal time: %f secs\n", (double)(end_t - start_t) / CLOCKS_PER_SEC);

	if (dxgi_format == DXGI_FORMAT_BC7_UNORM)
	{
		printf("BC7 mode histogram:\n");
		for (uint32_t i = 0; i < 8; i++)
			printf("%u: %u\n", i, bc7_mode_hist[i]);
	}
			
	if (has_alpha)
		printf("Source image had an alpha channel.\n");
			
	bool failed = false;
	if (!save_dds(dds_output_filename.c_str(), orig_width, orig_height, (bytes_per_block == 16) ? (void*)&packed_image16[0] : (void*)&packed_image8[0], pixel_format_bpp, dxgi_format, perceptual, force_dx10_dds))
		failed = true;
	else
		printf("Wrote DDS file %s\n", dds_output_filename.c_str());

	if ((!no_output_png) && (png_output_filename.size()))
	{
		image_u8 unpacked_image(source_image.width(), source_image.height());

		bool punchthrough_flag = false;
		for (uint32_t by = 0; by < blocks_y; by++)
		{
			for (uint32_t bx = 0; bx < blocks_x; bx++)
			{
				void* pBlock = (bytes_per_block == 16) ? (void *)&packed_image16[bx + by * blocks_x] : (void*)&packed_image8[bx + by * blocks_x];

				color_quad_u8 unpacked_pixels[16];
				for (uint32_t i = 0; i < 16; i++)
					unpacked_pixels[i].set(0, 0, 0, 255);

				switch (dxgi_format)
				{
				case DXGI_FORMAT_BC1_UNORM:
					rgbcx::unpack_bc1(pBlock, unpacked_pixels, true, bc1_mode);
					break;
				case DXGI_FORMAT_BC3_UNORM:
					if (!rgbcx::unpack_bc3(pBlock, unpacked_pixels, bc1_mode))
						punchthrough_flag = true;
					break;
				case DXGI_FORMAT_BC4_UNORM:
					rgbcx::unpack_bc4(pBlock, &unpacked_pixels[0][0], 4);
					break;
				case DXGI_FORMAT_BC5_UNORM:
					rgbcx::unpack_bc5(pBlock, &unpacked_pixels[0][0], 0, 1, 4);
					break;
				case DXGI_FORMAT_BC7_UNORM:
					bc7decomp::unpack_bc7((const uint8_t*)pBlock, (bc7decomp::color_rgba*)unpacked_pixels);
					break;
				default:
					assert(0);
					break;
				}

				unpacked_image.set_block(bx, by, 4, 4, unpacked_pixels);
			} // bx
		} // by

		if ((punchthrough_flag) && (dxgi_format == DXGI_FORMAT_BC3_UNORM))
			fprintf(stderr, "Warning: BC3 mode selected, but rgbcx::unpack_bc3() returned one or more blocks using 3-color mode!\n");

		if ((dxgi_format != DXGI_FORMAT_BC4_UNORM) && (dxgi_format != DXGI_FORMAT_BC5_UNORM))
		{
			image_metrics y_metrics;
			y_metrics.compute(source_image, unpacked_image, 0, 0);
			printf("Luma  Max error: %3.0f RMSE: %f PSNR %03.02f dB\n", y_metrics.m_max, y_metrics.m_root_mean_squared, y_metrics.m_peak_snr);

			image_metrics rgb_metrics;
			rgb_metrics.compute(source_image, unpacked_image, 0, 3);
			printf("RGB   Max error: %3.0f RMSE: %f PSNR %03.02f dB\n", rgb_metrics.m_max, rgb_metrics.m_root_mean_squared, rgb_metrics.m_peak_snr);

			image_metrics rgba_metrics;
			rgba_metrics.compute(source_image, unpacked_image, 0, 4);
			printf("RGBA  Max error: %3.0f RMSE: %f PSNR %03.02f dB\n", rgba_metrics.m_max, rgba_metrics.m_root_mean_squared, rgba_metrics.m_peak_snr);
		}
						
		for (uint32_t chan = 0; chan < 4; chan++)
		{
			if (dxgi_format == DXGI_FORMAT_BC4_UNORM)
			{
				if (chan != bc45_channel0)
					continue;
			}
			else if (dxgi_format == DXGI_FORMAT_BC5_UNORM)
			{
				if ((chan != bc45_channel0) && (chan != bc45_channel1))
					continue;
			}

			image_metrics c_metrics;
			c_metrics.compute(source_image, unpacked_image, chan, 1);
			static const char *s_chan_names[4] = { "Red  ", "Green", "Blue ", "Alpha" };
			printf("%s Max error: %3.0f RMSE: %f PSNR %03.02f dB\n", s_chan_names[chan], c_metrics.m_max, c_metrics.m_root_mean_squared, c_metrics.m_peak_snr);
		}

		if (bc1_mode != rgbcx::bc1_approx_mode::cBC1Ideal)
			printf("Note: BC1/BC3 RGB decoding was done with the specified vendor's BC1 approximations.\n");

		if (!save_png(png_output_filename.c_str(), unpacked_image, false))
			failed = true;
		else
			printf("Wrote PNG file %s\n", png_output_filename.c_str());
				
		if (png_alpha_output_filename.size())
		{
			image_u8 unpacked_image_alpha(unpacked_image);
			for (uint32_t y = 0; y < unpacked_image_alpha.height(); y++)
				for (uint32_t x = 0; x < unpacked_image_alpha.width(); x++)
					unpacked_image_alpha(x, y).set(unpacked_image_alpha(x, y)[3], 255);

			if (!save_png(png_alpha_output_filename.c_str(), unpacked_image_alpha, false))
				failed = true;
			else
				printf("Wrote PNG file %s\n", png_alpha_output_filename.c_str());
		}
	}
		
	return failed ? EXIT_FAILURE : EXIT_SUCCESS;
}
