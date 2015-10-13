package fr.aviz.tokennotifier.tests;

import java.awt.BorderLayout;

import javax.swing.Icon;
import javax.swing.ImageIcon;
import javax.swing.JFrame;

import com.explodingpixels.macwidgets.HudWindow;
import com.explodingpixels.macwidgets.SourceList;
import com.explodingpixels.macwidgets.SourceListCategory;
import com.explodingpixels.macwidgets.SourceListItem;
import com.explodingpixels.macwidgets.SourceListModel;

public class macWidgetsTests {

	static final Icon customFolderIcon = new ImageIcon("img/smart_folder_20.png");
	static final Icon gmailIcon = new ImageIcon("img/gmail_20.png");
	static final Icon inboxIcon = new ImageIcon("img/inbox_darker_20.png");
	static final Icon gmailFolderIcon = new ImageIcon("img/gray_folder_20.png");
	
	public static void main(String[] args) {
		HudWindow hud = new HudWindow("Window");
		hud.getJDialog().setSize(300, 350);
		hud.getJDialog().setLocationRelativeTo(null);
		hud.getJDialog().setDefaultCloseOperation(JFrame.DISPOSE_ON_CLOSE);
				
		SourceListModel model = new SourceListModel();
		SourceListCategory category = new SourceListCategory("MAILBOXES");
		model.addCategory(category);
		model.addItemToCategory(new SourceListItem("Custom category", customFolderIcon), category);
		model.addItemToCategory(new SourceListItem("Custom category 2", customFolderIcon), category);
		model.addItemToCategory(new SourceListItem("INBOX", inboxIcon), category);
		SourceListItem it = new SourceListItem("gmail", gmailIcon);
		model.addItemToCategory(it, category);
		model.addItemToItem(new SourceListItem("draft", gmailFolderIcon), it);
		SourceList sourceList = new SourceList(model);
		//sourceList.setColorScheme(new SourceListDarkColorScheme());
		hud.getJDialog().getContentPane().add(sourceList.getComponent(), BorderLayout.CENTER);

		hud.getJDialog().setVisible(true);

//		JFrame win = new JFrame();
//		win.getContentPane().add(sourceList.getComponent(), BorderLayout.CENTER);
//		win.setVisible(true);

	}

}
