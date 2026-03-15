#include "stdafx.h"
#include "MusicTrackManager.h"
#include "../Minecraft.World/Random.h"

MusicTrackManager::MusicTrackManager(Random* rng)
    : m_random(rng)
{
    // The domains will be filled later via setDomainRange().
}

MusicTrackManager::~MusicTrackManager()
{
    // unordered_map will automatically destroy its DomainInfo objects,
    // which in turn delete[] the heard arrays.
}

void MusicTrackManager::setDomainRange(Domain domain, int minIdx, int maxIdx)
{
    assert(minIdx <= maxIdx);
    // Use emplace to construct the DomainInfo in-place.
    // If the domain already exists, this will replace it (C++17).
    m_domains.erase(domain);  // Remove old entry if any
    m_domains.emplace(domain, DomainInfo(minIdx, maxIdx));
}

int MusicTrackManager::selectTrack(Domain domain)
{
    // Special case: if domain is None, return -1 (no track).
    if (domain == Domain::None)
        return -1;

    DomainInfo& info = getInfo(domain);

    // If range contains only one track, just return that track.
    if (info.trackCount == 1)
        return info.minIdx;

    // Check whether all tracks have been heard.
    bool allHeard = true;
    for (int i = 0; i < info.trackCount; ++i)
    {
        if (!info.heard[i])
        {
            allHeard = false;
            break;
        }
    }

    // If all tracks have been heard, reset the heard flags.
    if (allHeard)
    {
        std::memset(info.heard, 0, sizeof(bool) * info.trackCount);
    }

    // Try up to (trackCount/2 + 1) times to pick a track that hasn't been heard.
    // This biases toward unplayed tracks but doesn't guarantee it if the random
    // keeps hitting played ones. It's a compromise between fairness and performance.
    const int maxAttempts = info.trackCount / 2 + 1;
    for (int attempt = 0; attempt < maxAttempts; ++attempt)
    {
        int idx = m_random->nextInt(info.trackCount);  // 0 .. trackCount-1
        if (!info.heard[idx])
        {
            info.heard[idx] = true;
            return info.minIdx + idx;
        }
    }

    // Fallback: if we couldn't find an unplayed track (should be rare),
    // just pick any random track and mark it as heard.
    int fallbackIdx = m_random->nextInt(info.trackCount);
    info.heard[fallbackIdx] = true;
    return info.minIdx + fallbackIdx;
}

void MusicTrackManager::resetDomain(Domain domain)
{
    DomainInfo& info = getInfo(domain);
    std::memset(info.heard, 0, sizeof(bool) * info.trackCount);
}

// ---------- private helpers ----------

MusicTrackManager::DomainInfo& MusicTrackManager::getInfo(Domain domain)
{
    auto it = m_domains.find(domain);
    assert(it != m_domains.end() && "Domain not initialized. Call setDomainRange first.");
    return it->second;
}

const MusicTrackManager::DomainInfo& MusicTrackManager::getInfo(Domain domain) const
{
    auto it = m_domains.find(domain);
    assert(it != m_domains.end() && "Domain not initialized.");
    return it->second;
}

