package fr.aviz.tokennotifier.sinks.tokens;

import java.util.Hashtable;

import javax.swing.ImageIcon;

import fr.aviz.sensingtokens.TokenEvent;
import fr.aviz.sensingtokens.TokenEventListener;
import fr.aviz.sensingtokens.TokenSocketClient;
import fr.aviz.sensingtokens.Transitions;
import fr.aviz.tokenmachine.TokenMachine;
import fr.aviz.tokennotifier.gui.eventviews.EventStack;
import fr.aviz.tokennotifier.mapping.NotificationEvent;
import fr.aviz.tokennotifier.sinks.NotificationSink;
import fr.aviz.tokennotifier.sinks.NotificationSinkFactory;

/**
 * Represents all token dispensers.
 * 
 * @author dragice
 *
 */
public class TokenSystem implements NotificationSinkFactory, TokenEventListener {

    public static final int TOKEN_ASSOCIATION_TIMEOUT = 10000; // milliseconds

    /**
     * Icons for token dispensers, as they will appear in the right panel.
     */
    static final ImageIcon[] tokenIcons = new ImageIcon[] { new ImageIcon("img/puck-blue.png"),
	    new ImageIcon("img/puck-green.png"), new ImageIcon("img/sphere-red.png") };

    // Token Communication
    private TokenSocketClient socket = null;
    private EventStack notificationStack = null;
    NotificationSink[] sinks = null;
    private TokenMachine machine = null;
    private NotificationEvent lastNotificationEvent = null;
    long timeLastNotificationEvent = 0;
    private EventStack eventStack = new EventStack();
    private Hashtable<Byte, NotificationEvent> byteToNotificationEvent = new Hashtable<Byte, NotificationEvent>();

    /**
     * Returns the category name that will appear in the right panel.
     */
    @Override
    public String getNotificationFactoryName() {
	return "TOKENS";
    }

    public void DropToken(int id) {
	machine.DropToken(id);
    }

    public void stack(NotificationEvent e) {
	eventStack.stack(e);
    }

    /**
     * Called right after this class is instantiated. Do some optional
     * initialization here and return false if it fails.
     * 
     * If open fails, this category won't show up in the window and none of the
     * methods below will be called. So return false for example if you're not
     * able to list sinks. This method should not call open() on the sinks. This
     * is done automatically, when they're connected for the first time.
     */
    @Override
    public boolean open() {
	machine = new TokenMachine();
	socket = new TokenSocketClient();
	socket.start();
	socket.addTokenEventListener(this);
	notificationStack = new EventStack();
	return true;
    }

    /**
     * Create and return all available token dispensers.
     */
    @Override
    public NotificationSink[] getAllNotificationSinks() {
	if (sinks != null)
	    return sinks; // do not create sink objects more than once!

	// Create three sinks
	sinks = new NotificationSink[] { new TokenDispenser(1, tokenIcons[0], this.machine, this, notificationStack),
		new TokenDispenser(2, tokenIcons[1], this.machine, this, notificationStack),
		new TokenDispenser(3, tokenIcons[2], this.machine, this, notificationStack) };

	return sinks;
    }

    /**
     * Called when the application quits, in case open() returned true before.
     * 
     * This method should not call close() on the sinks. This is done
     * automatically. This method may not be called if the application is
     * stopped by other means than closing the window.
     */
    @Override
    public void close() {
	if (socket != null) {
	    if (socket.isConnected())
		socket.Close();
	}
	if (machine.isConnected())
	    machine.Close();
    }

    void setLastNotificationEvent(NotificationEvent e) {
	lastNotificationEvent = e;
	timeLastNotificationEvent = System.currentTimeMillis();
    }

    @Override
    public void tokenEventReceived(TokenEvent e) {
	byte tokenId = e.getTokenId();
	NotificationEvent notificationEvent = null;
	if (byteToNotificationEvent.containsKey(tokenId))
	    notificationEvent = byteToNotificationEvent.get(tokenId);

	if (e.getTransition() == Transitions.FALL) {
	    if (notificationEvent == null) {
		if (lastNotificationEvent != null
			&& System.currentTimeMillis() - timeLastNotificationEvent < TOKEN_ASSOCIATION_TIMEOUT) {
		    byteToNotificationEvent.put(tokenId, lastNotificationEvent);
		    System.out.println("Token#" + tokenId + " associated with notification "
			    + lastNotificationEvent.getTitle());

		    eventStack.stack(byteToNotificationEvent.get(tokenId));

		    lastNotificationEvent = null;
		} else {
		    System.err
			    .println("**** ERROR: no notification event can be associated to token " + e.getTokenId());
		}
	    } else {
		// Falls again
	    }
	}
	else if (e.getTransition() == Transitions.TOUCH) {
	    if (notificationEvent == null) {
		System.err.println("**** ERROR: touch received on unmapped token " + e.getTokenId());
	    } else {
		eventStack.showTitle(notificationEvent);
		eventStack.hideDescription(notificationEvent);
	    }
	}
	else if (e.getTransition() == Transitions.STILL_RELEASE) {
	    if (notificationEvent == null) {
		System.err.println("**** ERROR: release received on unmapped token " + e.getTokenId());
	    } else {
		eventStack.hideTitle(notificationEvent);
		eventStack.hideDescription(notificationEvent);
	    }
	}
	else if (e.getTransition() == Transitions.THROW) {
	    if (notificationEvent == null) {
		System.err.println("**** ERROR: throw received on unmapped token " + e.getTokenId());
	    } else {
//		System.out.println("Token#" + e.getTokenId() + " disassociated with notification "
//			+ notificationEvent.getTitle());
		eventStack.hideTitle(notificationEvent);
		eventStack.hideDescription(notificationEvent);
//		eventStack.unstack(notificationEvent);
//		byteToNotificationEvent.remove(tokenId);
	    }
	}
	else if (e.getTransition() == Transitions.MOVE) {
	    if (notificationEvent == null) {
		System.err.println("**** ERROR: move received on unmapped token " + e.getTokenId());
	    } else {
		eventStack.markRead(notificationEvent);
		System.out.println("Token#" + e.getTokenId() + " moved " + notificationEvent.getTitle());
		eventStack.showDescription(notificationEvent);
	    }
	}
	else if (e.getTransition() == Transitions.MOVING_GRIP_CHANGED && e.getGrip()>0x30) {
	    if (notificationEvent == null) {
		System.err.println("**** ERROR: moving received on unmapped token " + e.getTokenId());
	    } else {
		System.out.println(notificationEvent.getTitle()+" marked as important");
		eventStack.markImportant(notificationEvent);
	    }
	}
    }
}
