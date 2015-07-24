package fr.aviz.tokennotifier;

import javax.swing.SwingUtilities;

import fr.aviz.tokennotifier.gui.MainWindow;
import fr.aviz.tokennotifier.sinks.misc.MiscNotifiers;
import fr.aviz.tokennotifier.sinks.misc.ConsoleNotifier;
import fr.aviz.tokennotifier.sinks.tokens.TokenSystem;
import fr.aviz.tokennotifier.sources.gmail.GmailReader;
import fr.aviz.tokennotifier.sources.misc.MiscSources;
import fr.aviz.tokennotifier.util.Util;

public class NotifierMain {

	/**
	 * HOW TO USE WITH YOUR OWN GMAIL ACCOUNT:
	 * 
	 * 1 - Create a new gmail account, or use a gmail account that's not important to you
	 * 2 - Activate imap from gmail:
	 *     Settings -> Forwarding and POP/IMAP -> IMAP Access -> Enable IMAP
	 * 3 - Deactivate secure sign-in technology:
	 *     Go to https://www.google.com/settings/security/lesssecureapps and turn on "Access for less secure apps"
	 * 4 - Update user name and password below
	 */

	// Your gmail account credentials. See above.
	final static String userName = "WRITE YOUR USERNAME HERE";
	final static String password = "AND PASSWORD AND UNCOMMENT THE SEMI-COLUMN TO BE ABLE TO USE THE NOTIFIER"//;
	
	// Email checking frequency, in seconds
	final static double checkEmailsEvery = 3.0; // seconds
		
	public static final boolean DEBUG_EVENT_VIEW = true;

	public NotifierMain() {
					
		// Setup notification sources
		GmailReader mailReader = new GmailReader(userName, password);
		mailReader.setEmailCheckingFrequency(checkEmailsEvery);
		MiscSources miscSources = new MiscSources();
		
		// Setup notificttttation sinks
		TokenSystem tokenNotifiers = new TokenSystem();
		MiscNotifiers miscNotifiers = new MiscNotifiers();
		
		// Create main window
		final MainWindow mainWindow = new MainWindow();
		mainWindow.addSources(mailReader);
		mainWindow.addSources(miscSources);
		mainWindow.addSinks(tokenNotifiers);
		mainWindow.addSinks(miscNotifiers);

		///
		Util.placeWindow(mainWindow, 1.0, 0.0);
		ConsoleNotifier.setParentWindow(mainWindow);
		mainWindow.setVisible(true);
		
		SwingUtilities.invokeLater(new Runnable() {
			@Override
			public void run() {
				mainWindow.setup();
			}
		});

	}
	
	public static void main(String[] args) {
		new NotifierMain();
	}

}
