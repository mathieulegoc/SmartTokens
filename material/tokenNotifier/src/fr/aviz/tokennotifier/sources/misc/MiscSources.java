package fr.aviz.tokennotifier.sources.misc;

import fr.aviz.tokennotifier.sources.NotificationSource;
import fr.aviz.tokennotifier.sources.NotificationSourceFactory;

public class MiscSources implements NotificationSourceFactory {

	NotificationSource[] sources = new NotificationSource[] {
		new KeySource(1),
		new KeySource(2),
		new KeySource(3),
		new KeySource(4),
	};
	
	@Override
	public String getNotificationFactoryName() {
		return "MISC";
	}

	@Override
	public boolean open() {
		return true;
	}
	
	@Override
	public NotificationSource[] getAllNotificationSources() {
		return sources;
	}
	
	@Override
	public void close() {
	}
	
}
