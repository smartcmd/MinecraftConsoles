#pragma once

#ifdef _WINDOWS64

#include <fstream>
#include <vector>
#include <cstdint>
#include <string>


class BinkDecoder
{
public:
    enum class TransformType {
        DCT,
        RDFT
    };

    BinkDecoder();
    ~BinkDecoder();

    bool open(const std::string& filename);
    void close();

    int decodeFrame(std::vector<std::vector<float>>& output);

    bool isOpen() const { return fileOpen; }
    uint32_t getSampleRate() const { return sampleRate; }
    uint32_t getChannels() const { return channels; }
    uint32_t getFrameLength() const { return frameLength; }
    TransformType getTransform() const { return transform; }

private:
    std::ifstream fileStream;
    bool fileOpen;

    uint32_t sampleRate;
    uint32_t channels;
    uint32_t frameLength;
    uint32_t windowLength;
    TransformType transform;

    std::vector<float> prevBuffer[2];

    bool parseHeader();
    float readFloat29();
    bool readPacket(std::vector<uint8_t>& packet);
    void decodeBlock(const std::vector<uint8_t>& packet, std::vector<std::vector<float>>& output);

    void applyDCT(std::vector<float>& coeffs);
    void applyRDFT(std::vector<float>& coeffs);
};

#endif