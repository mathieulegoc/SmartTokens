package fr.aviz.sensingtokens;

public class TokenEvent {

	final byte tokenId;
	final byte from;
	final byte to;
	final byte grip;
	
	public TokenEvent(byte tokenId, byte transition, byte grip) {
		this.tokenId = tokenId;
		this.from = (byte) (0x0F & transition>>4);
		this.to = (byte) (0x0F & transition);
		this.grip = grip;
	}
	
	public byte getPreviousState() {
		return from;
	}

	public byte getState() {
		return to;
	}

	public byte getTransition() {
		return Transitions.transition(from, to);
	}

	public byte getGrip() {
		return grip;
	}

	public byte getTokenId() {
		return tokenId;
	}
	
}
