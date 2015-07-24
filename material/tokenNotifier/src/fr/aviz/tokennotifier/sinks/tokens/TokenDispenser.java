package fr.aviz.tokennotifier.sinks.tokens;

import java.util.Hashtable;

import javax.swing.ImageIcon;

import com.explodingpixels.macwidgets.SourceListItem;
import fr.aviz.tokenmachine.TokenMachine;
import fr.aviz.tokennotifier.gui.eventviews.EventStack;
import fr.aviz.tokennotifier.mapping.NotificationEvent;
import fr.aviz.tokennotifier.sinks.NotificationSink;

/**
 * Represents a single token dispenser.
 * 
 * @author dragice
 *
 */
public class TokenDispenser extends NotificationSink {

	int id;
	ImageIcon icon;
	private TokenSystem tokenSystem;
	private Hashtable<NotificationEvent, Integer> mailTokenAssociation;

	public TokenDispenser(int id, ImageIcon icon, TokenMachine machine, TokenSystem tokenSystem, EventStack stack) {
		super(null);
		this.id = id;
		this.icon = icon;
		this.tokenSystem = tokenSystem;
		this.mailTokenAssociation = new Hashtable<NotificationEvent, Integer>();
	}

	/**
	 * Creates the list item for the GUI. Called only once.
	 */
	@Override
	public SourceListItem createListItem() {
		return new SourceListItem("Tokens #" + id, icon);
	}

	/**
	 * Called when this sink is connected for the first time to a source.
	 * Perform some optional initialization here and return false if it fails.
	 * 
	 * If open fails, the connection won't be established and won't appear in
	 * the GUI. This method is called at most once, whether it fails or
	 * succeeds. It's not called if the sink is never used.
	 */
	@Override
	public boolean open() {
		return true;
	}

	/**
	 * Called every time an event (e.g., an email) has been sent to this token
	 * dispenser.
	 * 
	 * This method is not called if open() previously returned false.
	 */
	@Override
	public void eventReceived(NotificationEvent event) {
		tokenSystem.DropToken(this.id); // tells the machine to drop a token
		tokenSystem.setLastNotificationEvent(event);
	}

	/**
	 * Called when the application quits.
	 * 
	 * This method is not called if open() has never been called or has returned
	 * false. This method may not be called if the application is stopped by
	 * other means than closing the window.
	 */
	@Override
	public void close() {
	}

}
