package fr.aviz.tokennotifier.tests;

import java.util.Properties;
import javax.mail.Address;
import javax.mail.Flags;
import javax.mail.Folder;
import javax.mail.Message;
import javax.mail.Message.RecipientType;
import javax.mail.Session;
import javax.mail.internet.InternetAddress;

import com.sun.mail.imap.IMAPStore;

/**
 * @author dragice
 *
 * Code adapted from http://www.developer.com/java/data/monitoring-email-accounts-imap-in-java.html
 */
public class MailBean {

	IMAPStore store = null;

	private Properties getServerProperties(String protocol, String host, String port) {
		Properties properties = new Properties();
		properties.put(String.format("mail.%s.host", protocol), host);
		properties.put(String.format("mail.%s.port", protocol), port);
		properties.setProperty(String.format("mail.%s.socketFactory.class", protocol), "javax.net.ssl.SSLSocketFactory");
		properties.setProperty(String.format("mail.%s.socketFactory.fallback", protocol), "false");
		properties.setProperty(String.format("mail.%s.socketFactory.port", protocol), String.valueOf(port));
		return properties;
	}

	public boolean connect(String protocol, String host, String port, String userName, String password) {
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
	
	public void disconnect() {
		try {
			store.close();
		} catch (Exception e) {
			e.printStackTrace();
		}
	}

	/**
	 * For debugging
	 */
	public void displayInfo() {
		try {			
			System.out.println("Default folder:");
			displayFolderInfo(store.getDefaultFolder(), "  ");
			Folder[] folders;
			System.out.println("Personal namespaces:");
			folders = store.getPersonalNamespaces();
			for (Folder f : folders) displayFolderInfo(f, "  ");
			System.out.println("User namespaces:");
			folders = store.getUserNamespaces("dataphys");
			for (Folder f : folders) displayFolderInfo(f, "  ");
			System.out.println("Shared namespaces:");
			folders = store.getSharedNamespaces();
			for (Folder f : folders) displayFolderInfo(f, "  ");
		} catch (Exception e) {
			e.printStackTrace();
		}
	}
	
	private void displayFolderInfo(Folder folder, String prefix) {
		try {
			if (!folder.exists()) return;
			System.out.print(prefix);
			int count = -1;
			try {
				if (!folder.exists())
					count = -2;
				else
					count = folder.getMessageCount();
			} catch (Exception e) {} 
			System.out.print(folder.getName() + " / " + folder.getFullName() + " / " + count);
			System.out.println(" (" + folder.list().length + " children)");
			Folder[] children = folder.list();
			for(Folder f:children)
			    displayFolderInfo(f, prefix + "   ");
		} catch (Exception e) {
			e.printStackTrace();
		}
	}
	
	/**
	 * For debugging
	 */
	public void displayMessages() {

		try {
			Folder inbox = store.getFolder("INBOX");
			inbox.open(Folder.READ_WRITE);

			int count = inbox.getMessageCount();
			Message[] messages = inbox.getMessages(1, count);
			for (Message message : messages) {
				if (!message.getFlags().contains(Flags.Flag.SEEN)) {
					Address[] fromAddresses = message.getFrom();
					System.out.println("...................");
					System.out.println("\t From: " + fromAddresses[0].toString());
					System.out.println("\t To: " + parseAddresses(message.getRecipients(RecipientType.TO)));
					System.out.println("\t CC: " + parseAddresses(message.getRecipients(RecipientType.CC)));
					System.out.println("\t Subject: " + message.getSubject());
					System.out.println("\t Sent Date:" + message.getSentDate().toString());
					try {
						System.out.println(message.getContent().toString());
					} catch (Exception ex) {
						System.out.println("Error reading content!!");
						ex.printStackTrace();
					}
				}
			}

			inbox.close(false);
			store.close();
		} catch (Exception e) {
			e.printStackTrace();
		}
	}

	private String parseAddresses(Address[] address) {

		String listOfAddress = "";
		if ((address == null) || (address.length < 1))
			return null;
		if (!(address[0] instanceof InternetAddress))
			return null;

		for (int i = 0; i < address.length; i++) {
			InternetAddress internetAddress = (InternetAddress) address[0];
			listOfAddress += internetAddress.getAddress()+",";
		}
		return listOfAddress;
	}
}
