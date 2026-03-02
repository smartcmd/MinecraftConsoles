#include "TileEntityRenderer.h"
class PistonPieceEntity;
class TileRenderer;

class PistonPieceRenderer : public TileEntityRenderer 
{
private:
	TileRenderer *tileRenderer;

public:
	PistonPieceRenderer();
    virtual void render(shared_ptr<TileEntity> _entity, double x, double y, double z, float a, bool setColor, float alpha=1.0f, bool useCompiled = true); // 4J added setColor param
	virtual void onNewLevel(Level *level);
};
