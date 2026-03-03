#ifdef _WINDOWS64

#include "BinkDecoder.h"

#include <cmath>
#include <cstring>
#include <algorithm>

// Heavily referenced from https://github.com/FFmpeg/FFmpeg/blob/master/libavcodec/binkaudio.c 
// and https://wiki.multimedia.cx/index.php?title=Bink_Audio

#define M_PI 3.14159265358979323846

static const int critical_freqs[25] = {
    100, 200, 300, 400, 510, 630, 770, 920,
    1080, 1270, 1480, 1720, 2000, 2320, 2700, 3150,
    3700, 4400, 5300, 6400, 7700, 9500, 12000, 15500, 24500
};

static const int rle_lengths[16] = {2,3,4,5,6,8,9,10,11,12,13,14,15,16,32,64};

BinkDecoder::BinkDecoder()
    : fileOpen(false), sampleRate(0), channels(0), frameLength(0), windowLength(0),
      transform(TransformType::DCT)
{
    prevBuffer[0].resize(8192, 0.0f);
    prevBuffer[1].resize(8192, 0.0f);
}

BinkDecoder::~BinkDecoder() { close(); }

bool BinkDecoder::open(const std::string& filename) {
    close();
    fileStream.open(filename, std::ios::binary);
    if (!fileStream.is_open()) return false;
    if (!parseHeader()) { fileStream.close(); return false; }
    fileOpen = true;
    return true;
}

void BinkDecoder::close() {
    if (fileStream.is_open()) fileStream.close();
    fileOpen = false;
}

bool BinkDecoder::parseHeader() {
    uint32_t reportedSize = 0;
    fileStream.read(reinterpret_cast<char*>(&reportedSize), sizeof(uint32_t));
    if (fileStream.gcount() != 4 || reportedSize == 0) return false;

    sampleRate = 44100;
    channels = 2;
    transform = TransformType::DCT;

    if (sampleRate < 22050) frameLength = 2048;
    else if (sampleRate < 44100) frameLength = 4096;
    else frameLength = 8192;

    windowLength = frameLength / 16;
    return true;
}

float BinkDecoder::readFloat29() {
    uint32_t raw = 0;
    fileStream.read(reinterpret_cast<char*>(&raw), 4);
    if (fileStream.gcount() != 4) return 0.0f;
    int exponent = (raw >> 23) & 0x1F;
    int mantissa = raw & 0x7FFFFF;
    bool sign = (raw >> 31) != 0;
    float value = ldexpf(static_cast<float>(mantissa), exponent - 23);
    return sign ? -value : value;
}

bool BinkDecoder::readPacket(std::vector<uint8_t>& packet) {
    uint32_t packetSize = 0;
    fileStream.read(reinterpret_cast<char*>(&packetSize), 4);
    if (fileStream.eof() || packetSize == 0) return false;
    packet.resize(packetSize);
    fileStream.read(reinterpret_cast<char*>(packet.data()), packetSize);
    return fileStream.gcount() == static_cast<std::streamsize>(packetSize);
}

void BinkDecoder::applyDCT(std::vector<float>& coeffs) {
    size_t N = coeffs.size();
    std::vector<float> tmp(N, 0.0f);
    for (size_t k = 0; k < N; k++) {
        float sum = 0.0f;
        for (size_t n = 0; n < N; n++)
            sum += coeffs[n] * cosf(M_PI * (n + 0.5f) * k / N);
        tmp[k] = sum;
    }
    coeffs = tmp;
}

void BinkDecoder::applyRDFT(std::vector<float>& coeffs) {
    size_t N = coeffs.size();
    std::vector<float> tmp(N, 0.0f);
    for (size_t k = 0; k < N; k++) {
        float sumRe = 0.0f;
        float sumIm = 0.0f;
        for (size_t n = 0; n < N; n++) {
            float angle = 2.0f * M_PI * k * n / N;
            sumRe += coeffs[n] * cosf(angle);
            sumIm -= coeffs[n] * sinf(angle);
        }
        tmp[k] = sumRe;
    }
    coeffs = tmp;
}

void BinkDecoder::decodeBlock(const std::vector<uint8_t>& packet, std::vector<std::vector<float>>& output) {
    size_t bitPos = 0;
    auto readBit = [&]() -> bool {
        if (bitPos / 8 >= packet.size()) return 0;
        bool b = (packet[bitPos / 8] >> (7 - (bitPos % 8))) & 1;
        bitPos++;
        return b;
    };
    auto readBits = [&](int n) -> uint32_t {
        uint32_t val = 0;
        for (int i = 0; i < n; i++) val = (val << 1) | readBit();
        return val;
    };

    std::vector<float> coeffs(frameLength, 0.0f);
    std::vector<float> quant(25, 0.0f);

    for (uint32_t ch = 0; ch < channels; ch++) {
        coeffs[0] = readFloat29();
        coeffs[1] = readFloat29();

        for (int i = 0; i < 25; i++) {
            int value = readBits(8);
            quant[i] = powf(10.0f, std::min(value,95) * 0.0664f);
        }

        int i = 2, k = 0;
        while (i < static_cast<int>(frameLength)) {
            int width = readBits(4);
            if (width == 0) { i++; continue; }
            while (i < static_cast<int>(frameLength)) {
                if (i >= critical_freqs[k]) k++;
                int coeff = readBits(width);
                coeffs[i] = readBit() ? -coeff * quant[k] : coeff * quant[k];
                i++;
            }
        }

        if (transform == TransformType::DCT) applyDCT(coeffs);
        else applyRDFT(coeffs);

        for (uint32_t w = 0; w < windowLength; w++)
            output[ch][w] = (prevBuffer[ch][w] * (windowLength - w) + coeffs[w] * w) / windowLength;
        for (uint32_t s = windowLength; s < frameLength; s++)
            output[ch][s] = coeffs[s];

        std::copy(coeffs.end() - windowLength, coeffs.end(), prevBuffer[ch].begin());
    }
}

int BinkDecoder::decodeFrame(std::vector<std::vector<float>>& output) {
    if (!fileOpen) return -1;
    std::vector<uint8_t> packet;
    if (!readPacket(packet)) return -1;

    output.resize(channels);
    for (auto& buf : output) buf.resize(frameLength, 0.0f);

    decodeBlock(packet, output);
    return static_cast<int>(frameLength);
}

#endif