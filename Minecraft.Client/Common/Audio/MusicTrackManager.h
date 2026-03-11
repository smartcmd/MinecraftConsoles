#pragma once

#include <unordered_map>
#include <cstring>   // for memset
#include <cassert>

// Forward declaration of the random number generator used by SoundEngine.
// Replace with your actual random class if different.
class Random;

/**
 * Manages selection of music tracks across different gameplay domains
 * (Menu, Overworld Survival, Overworld Creative, Nether, End).
 * Each domain maintains its own range of track indices and a history
 * of recently played tracks to avoid immediate repetition.
 */
class MusicTrackManager
{
public:
    // Enumeration of all music domains. The values match the original
    // SoundEngine constants for easy conversion.
    enum class Domain
    {
        Menu = -1,
        OverworldSurvival = 0,
        OverworldCreative = 1,
        Nether = 2,
        End = 3,
        None = 4   // Special value for when no background music should play (e.g., during a music disc)
    };

    /**
     * Constructor.
     * @param rng Pointer to a random number generator (must remain valid).
     */
    explicit MusicTrackManager(Random* rng);

    /**
     * Destructor – frees all dynamically allocated heard-track arrays.
     */
    ~MusicTrackManager();

    // Prevent copying (to avoid double deletion of internal arrays).
    MusicTrackManager(const MusicTrackManager&) = delete;
    MusicTrackManager& operator=(const MusicTrackManager&) = delete;

    /**
     * Sets the inclusive index range for a given domain.
     * @param domain The music domain.
     * @param minIdx First valid track index.
     * @param maxIdx Last valid track index (must be >= minIdx).
     */
    void setDomainRange(Domain domain, int minIdx, int maxIdx);

    /**
     * Selects a track index for the specified domain.
     * Tries to return a track that has not been played recently;
     * if all tracks have been played, resets the history and picks one randomly.
     * For domains with only one track, that track is always returned.
     * @param domain The domain for which to choose a track.
     * @return A valid track index within the domain's range.
     */
    int selectTrack(Domain domain);

    /**
     * Resets the heard history for a domain, marking all tracks as "not heard".
     * Useful when switching domains or after a major game state change.
     * @param domain The domain to reset.
     */
    void resetDomain(Domain domain);

    int getDomainMin(Domain d) const { return getInfo(d).minIdx; }
    int getDomainMax(Domain d) const { return getInfo(d).maxIdx; }

private:
    // Internal structure storing range and heard-array for a domain.
    struct DomainInfo
    {
        int minIdx;           // First track index (inclusive)
        int maxIdx;           // Last track index (inclusive)
        int trackCount;       // Number of tracks in this domain
        bool* heard;          // Dynamic array: heard[i] == true if track (minIdx + i) has been played recently

        DomainInfo(int minVal = 0, int maxVal = 0)
            : minIdx(minVal), maxIdx(maxVal), trackCount(maxVal - minVal + 1)
        {
            heard = new bool[trackCount]();  // value-initialized to false
        }

        // Move constructor (optional, but needed if we want to store in unordered_map with emplace)
        DomainInfo(DomainInfo&& other) noexcept
            : minIdx(other.minIdx), maxIdx(other.maxIdx), trackCount(other.trackCount), heard(other.heard)
        {
            other.heard = nullptr;  // prevent double deletion
        }

        // Destructor
        ~DomainInfo()
        {
            delete[] heard;
        }

        // No copy
        DomainInfo(const DomainInfo&) = delete;
        DomainInfo& operator=(const DomainInfo&) = delete;
    };

    // Map from Domain to its info.
    std::unordered_map<Domain, DomainInfo> m_domains;

    // Pointer to the random number generator.
    Random* m_random;

    // Helper to get the info for a domain (assumes domain exists).
    DomainInfo& getInfo(Domain domain);
    const DomainInfo& getInfo(Domain domain) const;
};
