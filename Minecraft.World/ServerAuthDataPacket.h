#pragma once

#include "Packet.h"

class ServerAuthDataPacket : public Packet
{
#if 0
	private String serverId;
	private PublicKey publicKey;
	private uint8_t[] nonce = new uint8_t[]{};

	public ServerAuthDataPacket() {
		// Needed
	}

	public ServerAuthDataPacket(final String serverId, final PublicKey publicKey, final uint8_t[] nonce) {
		this.serverId = serverId;
		this.publicKey = publicKey;
		this.nonce = nonce;
	}

	@Override
		public void read(DataInputStream dis) throws IOException {
			serverId = readUtf(dis, 20);
			publicKey = Crypt.byteToPublicKey(readBytes(dis));
			nonce = readBytes(dis);
	}

	@Override
		public void write(DataOutputStream dos) throws IOException {
			writeUtf(serverId, dos);
			writeBytes(dos, publicKey.getEncoded());
			writeBytes(dos, nonce);
	}

	@Override
		public void handle(PacketListener listener) {
			listener.handleServerAuthData(this);
	}

	@Override
		public int getEstimatedSize() {
			return 2 + serverId.length() * 2 + 2 + publicKey.getEncoded().length + 2 + nonce.length;
	}

	public String getServerId() {
		return serverId;
	}

	public PublicKey getPublicKey() {
		return publicKey;
	}

	public uint8_t[] getNonce() {
		return nonce;
	}
#endif
};