#pragma once

#include "stdafx.h"

class ClientInformationPacket : public Packet
{
#if 0
	private String language;
	private int viewDistance;
	private int chatVisibility;
	private boolean chatColors;
	private int difficulty;

	public ClientInformationPacket() {
	}

	public ClientInformationPacket(String language, int viewDistance, int chatVisibility, boolean chatColors, int difficulty, boolean showCape) {
		this.language = language;
		this.viewDistance = viewDistance;
		this.chatVisibility = chatVisibility;
		this.chatColors = chatColors;
		this.difficulty = difficulty;
        this.showCape = showCape;
	}

	@Override
		public void read(DataInputStream dis) throws IOException {
			language = readUtf(dis, 7);
			viewDistance = dis.readByte();

			int chat = dis.readByte();
			chatVisibility = chat & 0x7;
			chatColors = (chat & 0x8) == 0x8;

			difficulty = dis.readByte();
        showCape = dis.readBoolean();
	}

	@Override
		public void write(DataOutputStream dos) throws IOException {
			writeUtf(language, dos);
			dos.writeByte(viewDistance);
			dos.writeByte(chatVisibility | (chatColors ? 1 : 0) << 3);
			dos.writeByte(difficulty);
        dos.writeBoolean(showCape);
	}

	@Override
		public void handle(PacketListener listener) {
			listener.handleClientInformation(this);
	}

	@Override
		public int getEstimatedSize() {
			return 7;
	}

	public String getLanguage() {
		return language;
	}

	public int getViewDistance() {
		return viewDistance;
	}

	public int getChatVisibility() {
		return chatVisibility;
	}

	public boolean getChatColors() {
		return chatColors;
	}

	public int getDifficulty() {
		return difficulty;
	}

    public boolean getShowCape() {
        return showCape;
    }

	public void setDifficulty(int difficulty) {
		this.difficulty = difficulty;
	}

	@Override
		public String getDebugInfo() {
			return String.format("lang='%s', view=%d, chat=%d, col=%b, difficulty=%d", language, viewDistance, chatVisibility, chatColors, difficulty);
	}

	@Override
		public boolean canBeInvalidated() {
			return true;
	}

	@Override
		public boolean isInvalidatedBy(Packet packet) {
			return true;
	}
#endif
};