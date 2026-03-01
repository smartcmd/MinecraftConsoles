#pragma once
#include "TileEntityRenderer.h"

class BookModel;

class EnchantTableRenderer : public TileEntityRenderer
{
	friend class CXuiCtrlEnchantmentBook;
	friend class UIControl_EnchantmentBook;
private:
	static ResourceLocation BOOK_LOCATION;

	BookModel *bookModel;

public:
	EnchantTableRenderer();
	~EnchantTableRenderer();

	virtual void render(shared_ptr<TileEntity> _table, double x, double y, double z, float a, bool setColor, float alpha=1.0f, bool useCompiled = true);
};
