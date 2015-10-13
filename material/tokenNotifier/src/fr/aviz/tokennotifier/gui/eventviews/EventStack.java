package fr.aviz.tokennotifier.gui.eventviews;

import java.awt.Container;
import java.awt.Image;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.awt.event.KeyEvent;
import java.io.File;
import java.util.ArrayList;
import java.util.Hashtable;

import javax.imageio.ImageIO;
import javax.swing.DefaultListModel;
import javax.swing.ImageIcon;
import javax.swing.JFrame;
import javax.swing.JList;

import fr.aviz.tokennotifier.mapping.NotificationEvent;
import fr.aviz.tokennotifier.util.GUIUtils;

public class EventStack {

	final static int UNREAD_TAG = 0;
	final static Image unreadIcon = loadImage("img/unread.png");
	final static Image unreadIcon_small = loadImage("img/unread_14.png");
	final static int IMPORTANT_TAG = 1;
	final static Image importantIcon = loadImage("img/important.png");
	final static Image importantIcon_small = loadImage("img/important_14.png");
	
	static Image loadImage(String filename) {
		try {
			return ImageIO.read(new File(filename));
		} catch (Exception e) {
			return null;
		}
	}
	
	static class EventState {
		TitlePreviewWindow titleWindow;
		DescriptionViewWindow descriptionWindow;
		ArrayList<TagViewWindow> tagWindows = new ArrayList<TagViewWindow>();
		boolean seen = false;
	}

	ArrayList<NotificationEvent> stackedEvents = new ArrayList<NotificationEvent>();
	Hashtable<NotificationEvent, EventState> eventStates = new Hashtable<NotificationEvent, EventState>();
	int lastUsedRow = -1;
	
	public EventStack() {
	}
	
	public void stack(NotificationEvent e) {
		if (stackedEvents.contains(e)) return;
		EventState state = new EventState();
		lastUsedRow++;
		state.titleWindow = new TitlePreviewWindow(e.getFormattedTitle(), lastUsedRow);
		state.descriptionWindow = new DescriptionViewWindow(e.getFormattedDescription(), state.titleWindow);
		state.tagWindows.add(new TagViewWindow(unreadIcon, unreadIcon_small, true, state.descriptionWindow, state.titleWindow));
		state.tagWindows.add(new TagViewWindow(importantIcon, importantIcon_small, false, state.descriptionWindow, state.titleWindow));
		stackedEvents.add(e);
		eventStates.put(e, state);
	}
	
	public NotificationEvent getNotificationEventWithID(int ID) {
		for (NotificationEvent ev : stackedEvents) {
			if (ev.getID() == ID)
				return ev;
		}
		return null;
	}
	
	public void showTitle(NotificationEvent e) {
		if (!stackedEvents.contains(e)) return;
		EventState state = eventStates.get(e);
		state.titleWindow.showTitle();
		if (!state.seen) {
			state.titleWindow.enterTitle();
			state.seen = true;
		}
	}
	
	public void hideTitle(NotificationEvent e) {
		if (!stackedEvents.contains(e)) return;
		EventState state = eventStates.get(e);
		state.titleWindow.hideTitle();
	}
	
	public void showDescription(NotificationEvent e) {
		if (!stackedEvents.contains(e)) return;
		EventState state = eventStates.get(e);
		state.descriptionWindow.showDescription();
		if (!state.seen) {
			state.seen = true;
		}
	}
	
	public void hideDescription(NotificationEvent e) {
		if (!stackedEvents.contains(e)) return;
		EventState state = eventStates.get(e);
		state.descriptionWindow.hideDescription();
	}

	public void setTagEnabled(NotificationEvent e, int tagIndex, boolean enabled) {
		if (!stackedEvents.contains(e)) return;
		EventState state = eventStates.get(e);
		if (enabled)
			state.tagWindows.get(tagIndex).makeAppear();
		else
			state.tagWindows.get(tagIndex).makeDisappear();
	}

	public void markRead(NotificationEvent e) {
		setTagEnabled(e, UNREAD_TAG, false);
	}

	public void markImportant(NotificationEvent e) {
		setTagEnabled(e, IMPORTANT_TAG, true);
	}

	public void unstack(NotificationEvent e) {
		if (!stackedEvents.contains(e)) return;
		EventState state = eventStates.get(e);

		// offset title windows below
		int removedRow = state.titleWindow.getRow();
		for (NotificationEvent ev : stackedEvents) {
			TitlePreviewWindow win = eventStates.get(ev).titleWindow;
			if (win.getRow() > removedRow)
				win.setRow(win.getRow() - 1);
		}
		
		state.titleWindow.exitTitle();
		state.titleWindow.hideTitle();
		eventStates.remove(e);
		stackedEvents.remove(e);
	}
	
	//////////////////////////////////////////////////////////
	
	public static EventStack debugStack = null;
	static JList<String> debugList = null;
	static DefaultListModel<String> debugListModel = null;
	
	private static NotificationEvent getSelectedDebugEvent() {
		String s = debugList.getSelectedValue();
		if (s == null) return null;
		int id = Integer.parseInt(s);
		return debugStack.getNotificationEventWithID(id);
	}
	
	public static void addDebugEvent(NotificationEvent e) {
		if (debugStack == null)
			createDebugWindow();

		debugStack.stack(e);
		debugListModel.addElement(""+e.getID());
		debugList.repaint();
	}
	
	private static void createDebugWindow() {
		
		debugStack = new EventStack();

		JFrame win = new JFrame();
		Container c = win.getContentPane();
		debugListModel = new DefaultListModel<String>();

		debugList = new JList<String>(debugListModel);
		c.add(debugList);
		win.setSize(100, 400);
		win.setVisible(true);
		
		GUIUtils.addAdvancedKeyListener(null, new GUIUtils.AdvancedKeyListener() {						
			@Override
			public void keyPressedOnce(KeyEvent e) {
				if (e.getKeyCode() == KeyEvent.VK_T) {
					NotificationEvent ev = getSelectedDebugEvent();
					if (ev == null) return;
					debugStack.showTitle(ev);
				}
				if (e.getKeyCode() == KeyEvent.VK_M) {
					NotificationEvent ev = getSelectedDebugEvent();
					if (ev == null) return;
					debugStack.showDescription(ev);
				}
				if (e.getKeyCode() == KeyEvent.VK_R) {
					NotificationEvent ev = getSelectedDebugEvent();
					if (ev == null) return;
					debugStack.markRead(ev);
				}
				if (e.getKeyCode() == KeyEvent.VK_D) {
					NotificationEvent ev = getSelectedDebugEvent();
					if (ev == null) return;
					debugStack.unstack(ev);
				}
			}

			@Override
			public void keyReleased(KeyEvent e) {
				if (e.getKeyCode() == KeyEvent.VK_T) {
					NotificationEvent ev = getSelectedDebugEvent();
					if (ev == null) return;
					debugStack.hideTitle(ev);
				}
				if (e.getKeyCode() == KeyEvent.VK_M) {
					NotificationEvent ev = getSelectedDebugEvent();
					if (ev == null) return;
					debugStack.hideDescription(ev);
				}
				if (e.getKeyCode() == KeyEvent.VK_I) {
					NotificationEvent ev = getSelectedDebugEvent();
					if (ev == null) return;
					debugStack.markImportant(ev);
				}

			}
			
			@Override
			public void keyTyped(KeyEvent e) {
			}

			@Override
			public void keyPressed(KeyEvent e) {
			}
			
			@Override
			public void keyRepeated(KeyEvent e) {
			}
			
		}, false);
	}
}
