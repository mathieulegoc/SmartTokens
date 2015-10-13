package fr.aviz.tokennotifier.sources.gmail;

import java.util.ArrayList;
import java.util.Date;

import javax.mail.Address;
import javax.mail.Folder;
import javax.mail.Message;
import javax.mail.internet.InternetAddress;
import javax.swing.Icon;
import javax.swing.ImageIcon;
import javax.swing.SwingUtilities;

import com.explodingpixels.macwidgets.SourceListItem;

import fr.aviz.tokennotifier.mapping.NotificationEvent;
import fr.aviz.tokennotifier.mapping.NotificationSystem;
import fr.aviz.tokennotifier.sources.NotificationSource;

public class GmailFolder extends NotificationSource {

	static final Icon customFolderIcon = new ImageIcon("img/smart_folder_20.png");
	static final Icon gmailIcon = new ImageIcon("img/gmail_20.png");
	static final Icon inboxIcon = new ImageIcon("img/inbox_darker_20.png");
	static final Icon gmailFolderIcon = new ImageIcon("img/gray_folder_20.png");
	static final Icon defaultIcon = new ImageIcon("img/smart_folder_20.png");

	final Folder mailFolder;
	final GmailReader reader;
	ArrayList<Message> lastMessages = new ArrayList<Message>();
	Date lastMessageDate = null;
	
	boolean listeningEnabled = false;
		
	public GmailFolder(GmailReader reader, GmailFolder parent, Folder mailFolder) {
		super(parent);
		this.reader = reader;
		this.mailFolder = mailFolder;
	}

	@Override
	public SourceListItem createListItem() {
		String name = mailFolder.getName();
		Icon icon = defaultIcon;
		try {
			String parentname = (mailFolder.getParent() == null) ? "" : mailFolder.getParent().getName(); 
			if (name.equalsIgnoreCase("inbox"))
				icon = inboxIcon;
			else if (name.equalsIgnoreCase("[gmail]"))
				icon = gmailIcon;
			else if (parentname.equalsIgnoreCase("[gmail]"))
				icon = gmailFolderIcon;
			else if (parentname.equals(""))
				icon = customFolderIcon;
		} catch (Exception e) {
			e.printStackTrace();
			return null;
		}
		return new SourceListItem(mailFolder.getName(), icon);
	}
	
	@Override
	public boolean open() {
		return true;
	}
	
	@Override
	public void setListeningEnabled(boolean enabled) {
		listeningEnabled = enabled;
		reader.updateListeningEnabled();
	}

	void checkEmails(int maxnum, boolean notify) {
		try {
			if (!mailFolder.exists()) return;
			if ((mailFolder.getType() & Folder.HOLDS_MESSAGES) == 0) return;
			mailFolder.open(Folder.READ_ONLY);
			int totalcount = mailFolder.getMessageCount();
//			System.err.println(mailFolder + " " + count);
			int count =  Math.min(maxnum, totalcount);
			// get "count" last messages
			int end = totalcount;
            int start = end - count + 1;
			Message[] messages = mailFolder.getMessages(start, end);
			addMessages(messages, notify);
			mailFolder.close(false);
		} catch (Exception e) {
			e.printStackTrace();
		}
	}
	
	void addMessages(Message[] messages, boolean notify) {
		
		Date maxDate = lastMessageDate;
		
		try {
//System.err.println("\n" + mailFolder.getName());
			for (Message message : messages) {
				Date date = message.getReceivedDate();
				boolean after = (lastMessageDate == null) || date.after(lastMessageDate);
				if (after) {
					lastMessages.add(message);
//System.err.println("ADDED MESSAGE: " + message.getSubject() + " (" + date + ")");
					if (notify) {
						
						Address[] froms = message.getFrom();
						String sender = froms == null ? "" : ((InternetAddress)froms[0]).getPersonal();
						String subject = message.getSubject();
						String title = sender + ": " + subject;
						String htmlTitle = "<html><nobr><b>" + sender + "</b>: <span style=\"font-weight: normal;\">" + subject + "</span></nobr></html>";
						String htmlBody = GmailReader.getText(message);
						final NotificationEvent event = new NotificationEvent(this, title, htmlTitle, htmlBody);
						SwingUtilities.invokeLater(new Runnable() {
							@Override
							public void run() {
								NotificationSystem.sendToSinks(event);
							}
						});
					}
					if (maxDate == null || date.after(maxDate))
						maxDate = date;
				}
			}
			lastMessageDate = maxDate;
//System.err.println();
		} catch (Exception e) {
			e.printStackTrace();
		}		
	}
	
	@Override
	public void close() {
		
	}

}
