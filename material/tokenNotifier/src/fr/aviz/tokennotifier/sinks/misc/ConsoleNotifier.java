package fr.aviz.tokennotifier.sinks.misc;

import java.awt.BorderLayout;
import java.awt.Container;
import java.awt.Window;

import javax.swing.BoxLayout;
import javax.swing.Icon;
import javax.swing.ImageIcon;
import javax.swing.JFrame;
import javax.swing.JLabel;

import com.explodingpixels.macwidgets.HudWidgetFactory;
import com.explodingpixels.macwidgets.HudWindow;
import com.explodingpixels.macwidgets.SourceListItem;

import fr.aviz.tokennotifier.mapping.NotificationEvent;
import fr.aviz.tokennotifier.sinks.*;
import fr.aviz.tokennotifier.util.Util;

public class ConsoleNotifier extends NotificationSink {
	
	static final Icon consoleIcon = new ImageIcon("img/console_20.png");

	static HudWindow win = null;
	static Container contentPane = null;
	static Window parentWindow = null;
	
	public ConsoleNotifier() {
		super(null);
	}
	
	public static void addToConsole(String message) {
		if (win == null) {
			win = new HudWindow("Notifications");
			contentPane = win.getContentPane();
			contentPane.setLayout(new BoxLayout(contentPane, BoxLayout.Y_AXIS));
			win.getJDialog().setAlwaysOnTop(true);
			win.getJDialog().setSize(150, 350);
			//win.getJDialog().setLocationRelativeTo(null);
			if (parentWindow != null) {
				Util.placeWindowLeft(win.getJDialog(), parentWindow);
				win.getJDialog().setSize(150, parentWindow.getHeight());
			}
		}
		if (!win.getJDialog().isVisible())
			win.getJDialog().setVisible(true);
		//
		JLabel newlabel = HudWidgetFactory.createHudLabel(" " + message);
		contentPane.add(newlabel);
		contentPane.revalidate();
	}
	
	public static void setParentWindow(Window win) {
		parentWindow = win;
	}
	
	@Override
	public SourceListItem createListItem() {
		return new SourceListItem("Console", consoleIcon);
	}
	
	@Override
	public boolean open() {
		return true;
	}

	@Override
	public void eventReceived(NotificationEvent event) {
		addToConsole("#" + event.getID() + " " + event.getTitle());
	}

	@Override
	public void close() {
	}

}
