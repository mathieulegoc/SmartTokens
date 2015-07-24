package fr.aviz.tokennotifier.sinks;

import java.util.Hashtable;

import com.explodingpixels.macwidgets.SourceListItem;

import fr.aviz.tokennotifier.mapping.NotificationEvent;
import fr.aviz.tokennotifier.mapping.NotificationObject;
import fr.aviz.tokennotifier.sources.NotificationSource;

public abstract class NotificationSink extends NotificationObject {

	final static private Hashtable<SourceListItem, NotificationSink> itemToSink = new Hashtable<SourceListItem, NotificationSink>();

	private final NotificationSink parent;
	SourceListItem listItem = null;
	
	public NotificationSink(NotificationSink parent) {
		this.parent = parent;
	}

	public NotificationSink getParent() {
		return parent;
	}

	public SourceListItem getListItem() {
		if (listItem == null) {
			listItem = createListItem();
			itemToSink.put(listItem, this);
		}
		return listItem;
	}
	
	public static NotificationSink getNotificationSinkForItem(SourceListItem item) {
		return itemToSink.get(item);
	}

	public abstract void eventReceived(NotificationEvent event);

}
