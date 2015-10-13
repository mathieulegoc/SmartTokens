package fr.aviz.tokennotifier.sinks.misc;

import fr.aviz.tokennotifier.sinks.NotificationSink;
import fr.aviz.tokennotifier.sinks.NotificationSinkFactory;

public class MiscNotifiers implements NotificationSinkFactory {

	final NotificationSink[] sinks = {
			new ConsoleNotifier(),
			new SpeechNotifier(),
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
	public NotificationSink[] getAllNotificationSinks() {
		return sinks;
	}
	
	@Override
	public void close() {
	}
}
