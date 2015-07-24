package fr.aviz.sensingtokens;

public class MessageFromServer {
	
	private byte messageType;
	private byte senderId;
	private byte[] payload;

	public MessageFromServer(byte sender, byte type, byte[] data)
	{
		senderId = sender;
		messageType = type;
		payload = data;
	}

	public byte getMessageType() {
		return messageType;
	}

	public void setMessageType(byte messageType) {
		this.messageType = messageType;
	}

	public byte getSenderId() {
		return senderId;
	}

	public void setSenderId(byte senderId) {
		this.senderId = senderId;
	}

	public byte[] getPayload() {
		return payload;
	}

	public void setPayload(byte[] payload) {
		this.payload = payload;
	}      
}
