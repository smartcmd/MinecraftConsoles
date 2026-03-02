#include "stdafx.h"
#include "Camera.h"
#include "MemoryTracker.h"
#include "..\Minecraft.World\net.minecraft.world.entity.player.h"
#include "..\Minecraft.World\net.minecraft.world.level.h"
#include "..\Minecraft.World\net.minecraft.world.level.tile.h"
#include "..\Minecraft.World\TilePos.h"

float Camera::xPlayerOffs = 0.0f;
float Camera::yPlayerOffs = 0.0f;
float Camera::zPlayerOffs = 0.0f;

//IntBuffer *Camera::viewport		= MemoryTracker::createIntBuffer(16);
FloatBuffer *Camera::modelview	= MemoryTracker::createFloatBuffer(16);
FloatBuffer *Camera::projection = MemoryTracker::createFloatBuffer(16);
//FloatBuffer *Camera::position	= MemoryTracker::createFloatBuffer(3);

float Camera::xa = 0.0f;
float Camera::ya = 0.0f;
float Camera::za = 0.0f;
float Camera::xa2 = 0.0f;
float Camera::za2 = 0.0f;

void Camera::prepare(shared_ptr<Player> player, bool mirror)
{
    glGetFloat(GL_MODELVIEW_MATRIX, modelview);
    glGetFloat(GL_PROJECTION_MATRIX, projection);

	/* Original java code for reference
    glGetInteger(GL_VIEWPORT, viewport);

    float x = (viewport.get(0) + viewport.get(2)) / 2;
    float y = (viewport.get(1) + viewport.get(3)) / 2;
    gluUnProject(x, y, 0, modelview, projection, viewport, position);

    xPlayerOffs = position->get(0);
    yPlayerOffs = position->get(1);
    zPlayerOffs = position->get(2);
	*/

	// Xbox conversion here... note that we don't bother getting the viewport as this is just working out how to get a (0,0,0) point in clip space to pass into the inverted
	// combined model/view/projection matrix, so we just need to get this matrix and get its translation as an equivalent.
	XMMATRIX _modelview, _proj, _final, _invert;
	XMVECTOR _det;
	XMFLOAT4 trans;

	memcpy( &_modelview, modelview->_getDataPointer(), 64 );
	memcpy( &_proj, projection->_getDataPointer(), 64 );

#if ( defined __ORBIS__ ) || ( defined __PSVITA__ )
	_modelview = transpose(_modelview);
	_proj = transpose(_proj);
	_final = _modelview * _proj;
	_invert = sce::Vectormath::Simd::Aos::inverse(_final);
	xPlayerOffs = _invert.getElem(0,3) / _invert.getElem(3,3);
	yPlayerOffs = _invert.getElem(1,3) / _invert.getElem(3,3);
	zPlayerOffs = _invert.getElem(2,3) / _invert.getElem(3,3);
#elif defined  __PS3__
	_modelview = transpose(_modelview);
	_proj = transpose(_proj);
	_final = _modelview * _proj;
	_invert = Vectormath::Aos::inverse(_final);
	xPlayerOffs = _invert.getElem(0,3) / _invert.getElem(3,3);
	yPlayerOffs = _invert.getElem(1,3) / _invert.getElem(3,3);
	zPlayerOffs = _invert.getElem(2,3) / _invert.getElem(3,3);
#else
	_final = XMMatrixMultiply( _modelview, _proj );
	_det = XMMatrixDeterminant(_final);
	_invert = XMMatrixInverse(&_det, _final);

	XMStoreFloat4(&trans,_invert.r[3]);

	xPlayerOffs = trans.x / trans.w;
	yPlayerOffs = trans.y / trans.w;
	zPlayerOffs = trans.z / trans.w;
#endif

    int flipCamera = mirror ? 1 : 0;

    float xRot = player->xRot;
    float yRot = player->yRot;

    xa = cosf(yRot * PI / 180.0f) * (1 - flipCamera * 2);
    za = sinf(yRot * PI / 180.0f) * (1 - flipCamera * 2);

    xa2 = -za * sinf(xRot * PI / 180.0f) * (1 - flipCamera * 2);
    za2 = xa * sinf(xRot * PI / 180.0f) * (1 - flipCamera * 2);
    ya = cosf(xRot * PI / 180.0f);
}

TilePos *Camera::getCameraTilePos(shared_ptr<LivingEntity> player, double alpha)
{
	return new TilePos(getCameraPos(player, alpha));
}

Vec3 *Camera::getCameraPos(shared_ptr<LivingEntity> player, double alpha)
{
    double xx = player->xo + (player->x - player->xo) * alpha;
    double yy = player->yo + (player->y - player->yo) * alpha + player->getHeadHeight();
    double zz = player->zo + (player->z - player->zo) * alpha;

    double xt = xx + Camera::xPlayerOffs * 1;
    double yt = yy + Camera::yPlayerOffs * 1;
    double zt = zz + Camera::zPlayerOffs * 1;

    return Vec3::newTemp(xt, yt, zt);
}

int Camera::getBlockAt(Level *level, shared_ptr<LivingEntity> player, float alpha)
{
    Vec3 *p = Camera::getCameraPos(player, alpha);
    TilePos tp = TilePos(p);
    int t = level->getTile(tp.x, tp.y, tp.z);
    if (t != 0 && Tile::tiles[t]->material->isLiquid())
	{
        float hh = LiquidTile::getHeight(level->getData(tp.x, tp.y, tp.z)) - 1 / 9.0f;
        float h = tp.y + 1 - hh;
        if (p->y >= h)
		{
            t = level->getTile(tp.x, tp.y + 1, tp.z);
        }
    }
    return t;
}