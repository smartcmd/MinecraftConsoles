#pragma once

/*
class MapCloningRecipe implements Recipy {
	@Override
		public boolean matches(CraftingContainer craftSlots, Level level) {
			int count = 0;
			ItemInstance source = null;

			for (int slot = 0; slot < craftSlots.getContainerSize(); slot++) {
				ItemInstance item = craftSlots.getItem(slot);
				if (item == null) continue;

				if (item.id == Item.map.id) {
					if (source != null) return false;
					source = item;
				} else if (item.id == Item.emptyMap.id) {
					count++;
				} else {
					return false;
				}
			}

			return source != null && count > 0;
	}

	@Override
		public ItemInstance assemble(CraftingContainer craftSlots) {
			int count = 0;
			ItemInstance source = null;

			for (int slot = 0; slot < craftSlots.getContainerSize(); slot++) {
				ItemInstance item = craftSlots.getItem(slot);
				if (item == null) continue;

				if (item.id == Item.map.id) {
					if (source != null) return null;
					source = item;
				} else if (item.id == Item.emptyMap.id) {
					count++;
				} else {
					return null;
				}
			}

			if (source == null || count < 1) return null;

			ItemInstance result = new ItemInstance(Item.map, count + 1, source.getAuxValue());
			if (source.hasCustomHoverName()) result.setHoverName(source.getHoverName());
			return result;
	}

	@Override
		public int size() {
			return 9;
	}

	@Override
		public ItemInstance getResultItem() {
			return null;
	}
};
*/