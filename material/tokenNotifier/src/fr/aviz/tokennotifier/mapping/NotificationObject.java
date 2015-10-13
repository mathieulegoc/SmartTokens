package fr.aviz.tokennotifier.mapping;

import com.explodingpixels.macwidgets.SourceListItem;

public abstract class NotificationObject {
	
	private boolean attemptedOpen = false; 
	private boolean open = false;
	
	public void doOpen() {
		attemptedOpen = true;
		open = open();
	}
	
	public boolean isOpen() {
		return open;
	}
	
	public boolean attemptedOpen() {
		return attemptedOpen;
	}
	
	public boolean openFailed() {
		return attemptedOpen && !open;
	}

	public abstract SourceListItem createListItem();

	public abstract boolean open();
	
	public abstract void close();

}
