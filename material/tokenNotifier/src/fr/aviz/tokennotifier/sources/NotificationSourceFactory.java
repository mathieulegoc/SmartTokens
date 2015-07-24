package fr.aviz.tokennotifier.sources;

import fr.aviz.tokennotifier.mapping.NotificationFactory;

public interface NotificationSourceFactory extends NotificationFactory {

	public NotificationSource[] getAllNotificationSources();
		
}
