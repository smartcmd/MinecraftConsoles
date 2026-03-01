#pragma once
/*
class MapExtendingRecipe extends ShapedRecipy {
	public MapExtendingRecipe() {
		super(3, 3, new ItemInstance[] {
			new ItemInstance(Item.paper), new ItemInstance(Item.paper), new ItemInstance(Item.paper),
				new ItemInstance(Item.paper), new ItemInstance(Item.map, 0, Recipes.ANY_AUX_VALUE), new ItemInstance(Item.paper),
				new ItemInstance(Item.paper), new ItemInstance(Item.paper), new ItemInstance(Item.paper),
		}, new ItemInstance(Item.emptyMap, 0, 0));
	}

	@Override
		public boolean matches(CraftingContainer craftSlots, Level level) {
			if (!super.matches(craftSlots, level)) return false;
			ItemInstance map = null;

			for (int i = 0; i < craftSlots.getContainerSize() && map == null; i++) {
				ItemInstance item = craftSlots.getItem(i);
				if (item != null && item.id == Item.map.id) map = item;
			}

			if (map == null) return false;
			MapItemSavedData data = Item.map.getSavedData(map, level);
			if (data == null) return false;
			return data.scale < MapItemSavedData.MAX_SCALE;
	}

	@Override
		public ItemInstance assemble(CraftingContainer craftSlots) {
			ItemInstance map = null;

			for (int i = 0; i < craftSlots.getContainerSize() && map == null; i++) {
				ItemInstance item = craftSlots.getItem(i);
				if (item != null && item.id == Item.map.id) map = item;
			}

			map = map.copy();
			map.count = 1;

			if (map.getTag() == null) map.setTag(new CompoundTag());
			map.getTag().putBoolean("map_is_scaling", true);

			return map;
	}
};
*/