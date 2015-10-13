package fr.aviz.tokennotifier.sources.gmail;

import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.io.IOException;
import java.net.URL;
import java.util.ArrayList;
import java.util.Date;
import java.util.Properties;

import javax.mail.Address;
import javax.mail.BodyPart;
import javax.mail.Flags;
import javax.mail.Folder;
import javax.mail.Message;
import javax.mail.Message.RecipientType;
import javax.mail.MessagingException;
import javax.mail.Multipart;
import javax.mail.Part;
import javax.mail.Session;
import javax.mail.internet.InternetAddress;
import javax.mail.internet.MimeBodyPart;
import javax.mail.internet.MimeMultipart;
import javax.swing.JEditorPane;
import javax.swing.JFrame;
import javax.swing.JScrollPane;
import javax.swing.JWindow;
import javax.swing.SwingUtilities;
import javax.swing.Timer;
import javax.swing.text.Document;
import javax.swing.text.html.HTMLEditorKit;
import javax.swing.text.html.StyleSheet;

import com.sun.mail.imap.IMAPStore;

import fr.aviz.tokennotifier.sources.NotificationSource;
import fr.aviz.tokennotifier.sources.NotificationSourceFactory;
import fr.aviz.tokennotifier.util.ExecTime;
import fr.aviz.tokennotifier.util.GUIUtils;
import fr.aviz.tokennotifier.util.Util;

/**
 * @author dragice
 */
public class GmailReader implements NotificationSourceFactory {
	
	final String userName, password;
	final String protocol = "imap";
	final String host = "imap.gmail.com";
	final String port = "993";
	double checkEvery = 10; //in seconds

	IMAPStore store = null;
	GmailFolder[] mailFolders = null;
	
	boolean listeningEnabled = false;
	
	Thread mailPollingThread = null;

	public GmailReader(String userName, String password) {
		this.userName = userName;
		this.password = password;
	}
	
	public String getNotificationFactoryName() {
		return "MAILBOXES";
	}
		
	/**
	 * Do some initialization and return false if it fails. If it fails, the category won't show up in the window.
	 */
	@Override
	public boolean open() {
		return connect();
	}
	
	/**
	 * 
	 */
	public NotificationSource[] getAllNotificationSources() {
		if (mailFolders != null)
			return mailFolders;
		
		ArrayList<GmailFolder> sources = new ArrayList<>();
		try {
			Folder toplevel = store.getDefaultFolder();
			addNotificationSourcesToList(null, toplevel, sources, false);
		} catch (Exception e) {
			e.printStackTrace();
		}
		GmailFolder[] sources_array = new GmailFolder[sources.size()];
		for (int i=0; i<sources_array.length; i++)
			sources_array[i] = sources.get(i);
		//disconnect();
		mailFolders = sources_array;
		
		// get last email in each folder
		checkEmails_sync(1, false, false);
		
		return mailFolders;
	}
	
	private void addNotificationSourcesToList(GmailFolder parent, Folder folder, ArrayList<GmailFolder> sources, boolean includeParent) {
		GmailFolder source = new GmailFolder(this, parent, folder);
		if (includeParent)
			sources.add(source);
		try {
			Folder[] children = folder.list();
			for (Folder child : children) {
				addNotificationSourcesToList(includeParent ? source : null, child, sources, true);
			}
		} catch (Exception e) {
			e.printStackTrace();
		}
	}
	
	/**
	 * Connects to gmail.
	 * 
	 * @return true if connection succeeded.
	 */
	private boolean connect() {
		return connect(protocol, host, port, userName, password);
	}
	
	/**
	 * Disconnects from gmail.
	 */
	private void disconnect() {
		try {
			store.close();
		} catch (Exception e) {
			e.printStackTrace();
		}
	}

