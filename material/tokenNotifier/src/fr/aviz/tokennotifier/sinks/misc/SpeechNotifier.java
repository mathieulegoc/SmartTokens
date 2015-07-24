package fr.aviz.tokennotifier.sinks.misc;

import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.awt.event.KeyEvent;

import javax.swing.Icon;
import javax.swing.ImageIcon;

import com.explodingpixels.macwidgets.SourceListItem;

import fr.aviz.tokennotifier.mapping.NotificationEvent;
import fr.aviz.tokennotifier.mapping.NotificationSystem;
import fr.aviz.tokennotifier.sinks.NotificationSink;
import fr.aviz.tokennotifier.sources.NotificationSource;
import fr.aviz.tokennotifier.util.GUIUtils;

public class SpeechNotifier extends NotificationSink {

	static final Icon icon = new ImageIcon("img/speech-20.png");
	
	public SpeechNotifier() {
		super(null);
	}

	@Override
	public SourceListItem createListItem() {
		return new SourceListItem("Say ", icon);
	}	
	
	@Override
	public boolean open() {
		// The "say" command is usually only available on Mac
		return (System.getProperty("os.name").toLowerCase().contains("mac"));
	}

	@Override
	public void eventReceived(NotificationEvent event) {
		try {
			Runtime.getRuntime().exec("say \"" + event.getTitle() + "\"");
		} catch (Exception e) {
			e.printStackTrace();
		}
	}
	
	@Override
	public void close() {
	}

}
