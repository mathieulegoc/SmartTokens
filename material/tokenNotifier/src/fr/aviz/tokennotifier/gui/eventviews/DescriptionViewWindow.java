package fr.aviz.tokennotifier.gui.eventviews;

import java.awt.Color;
import java.awt.Rectangle;
import java.net.URL;

import javax.swing.JEditorPane;
import javax.swing.JScrollPane;
import javax.swing.JWindow;
import javax.swing.border.EmptyBorder;
import javax.swing.text.Document;
import javax.swing.text.html.HTMLEditorKit;
import javax.swing.text.html.StyleSheet;

import fr.aviz.animation.AnimatedFloat;
import fr.aviz.animation.AnimationListener;
import fr.aviz.animation.AnimationTimer;
import fr.aviz.tokennotifier.util.GUIUtils;

public class DescriptionViewWindow extends JWindow implements AnimationListener {

	final int min_width = 100;
	final int max_width = 700;
	
	final int min_height = 0;
	final int max_height = 900;
	
	AnimatedFloat visibility = new AnimatedFloat(0);
	AnimatedFloat x = new AnimatedFloat(0);
	AnimatedFloat y = new AnimatedFloat(0);
	AnimatedFloat w = new AnimatedFloat(0);
	AnimatedFloat h = new AnimatedFloat(0);
	
	JEditorPane htmlView;
	TitlePreviewWindow child;
	String html;
	
	int full_w, full_h;
	//int min_x, min_h;
	
	public DescriptionViewWindow(String formattedDescription, TitlePreviewWindow child) {
		super();
		
		this.html = formattedDescription;
		this.child = child;
		
		htmlView = new JEditorPane();
		htmlView.setEditable(false);
		HTMLEditorKit kit = new HTMLEditorKit();
		htmlView.setEditorKit(kit);
		
		// Mimick gmail CSS
		try {
			URL url = new URL("http://weaver4.myfastmail.com/custom-google.css");
			StyleSheet styleSheet = new StyleSheet();
			styleSheet.importStyleSheet(url);
			kit.setStyleSheet(styleSheet);
		} catch (Exception e) {
			e.printStackTrace();
		}
		
		// Set html
		Document doc = kit.createDefaultDocument();
		htmlView.setDocument(doc);
		htmlView.setText(html);
		
		JScrollPane scrollPane = new JScrollPane(htmlView);
		int m = 10;
		scrollPane.setBorder(new EmptyBorder(m, m, m, m));
		scrollPane.setBackground(Color.white);
		getContentPane().add(scrollPane);
		//setAlwaysOnTop(true);
		
		pack();
		
		full_w = Math.min(max_width, Math.max(min_width, getWidth() + 20));
		full_h = Math.min(max_height, Math.max(min_height, getHeight() + 20));
		
		Rectangle b0 = child.getNormalBounds();
		x.directSet(b0.getX() + b0.getWidth());
		y.directSet(b0.getY());
		w.directSet(b0.getWidth());
		h.directSet(b0.getHeight());
		
		AnimationTimer.addListener(this);
	}
	
	public void showDescription() {
		Rectangle b0 = child.getNormalBounds();
		
		x.directSet(b0.getX());
		y.directSet(b0.getY());
		w.directSet(b0.getWidth());
		h.directSet(b0.getHeight());
		visibility.directSet(0);
		
		x.set(b0.x - full_w - 5);
		y.set(b0.y);
		w.set(full_w);
		h.set(full_h);
		visibility.set(1);
	}
	
	public void hideDescription() {
		
		Rectangle b0 = child.getNormalBounds();

		x.set(b0.getX());
		y.set(b0.getY());
		w.set(b0.getWidth());
		h.set(b0.getHeight());
		visibility.set(0);
	}
	
	public void animateEvent() {
		if (x.changed() || y.changed() || w.changed() || h.changed()) {
			setBounds(x.getInt(), y.getInt(), w.getInt(), h.getInt());
		}
		if (visibility.changed()) {
			float op = (float)visibility.get();
			setOpacity((float)Math.pow(op, 0.2));
			if (visibility.get() > 0 && !isVisible())
				setVisible(true);
			else if (visibility.get() == 0 && isVisible())
				setVisible(false);
		}
	}

}
