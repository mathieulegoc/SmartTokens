package fr.aviz.tokennotifier.mapping;

import fr.aviz.tokennotifier.sources.NotificationSource;

public class NotificationEvent {

	static int next_id = 0;
	
	private final NotificationSource source;
	private final String title;
	private final String formattedDescription;
	private final String formattedTitle;
	private final int id;
	
	public NotificationEvent(NotificationSource source, String title, String formattedTitle, String formattedDescription) {
		this.source = source;
		this.title = title;
		this.formattedDescription = formattedDescription;
		this.formattedTitle = formattedTitle;
		this.id = next_id;
		next_id++;
	}

	NotificationSource getSource() {
		return source;
	}
	
	public int getID() {
		return id;
	}

	public String getTitle() {
		return title;
	}

	public String getFormattedDescription() {
		return formattedDescription;
	}
	
	public String getFormattedTitle() {
		return formattedTitle;
	}

	public int hashCode() {
		return id;
	}

}
