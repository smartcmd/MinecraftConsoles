#pragma once
using namespace std;

class Particle;
class Level;
class Textures;
class Entity;
class Random;
using namespace std;

class ParticleEngine
{
private:
	static ResourceLocation PARTICLES_LOCATION;
	static const int MAX_PARTICLES_PER_LAYER = 200;		// 4J - reduced from 4000
	static const int MAX_DRAGON_BREATH_PARTICLES = 1000;
	static const int MAX_FIREWORK_SPARK_PARTICLES = 2000;

public:
	static const int MISC_TEXTURE = 0;
    static const int TERRAIN_TEXTURE = 1;
    static const int ITEM_TEXTURE = 2;
    static const int ENTITY_PARTICLE_TEXTURE = 3;
	static const int DRAGON_BREATH_TEXTURE = 4; // 4J Added
    static const int TEXTURE_COUNT = 5;

	// Brought forward from Java 1.8
	static const int TRANSLUCENT_LIST = 0;
	static const int OPAQUE_LIST = 1;
	static const int LIST_COUNT = 2;

protected:
	Level *level;
private:
	deque<shared_ptr<Particle> > particles[3][TEXTURE_COUNT][LIST_COUNT];		// 4J made three arrays to cope with simultaneous two dimensions
    Textures *textures;
    Random *random;

public:
	ParticleEngine(Level *level, Textures *textures);
	~ParticleEngine();
    void add(shared_ptr<Particle> p);
    void tick();
    void render(shared_ptr<Entity> player, float a, int list);
    void renderLit(shared_ptr<Entity> player, float a, int list);
    void setLevel(Level *level);
    void destroy(int x, int y, int z, int tid, int data);
    void crack(int x, int y, int z, int face);

	// 4J - Brought forward from Java 1.8
	void markTranslucent(shared_ptr<Particle> particle);
	void markOpaque(shared_ptr<Particle> particle);
	void moveParticleInList(shared_ptr<Particle> particle, int source, int destination);

	wstring countParticles();
};