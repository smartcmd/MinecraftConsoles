#pragma once

/*
class WrittenBookItem extends Item {

	public static final int TITLE_LENGTH = 16;
	public static final int PAGE_LENGTH = 256;
	public static final int MAX_PAGES = 50;
	public static final String TAG_TITLE = "title";
	public static final String TAG_AUTHOR = "author";
	public static final String TAG_PAGES = "pages";

	public WrittenBookItem(int id) {
		super(id);
		setMaxStackSize(1);
	}

	public static boolean makeSureTagIsValid(CompoundTag bookTag) {

		if (!WritingBookItem.makeSureTagIsValid(bookTag)) {
			return false;
		}

		if (!bookTag.contains(TAG_TITLE)) {
			return false;
		}
		String title = bookTag.getString(TAG_TITLE);
		if (title == null || title.length() > TITLE_LENGTH) {
			return false;
		}

		if (!bookTag.contains(TAG_AUTHOR)) {
			return false;
		}

		return true;
	}

	@Override
		public String getHoverName(ItemInstance itemInstance) {
			if (itemInstance.hasTag()) {
				CompoundTag itemTag = itemInstance.getTag();

				StringTag titleTag = (StringTag) itemTag.get(TAG_TITLE);
				if (titleTag != null) {
					return titleTag.toString();
				}
			}
			return super.getHoverName(itemInstance);
	}

	@Override
		public void appendHoverText(ItemInstance itemInstance, Player player, List<String> lines, boolean advanced) {

			if (itemInstance.hasTag()) {
				CompoundTag itemTag = itemInstance.getTag();

				StringTag authorTag = (StringTag) itemTag.get(TAG_AUTHOR);
				if (authorTag != null) {
					lines.add(ChatFormatting.GRAY + String.format(I18n.get("book.byAuthor", authorTag.data)));
				}
			}
	}

	@Override
		public ItemInstance use(ItemInstance itemInstance, Level level, Player player) {
			player.openItemInstanceGui(itemInstance);
			return itemInstance;
	}

	@Override
		public boolean shouldOverrideMultiplayerNBT() {
			return true;
	}

	@Override
		public boolean isFoil(ItemInstance itemInstance) {
			return true;
	}

};
*/