package fr.aviz.tokennotifier.mapping;

public interface NotificationFactory {

	public String getNotificationFactoryName();
	
	public boolean open();
	
	public void close();
}
