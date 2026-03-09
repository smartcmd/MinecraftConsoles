# Scope of Project
At the moment, this project's scope is generally limited outside of adding new content to the game (blocks, mobs, items). We are currently prioritizing stability, quality of life, and platform support over these things.

## Parity
We are attempting to keep our version of LCE as close to visual and experience parity with the original console experience of LCE as possible. This means that we will not be accepting changes that...
- Backport things from Java Edition that did not ever exist in LCE
- Swap out LCE visuals for Java Edition or Bedrock Edition style visuals
- Change LCE defaults in favor of different defaults if it changes the experience
  - For example, increasing mob spawn limits without increasing the area mobs can spawn within, aka increasing mob density past what was the original console experience
- Redesign UI components different than LCE
- Break controller support, or otherwise do not support play with a controller
- Add custom texture packs or DLC that never existed in LCE
- Add any gameplay content (block, item, mob) that has no existing point of reference in any official LCE build

However, we would accept changes that...
- Fix legitimately buggy or inconsistent behavior in LCE that causes unexpected outcomes
  - For example, mobs clipping outside of walls, clipping through the world, broken mechanics
- Add features to better support multi-platform use of LCE, such as video and control settings
  - These menus need to respect the visual style of LCE, though.
- Replace existing UI systems with SWF-free rendering techniques that are as visually and functionally identical as possible
- Improve the quality of assets (such as sounds) while preserving their contents
  - For example, upgrading the quality of all music in-game while preserving any unique cuts / versions, or faithfully remaking those unique cuts / versions with higher quality assets
- Backport things like modern skin rendering
- Change the code from using non-stitched textures to individually named texture PNGs and stitching at runtime
- Adding menus to better support custom dedicated servers with their own fixed IPs
- Add support for things like Steamworks Networking and other P2P networking and auth strategies
- Improve Keyboard and Mouse control support
- Add minimal, non-invasive Quality of Life features that don't otherwise compromise the LCE experience
  - For example, adjusting certain crafting recipes or change item behaviors like non-stackable doors 


## Current Goals
- Being a robust Desktop version of LCE
- Having proper controller support across all types, brands on Desktop or Desktop-like platforms (Steam Deck)
- Improving stability as much as possible
- Fixing as many bugs as possible
- Enabling Desktop multiplayer options
  - LAN P2P Multiplayer
  - Splitscreen Multiplayer
  - WAN Servers (IP:Port connectivity)
  - Platform-based P2P Connectivity
    - Steam Networking
    - GameDate?
    - Maybe more?
- Refining rendering settings, renderer options, as well as reaching rendering parity with true LCE
- Having workable multi-platform compilation for ARM, Consoles, Linux
- Being a good base for further expansion and modding of LCE, such as backports and "modpacks".

# Use of AI and LLMs
We currently do not accept any new code into the project that was written largely, entirely, or even noticably by an LLM. All contributions should be made by humans that understand the codebase.

# Pull Request Template
We request that all PRs made for this repo use our PR template to the fullest extent possible. Completely wiping it out to write minimal information will likely get your PR closed.
