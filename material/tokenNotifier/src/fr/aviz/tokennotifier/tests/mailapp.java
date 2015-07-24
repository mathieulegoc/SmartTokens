package fr.aviz.tokennotifier.tests;

/**
 * @author dragice
 *
 * Code adapted from http://www.developer.com/java/data/monitoring-email-accounts-imap-in-java.html
 */
public class mailapp {
	
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

	// Your account credentials. See above.
	final static String userName = "physvis";
	final static String password = "q2@w3#e4$";
	
	// Settings for imap through gmail
	final static String imapHost = "imap.gmail.com";
	final static String port = "993";

	public static void main(String[] args){
		MailBean bean=new MailBean();
		if (bean.connect("imap", imapHost, port, userName, password)) {
			bean.displayInfo();
		}
	}

}