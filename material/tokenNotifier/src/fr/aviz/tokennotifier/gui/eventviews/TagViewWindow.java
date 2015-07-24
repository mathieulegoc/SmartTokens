package fr.aviz.tokennotifier.gui.eventviews;

import java.awt.Color;
import java.awt.Frame;
import java.awt.Graphics;
import java.awt.Graphics2D;
import java.awt.GraphicsConfiguration;
import java.awt.Image;
import java.awt.RenderingHints;
import java.awt.Window;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;

import javax.swing.ImageIcon;
import javax.swing.JComponent;
import javax.swing.JWindow;

import fr.aviz.animation.AnimatedFloat;
import fr.aviz.animation.AnimationListener;
import fr.aviz.animation.AnimationTimer;

public class TagViewWindow extends JWindow implements AnimationListener {

	static class JImg extends JComponent {
		TagViewWindow parent;
		JImg(TagViewWindow parent) {
			super();
			setBackground(new Color(1f, 1f, 1f, 0f));
			this.parent = parent;
		}
		@Override
		public void paint(Graphics g) {
			parent.paintIcon(g);
		}
	}
	
	Image icon;
	Image icon_small;
	DescriptionViewWindow descWin;
	TitlePreviewWindow titleWin;
	
	final double minSize = 20;
	final double maxSize = 100;
	AnimatedFloat size = new AnimatedFloat(minSize);
	AnimatedFloat visibility = new AnimatedFloat(0);
	
	public TagViewWindow(Image icon, Image icon_small, boolean visible, DescriptionViewWindow descWin, TitlePreviewWindow titleWin) {
		
		super();
		
		this.icon = icon;
		this.icon_small = icon_small;
		this.descWin = descWin;
		this.titleWin = titleWin;
		
		size.setIncrement(AnimatedFloat.DEFAULT_QUADRATIC_INCREMENT * 0.75, AnimatedFloat.DEFAULT_LINEAR_INCREMENT * 0.75);
		visibility.setIncrement(AnimatedFloat.DEFAULT_QUADRATIC_INCREMENT * 0.75, AnimatedFloat.DEFAULT_LINEAR_INCREMENT * 0.75);
		
		if (visible) {
			size.directSet(minSize);
			visibility.directSet(1);
		} else {
			size.directSet(maxSize);
			visibility.directSet(0);
		}
		
		setContentPane(new JImg(this));
		setBackground(new Color(1f, 1f, 1f, 0f));
		
		setAlwaysOnTop(true);
		
		AnimationTimer.addListener(this);
	}
	
	public void makeAppear() {
		size.set(minSize);
		visibility.set(1);
	}
	
	public void makeDisappear() {
		size.set(maxSize);
		visibility.set(0);
	}

	public void paintIcon(Graphics g) {
		
		final int m = 3;
		final int dy = -1;
		
		int w = getWidth();
		int h = getHeight();
//		g.setColor(Color.white);
//		g.fillRect(0, 0, w, h);
		
		Graphics2D g2 = (Graphics2D)g;
		g2.setRenderingHint(RenderingHints.KEY_INTERPOLATION, RenderingHints.VALUE_INTERPOLATION_BILINEAR);
		g2.setRenderingHint(RenderingHints.KEY_ANTIALIASING, RenderingHints.VALUE_ANTIALIAS_ON);
		g2.setRenderingHint(RenderingHints.KEY_RENDERING, RenderingHints.VALUE_RENDER_QUALITY);
		
		Image img = icon;
		if (icon_small.getWidth(null) == w-m*2)
			img = icon_small;
		
		g.drawImage(img, m, m + dy, w-m*2, h-m*2, null);
	}
	
	@Override
	public void animateEvent() {
		if (
			visibility.changed() || titleWin.visibility.changed() || descWin.visibility.changed()
			|| size.changed() || titleWin.position.changed() || descWin.x.changed() || descWin.y.changed()) {
			
			// Update position
			final int margin = 0;
			int xmid = Math.min(titleWin.getX(), descWin.getX()) - (int)minSize/2 - margin;
			int ymid = titleWin.getY() + (int)minSize/2;
			int s = size.getInt();
			int xoffset = - (int)((s - minSize) * 0.5);
			setBounds(xmid - s/2 + xoffset, ymid - s/2, s, s);

			// Update opacity
			double opacity = visibility.get() * Math.max(titleWin.visibility.get(), descWin.visibility.get());
			setOpacity((float)opacity);
			if (opacity == 0 && isVisible())
				setVisible(false);
			if (opacity > 0 && !isVisible())
				setVisible(true);			
		}
	}


}
