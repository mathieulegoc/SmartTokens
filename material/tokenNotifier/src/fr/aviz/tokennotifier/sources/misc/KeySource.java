package fr.aviz.tokennotifier.sources.misc;

import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.awt.event.KeyEvent;

import javax.swing.Icon;
import javax.swing.ImageIcon;

import com.explodingpixels.macwidgets.SourceListItem;

import fr.aviz.tokennotifier.mapping.NotificationEvent;
import fr.aviz.tokennotifier.mapping.NotificationSystem;
import fr.aviz.tokennotifier.sources.NotificationSource;
import fr.aviz.tokennotifier.util.GUIUtils;

public class KeySource extends NotificationSource implements ActionListener {

	static final Icon[] icons = new Icon[] {
		new ImageIcon("img/1-20.png"),
		new ImageIcon("img/2-20.png"),
		new ImageIcon("img/3-20.png"),
		new ImageIcon("img/4-20.png"),
	};
	
	int num;
	
	public KeySource(int num) {
		super(null);
		this.num = num;
	}
	
	boolean listenerAdded = false;
	@Override
	public boolean open() {
		if (listenerAdded) return true;
		listenerAdded = true;
		GUIUtils.addGlobalKeyListener(KeyEvent.VK_1 + (num - 1), this, "");
		return true;
	}

	@Override
	public SourceListItem createListItem() {
		return new SourceListItem("Key " + num, icons[num - 1]);
	}
	
	@Override
	public void setListeningEnabled(boolean enabled) {
		// don't bother
	}
	
	public void actionPerformed(ActionEvent e) {
		String title = "Key " + num + " pressed";
		String htmlTitle = "<html>Key <b>" + num + "</b> pressed</html>";
		String htmlDescription = "<html><h2>Message from KeySource:</h2><p>Key <b>" + title + "</b> has been pressed.</p></html>";
		NotificationEvent event = new NotificationEvent(this, title, htmlTitle, htmlDescription);
		NotificationSystem.sendToSinks(event);
	}

	@Override
	public void close() {
		// don't bother
	}
}
