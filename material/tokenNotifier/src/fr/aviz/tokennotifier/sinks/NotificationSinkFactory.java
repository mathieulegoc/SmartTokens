package fr.aviz.tokennotifier.sinks;

import fr.aviz.tokennotifier.mapping.NotificationFactory;

public interface NotificationSinkFactory extends NotificationFactory {

	public NotificationSink[] getAllNotificationSinks();
	
}
