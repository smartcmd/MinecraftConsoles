#pragma once

class ChestTileEntity;
class EnderChestTileEntity;
class Tile;

class EntityTileRenderer
 {
 public:
	 static EntityTileRenderer *instance;

 private:
	 shared_ptr<ChestTileEntity> chest;
	 shared_ptr<ChestTileEntity> trappedChest;
	 shared_ptr<EnderChestTileEntity> enderChest;

 public:
	 EntityTileRenderer();
	 void render(Tile *tile, int data, float brightness, float alpha, bool setColor = true, bool useCompiled = true);		// 4J - added setColor parameter and alpha for chest in the crafting menu, and added useCompiled
};
