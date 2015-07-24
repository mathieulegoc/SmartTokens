package fr.aviz.tokennotifier.mapping;

import java.awt.event.ComponentAdapter;
import java.awt.event.ComponentEvent;
import java.awt.event.ComponentListener;
import java.util.ArrayList;

import fr.aviz.tokennotifier.NotifierMain;
import fr.aviz.tokennotifier.gui.NotificationMappingList;
import fr.aviz.tokennotifier.gui.NotificationSinkList;
import fr.aviz.tokennotifier.gui.NotificationSourceList;
import fr.aviz.tokennotifier.gui.eventviews.EventStack;
import fr.aviz.tokennotifier.sinks.NotificationSink;
import fr.aviz.tokennotifier.sources.NotificationSource;

public class NotificationSystem {

	static ArrayList<NotificationMapping> mappings = new ArrayList<NotificationMapping>();

	// views
	static NotificationSourceList sourceList;
	static NotificationSinkList sinkList;
	static NotificationMappingList mappingList;

	// all registered notification objects. for closing them when app quits.
	public static ArrayList<NotificationObject> allNotificationObjects = new ArrayList<NotificationObject>();
	public static ArrayList<NotificationFactory> allNotificationFactories = new ArrayList<NotificationFactory>();
	
//	static {
//		Runtime.getRuntime().addShutdownHook(new Thread()
//		{
//		    @Override
//		    public void run() {
//				closeAll();
//		    }
//		});
//	}

	public static void sendToSinks(NotificationEvent event) {
		NotificationSource source = event.getSource();
		ArrayList<NotificationSink> sinks = getConnectedSinks(source);
		for (NotificationSink sink : sinks)
			sink.eventReceived(event);
		if (NotifierMain.DEBUG_EVENT_VIEW)
			EventStack.addDebugEvent(event);
	}

	public static void setViews(NotificationMappingList mappingList_, NotificationSourceList sourceList_, NotificationSinkList sinkList_) {
		mappingList = mappingList_;
		sourceList = sourceList_;
		sinkList = sinkList_;		
	}
	
	public static NotificationSourceList getSourceListView() {
		return sourceList;
	}
		
	public static NotificationSinkList getSinkListView() {
		return sinkList;
	}
	
	public static void connect(NotificationSource source, NotificationSink sink) {
		if (!source.attemptedOpen())
			source.doOpen();
		if (!sink.attemptedOpen())
			sink.doOpen();
		if (source.openFailed() || sink.openFailed())
			return;	
		
		NotificationMapping mapping = getMappingWithSource(source);
		if (mapping == null) {
			mapping = new NotificationMapping(source);
			mappings.add(mapping);
			source.setListeningEnabled(true);
		}
		if (!mapping.getSinks().contains(sink)) {
			mapping.addSink(sink);
		}
	}
	
	public static void disconnect(NotificationSource source, NotificationSink sink) {
		NotificationMapping mapping = getMappingWithSource(source);
		if (mapping == null)
			return;
		mapping.removeSink(sink);
		if (mapping.isEmpty()) {
			source.setListeningEnabled(false);
			mappings.remove(mapping);
		}
	}
	
	public static void closeAll() {
		for (NotificationObject object : allNotificationObjects) {
			if (object.isOpen())
				object.close();
		}
		for (NotificationFactory factory : allNotificationFactories) {
			factory.close();
		}
	}
	
	public static boolean isConnected(NotificationSource source, NotificationSink sink) {
		NotificationMapping mapping = getMappingWithSource(source);
		if (mapping == null)
			return false;
		return mapping.getSinks().contains(sink);
	}
	
	public static ArrayList<NotificationSink> getConnectedSinks(NotificationSource source) {
		NotificationMapping mapping = getMappingWithSource(source);
		if (mapping == null)
			return new ArrayList<NotificationSink>();
		return mapping.getSinks();
	}

	public static NotificationMapping getMappingWithSource(NotificationSource source) {
		for (NotificationMapping mapping : mappings)
			if (mapping.getSource() == source)
				return mapping;
		return null;
	}
	
	public static ArrayList<NotificationMapping> getMappings() {
		return mappings;
	}
}
