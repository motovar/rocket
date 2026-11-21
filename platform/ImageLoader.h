#ifndef __PLATFORM_IMAGELOADER_H__
#define __PLATFORM_IMAGELOADER_H__

// Image loading for the port (replaces ImageLib::GetImage). Implements the SexyAppFramework rules:
//
//  * Extensionless names probe .tga, .jpg, .png, .gif in that order (SAF 1.3 order); the extensions of
//    formats a game registered (RegisterImageFormat, e.g. .jp2) follow, so a .png of the same name wins.
//  * Alpha masks: "_name" or "name_"; the mask's low (blue) byte becomes the alpha channel. A mask
//    without a colour image yields a white image with that alpha.
//  * HD variants: "name@2x.png" (or @3x/@4x) beside the image. The pixels are
//    kept at full resolution for the GPU texture while the image reports its logical (1x) size; a 1x
//    copy is produced for CPU-side code (hit tests, software blits, fonts).

#include <cstdint>
#include <functional>
#include <string>
#include <vector>

namespace Platform
{

struct LoadedImage
{
	int mWidth = 0;   // logical size
	int mHeight = 0;
	std::vector<uint32_t> mBits;  // ARGB (0xAARRGGBB), logical size

	int mHDScale = 1;  // > 1 when an HD variant was loaded
	int mHDWidth = 0;
	int mHDHeight = 0;
	std::vector<uint32_t> mHDBits;  // ARGB at mHDWidth x mHDHeight

	bool mHasAlpha = false;
	bool mGenerated = false;  // made by the ImageGenerator (SetImageGenerator), not read from a file
	std::string mSourcePath;  // real path of the colour (or mask) file
};

// theGamePath: "images/gemsheet6", "images\\titlescreen6.jpg", "../images/afont1" relative paths etc.
bool LoadGameImage(const std::string& theGamePath, LoadedImage& theOut, bool theLookForAlpha = true);

// Generated images (a game's code-drawn art): one function that may draw any game image at load time. LoadGameImage asks
// it before the game's files. theGamePath comes normalized and lower case, without its extension ("images/gemsheet6"). The function
// fills theOut as a decoded file would (mBits at the logical size, with their alpha; mHDBits and mHDScale for sharper
// pixels) and returns false to leave the image to the game's files. An empty function turns it off. Set it before
// loading starts: the loading thread calls it.
using ImageGenerator = std::function<bool(const std::string& theGamePath, LoadedImage& theOut)>;
void SetImageGenerator(ImageGenerator theGenerator);

// Decodes an image file from memory into ARGB. Returns false on failure. Registered formats
// (RegisterImageFormat) whose detector accepts the data are tried first, then JPEGs go through IJG libjpeg 6b
// (DecodeJPEG), then stb_image (PNG, GIF, BMP, TGA, ... and any JPEG libjpeg refuses). theHasAlpha is set when any
// pixel's alpha is not 255.
bool DecodeImage(const std::vector<uint8_t>& theData, int& theWidth, int& theHeight, std::vector<uint32_t>& theBits, bool* theHasAlpha);

// An image format besides the ones stb_image reads. A game that needs one (JPEG 2000: Platform::
// RegisterOpenJPEGImages in platform/openjpeg/JP2Decoder.h) registers it once before it loads images; games
// that register none never link that library, as with Mixer::RegisterModuleFormat.
//   theDetector: true when the file's bytes are this format (a signature check, not a full parse)
//   theDecoder:  decodes the file into theArgb (0xAARRGGBB, theWidth x theHeight pixels, row by row from the
//                top) and sets *theHasAlpha (when not null) to whether any pixel's alpha is not 255, exactly
//                as DecodeImage does for the built-in formats; false on failure
//   theExtension: ".jp2" (the dot is optional, case is ignored). It joins the extension search after the
//                built-in extensions, so an extensionless name finds a staged .png before a .jp2, and a name
//                that ends in it is taken to give its extension. Alpha masks ("name_.jp2" beside "name.jp2")
//                go through the same path as any other mask file. One detector and decoder pair registered
//                under several extensions is tried once per file.
typedef bool (*ImageDetector)(const std::vector<uint8_t>& theData);
typedef bool (*ImageDecoder)(const std::vector<uint8_t>& theData, int& theWidth, int& theHeight, std::vector<uint32_t>& theArgb,
	bool* theHasAlpha);
void RegisterImageFormat(const char* theExtension, ImageDetector theDetector, ImageDecoder theDecoder);

// ---- JPEG through IJG libjpeg 6b (platform/JPEGDecoder.cpp) ----
// The games' own exes decode JPEGs with libjpeg 6b (PopCap's ImageLib), with its default settings; stb_image
// rounds some pixels differently (by up to 3 levels on about 1% of them), so a port that must match the original's
// pixels decodes JPEGs here. DecodeImage and LoadGameImage already do. Game code that decodes image data itself
// (a host that gets bytes from a pak or a SWF) calls one of these instead of stbi_load_from_memory.

// True when theData starts with a JPEG's SOI marker (FF D8 FF).
bool IsJPEG(const uint8_t* theData, size_t theSize);

// Decodes a JPEG exactly as libjpeg 6b does with its defaults. Returns false for data that is not a JPEG, that
// libjpeg rejects (a cut-off file is not rejected: it decodes as far as it goes, as in the games) or that has
// four components (CMYK). theComponents is 3 (RGB, rows from the top) or 1 (grey).
bool DecodeJPEG(const uint8_t* theData, size_t theSize, int& theWidth, int& theHeight, int& theComponents,
	std::vector<uint8_t>& thePixels);

// stbi_load_from_memory with the same arguments and result (8-bit pixels from malloc, freed with stbi_image_free
// or free; theRequired 0 keeps the file's own component count), but a JPEG that libjpeg can decode goes through
// DecodeJPEG; every other image, and any JPEG it cannot, through stb_image. Components stb_image would convert
// (theRequired of 1, 2 or 4 from an RGB file, or a grey file asked for RGB) are converted the same way.
uint8_t* LoadImageFromMemory(const uint8_t* theData, int theSize, int* theWidth, int* theHeight, int* theComponents,
	int theRequired);

}

#endif
