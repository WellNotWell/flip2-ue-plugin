#pragma once

#include <cstdint>
#include <stdexcept>
#include <string>
#include <vector>

// Image — simple floating-point RGBA image container.
// Pixels are stored in row-major order, each channel in [0, 1].
struct Image {
    int width = 0;
    int height = 0;

    std::vector<float> pixels;

    float& at(int x, int y, int c) { return pixels[(y * width + x) * 4 + c]; }
    float at(int x, int y, int c) const { return pixels[(y * width + x) * 4 + c]; }

    bool empty() const { return pixels.empty(); }
};

// Load a PNG (or any stb_image-supported format: png, bmp, tga) from disk.
Image loadImage(const std::string& path);

// Load an OpenEXR (.exr) file from disk using tinyexr.
Image loadImageEXR(const std::string& path);

// Dispatch loadImage or loadImageEXR based on the file extension.
Image loadImageAuto(const std::string& path);

// Save an Image as a PNG file.
void saveImage(const std::string& path, const Image& img);

// Save an Image as an OpenEXR (.exr) file using tinyexr.
void saveImageEXR(const std::string& path, const Image& img);

// Load a Radiance HDR (.hdr) image from disk using stbi_loadf.
Image loadImageHDR(const std::string& path);

// Save an Image as a Radiance HDR (.hdr) file using stbi_write_hdr.
void saveImageHDR(const std::string& path, const Image& img);
