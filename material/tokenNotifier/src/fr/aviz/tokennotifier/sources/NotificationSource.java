package fr.aviz.tokennotifier.sources;

import java.util.Hashtable;

import com.explodingpixels.macwidgets.SourceListItem;

import fr.aviz.tokennotifier.mapping.NotificationObject;

public abstract class NotificationSource extends NotificationObject {

	final static private Hashtable<SourceListItem, NotificationSource> itemToSource = new Hashtable<SourceListItem, NotificationSource>();
	
	private final NotificationSource parent;
	SourceListItem listItem = null;
	
	public NotificationSource(NotificationSource parent) {
		this.parent = parent;
	}

	public NotificationSource getParent() {
		return parent;
	}

	public SourceListItem getListItem() {
		if (listItem == null) {
			listItem = createListItem();
			itemToSource.put(listItem, this);
		}
		return listItem;
	}
	
	public static NotificationSource getNotificationSourceForItem(SourceListItem item) {
		return itemToSource.get(item);
	}
	
	public abstract void setListeningEnabled(boolean enabled);

}
