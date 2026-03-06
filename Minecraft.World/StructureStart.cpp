#include "stdafx.h"
#include "net.minecraft.world.level.h"
#include "net.minecraft.world.level.levelgen.structure.h"
#include "StructureStart.h"
#include "StructurePiece.h"
#include "BoundingBox.h"


StructureStart::StructureStart()
{
	chunkX = chunkZ = 0;
	boundingBox = NULL;		// 4J added initialiser
}

StructureStart::StructureStart(int x, int z)
{
	this->chunkX = x;
	this->chunkZ = z;
	boundingBox = NULL;
}

StructureStart::~StructureStart()
{
	for(auto& piece : pieces)
	{
		delete piece;
	}
	delete boundingBox;
}

BoundingBox *StructureStart::getBoundingBox()
{
	return boundingBox;
}

list<StructurePiece *> *StructureStart::getPieces()
{
	return &pieces;
}

void StructureStart::postProcess(Level *level, Random *random, BoundingBox *chunkBB)
{
    auto it = pieces.begin();

    while( it != pieces.end() )
	{
		if( (*it)->getBoundingBox()->intersects(chunkBB) && !(*it)->postProcess(level, random, chunkBB))
		{
			// this piece can't be placed, so remove it to avoid future
			// attempts
			it = pieces.erase(it);
		}
		else
		{
			it++;
		}
	}
}

void StructureStart::calculateBoundingBox()
{
	boundingBox = BoundingBox::getUnknownBox();

	for(auto& piece : pieces)
	{
		boundingBox->expand(piece->getBoundingBox());
	}
}

CompoundTag *StructureStart::createTag(int chunkX, int chunkZ)
{
	CompoundTag *tag = new CompoundTag();

	tag->putString(L"id", StructureFeatureIO::getEncodeId(this));
	tag->putInt(L"ChunkX", chunkX);
	tag->putInt(L"ChunkZ", chunkZ);
	tag->put(L"BB", boundingBox->createTag(L"BB"));

	ListTag<CompoundTag> *childrenTags = new ListTag<CompoundTag>(L"Children");
	for(auto& piece : pieces)
	{
		childrenTags->add(piece->createTag());
	}
	tag->put(L"Children", childrenTags);

	addAdditonalSaveData(tag);

	return tag;
}

void StructureStart::addAdditonalSaveData(CompoundTag *tag)
{

}

void StructureStart::load(Level *level, CompoundTag *tag)
{
	chunkX = tag->getInt(L"ChunkX");
	chunkZ = tag->getInt(L"ChunkZ");
	if (tag->contains(L"BB"))
	{
		boundingBox = new BoundingBox(tag->getIntArray(L"BB"));
	}

	ListTag<CompoundTag> *children = (ListTag<CompoundTag> *) tag->getList(L"Children");
	for (int i = 0; i < children->size(); i++)
	{
		pieces.push_back(StructureFeatureIO::loadStaticPiece(children->get(i), level));
	}

	readAdditonalSaveData(tag);
}

void StructureStart::readAdditonalSaveData(CompoundTag *tag)
{

}

void StructureStart::moveBelowSeaLevel(Level *level, Random *random, int offset)
{
	const int MAX_Y = level->seaLevel - offset;

	// set lowest possible position (at bedrock)
	int y1Pos = boundingBox->getYSpan() + 1;
	// move up randomly within the available span
	if (y1Pos < MAX_Y)
	{
		y1Pos += random->nextInt(MAX_Y - y1Pos);
	}

	// move all bounding boxes
	int dy = y1Pos - boundingBox->y1;
	boundingBox->move(0, dy, 0);
	for(auto& piece : pieces)
	{
		piece->getBoundingBox()->move(0, dy, 0);
	}
}

void StructureStart::moveInsideHeights(Level *level, Random *random, int lowestAllowed, int highestAllowed)
{
	int heightSpan = highestAllowed - lowestAllowed + 1 - boundingBox->getYSpan();
	int y0Pos = 1;

	if (heightSpan > 1)
	{
		y0Pos = lowestAllowed + random->nextInt(heightSpan);
	}
	else
	{
		y0Pos = lowestAllowed;
	}

	// move all bounding boxes
	int dy = y0Pos - boundingBox->y0;
	boundingBox->move(0, dy, 0);
	for(auto& piece : pieces)
	{
		piece->getBoundingBox()->move(0, dy, 0);
	}
}

bool StructureStart::isValid()
{
	return true;
}

int StructureStart::getChunkX()
{
	return chunkX;
}

int StructureStart::getChunkZ()
{
	return chunkZ;
}