	private boolean connect(String protocol, String host, String port, String userName, String password) {
		Properties properties = getServerProperties(protocol, host, port);
		Session session = Session.getDefaultInstance(properties);
		try {
			store = (IMAPStore)session.getStore(protocol);
			store.connect(userName, password);
			return true;
		} catch (Exception e) {
			e.printStackTrace();
			return false;
		}
	}
	
	private static Properties getServerProperties(String protocol, String host, String port) {
		Properties properties = new Properties();
		properties.put(String.format("mail.%s.host", protocol), host);
		properties.put(String.format("mail.%s.port", protocol), port);
		properties.setProperty(String.format("mail.%s.socketFactory.class", protocol), "javax.net.ssl.SSLSocketFactory");
		properties.setProperty(String.format("mail.%s.socketFactory.fallback", protocol), "false");
		properties.setProperty(String.format("mail.%s.socketFactory.port", protocol), String.valueOf(port));
		return properties;
	}
	
	public void setEmailCheckingFrequency(double freq) {
		this.checkEvery = freq;
	}
		
	// Figure out whether listening should be enabled
	void updateListeningEnabled() {
		boolean enable = false;
		for (GmailFolder folder : mailFolders)
			if (folder.listeningEnabled)
				enable = true;
		if (listeningEnabled == enable)
			return;
		this.listeningEnabled = enable;
		if (enable) {
			SwingUtilities.invokeLater(new Runnable() {
				@Override
				public void run() {
					startListening();
				}
			});
		} else {
			SwingUtilities.invokeLater(new Runnable() {
				@Override
				public void run() {
					stopListening();
				}
			});
		}
	}
	
	private void startListening() {
//System.err.println("-- start listening");
		
		if (mailPollingThread != null)
			return;
		
		mailPollingThread = new Thread() {
			@Override
			public void run() {
				while (mailPollingThread == this) {
					long start = System.currentTimeMillis();
					checkEmails_sync(10, true, true);
					int duration = (int)(System.currentTimeMillis() - start);
					int sleep = Math.max(500, (int)(checkEvery*1000) - duration);
					try {
						Thread.sleep(sleep);
					} catch (InterruptedException e) {
					}
				}
			}
		};
		mailPollingThread.start();
		//connect();
	}
	
	private void stopListening() {
//System.err.println("-- stop listening");
		mailPollingThread = null;
		//disconnect();
	}
	
	private synchronized void checkEmails_sync(int maxnum, boolean activeFoldersOnly, boolean notify) {
		for (GmailFolder folder : mailFolders) {
			if (!activeFoldersOnly || folder.listeningEnabled)
				folder.checkEmails(maxnum, notify);
		}
	}
	
	
    /**
     * Return the primary text content of the message.
     * 
     * Code from http://www.oracle.com/technetwork/java/javamail/faq/index.html
     */
    static String getText(Part p) throws MessagingException, IOException {
    	boolean textIsHtml = false;
        if (p.isMimeType("text/*")) {
            String s = (String)p.getContent();
            textIsHtml = p.isMimeType("text/html");
            return s;
        }
        if (p.isMimeType("multipart/alternative")) {
            // prefer html text over plain text
            Multipart mp = (Multipart)p.getContent();
            String text = null;
            for (int i = 0; i < mp.getCount(); i++) {
                Part bp = mp.getBodyPart(i);
                if (bp.isMimeType("text/plain")) {
                    if (text == null)
                        text = getText(bp);
                    continue;
                } else if (bp.isMimeType("text/html")) {
                    String s = getText(bp);
                    if (s != null)
                        return s;
                } else {
                    return getText(bp);
                }
            }
            return text;
        } else if (p.isMimeType("multipart/*")) {
            Multipart mp = (Multipart)p.getContent();
            for (int i = 0; i < mp.getCount(); i++) {
                String s = getText(mp.getBodyPart(i));
                if (s != null)
                    return s;
            }
        }
        return null;
    }
	
	@Override
	public void close() {
		disconnect();
	}
}
