package fr.aviz.tokennotifier.mapping;

import java.util.ArrayList;

import fr.aviz.tokennotifier.sinks.NotificationSink;
import fr.aviz.tokennotifier.sources.NotificationSource;

public class NotificationMapping {

	NotificationSource source;
	ArrayList<NotificationSink> sinks = new ArrayList<NotificationSink>();
	
	public NotificationMapping(NotificationSource source) {
		this.source = source;
	}
	
	public void addSink(NotificationSink sink) {
		if (!sinks.contains(sink))
			sinks.add(sink);
	}
	
	public NotificationSource getSource() {
		return source;
	}
	
	public ArrayList<NotificationSink> getSinks() {
		return sinks;
	}

	public void removeSink(NotificationSink sink) {
		if (sinks.contains(sink))
			sinks.remove(sink);
	}
	
	public boolean isEmpty() {
		return sinks.size() == 0;
	}
}
