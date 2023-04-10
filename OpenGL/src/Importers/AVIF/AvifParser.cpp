#include "AvifParser.h"

/*unsigned char* LoadAVIF(const char* filename, int* width, int* height, int* bpp, int desiredChannels) {
    avifROData raw;
    avifResult result = avifRWDataReadFile(&raw, filename);
    if (result != AVIF_RESULT_OK) {
        std::cerr << "Error reading AVIF file: " << avifResultToString(result) << std::endl;
        return nullptr;
    }

    avifDecoder* decoder = avifDecoderCreate();
    result = avifDecoderSetIOMemory(decoder, raw.data, raw.size);
    if (result != AVIF_RESULT_OK) {
        std::cerr << "Error setting AVIF decoder IO: " << avifResultToString(result) << std::endl;
        avifRWDataFree(&raw);
        avifDecoderDestroy(decoder);
        return nullptr;
    }

    result = avifDecoderParse(decoder);
    if (result != AVIF_RESULT_OK) {
        std::cerr << "Error parsing AVIF file: " << avifResultToString(result) << std::endl;
        avifRWDataFree(&raw);
        avifDecoderDestroy(decoder);
        return nullptr;
    }

    result = avifDecoderNextImage(decoder);
    if (result != AVIF_RESULT_OK) {
        std::cerr << "Error decoding AVIF image: " << avifResultToString(result) << std::endl;
        avifRWDataFree(&raw);
        avifDecoderDestroy(decoder);
        return nullptr;
    }

    *width = decoder->image->width;
    *height = decoder->image->height;
    *bpp = desiredChannels;

    avifRGBImage rgb;
    avifRGBImageSetDefaults(&rgb, decoder->image);
    rgb.format = AVIF_RGB_FORMAT_RGBA;
    rgb.depth = 8;
    avifRGBImageAllocatePixels(&rgb);

    result = avifImageYUVToRGB(decoder->image, &rgb);
    if (result != AVIF_RESULT_OK) {
        std::cerr << "Error converting AVIF image to RGB: " << avifResultToString(result) << std::endl;
        avifRGBImageFreePixels(&rgb);
        avifRWDataFree(&raw);
        avifDecoderDestroy(decoder);
        return nullptr;
    }

    std::vector<unsigned char> imageData(*width * *height * desiredChannels);
    std::copy(rgb.pixels, rgb.pixels + imageData.size(), imageData.data());

    avifRGBImageFreePixels(&rgb);
    avifRWDataFree(&raw);
    avifDecoderDestroy(decoder);

    unsigned char* result = new unsigned char[imageData.size()];
    std::copy(imageData.begin(), imageData.end(), result);
    return result;
}*/