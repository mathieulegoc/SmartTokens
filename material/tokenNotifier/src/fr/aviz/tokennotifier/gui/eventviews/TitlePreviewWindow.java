package fr.aviz.tokennotifier.gui.eventviews;

import java.awt.BorderLayout;
import java.awt.Color;
import java.awt.Font;
import java.awt.Rectangle;

import javax.swing.JLabel;
import javax.swing.JWindow;
import javax.swing.border.EmptyBorder;

import fr.aviz.animation.AnimatedFloat;
import fr.aviz.animation.AnimationListener;
import fr.aviz.animation.AnimationTimer;
import fr.aviz.tokennotifier.util.Util;

public class TitlePreviewWindow extends JWindow implements AnimationListener {
	
	String title;
	AnimatedFloat visibility = new AnimatedFloat(0);
	AnimatedFloat position = new AnimatedFloat(0);

	Rectangle normalBounds = new Rectangle();
	int fontSize = 12;
	int row = 0;
	JLabel lbl;
	
	final int w = 400;
	final int h = 20;
	final int m = 5;
	
	public TitlePreviewWindow(String title, int row) {
		super();
		this.row = row;
		
		lbl = new JLabel();
		lbl.setFont(new Font("Helvetica", 0, fontSize));
		lbl.setText(title);
		lbl.setForeground(Color.black);
		lbl.setBorder(new EmptyBorder(2, 5, 3, 5));
		getContentPane().add(lbl, BorderLayout.CENTER);
		
		getContentPane().setBackground(Color.white);
		updateNormalBounds();
		setBounds(normalBounds);
		setOpacity(0f);
		setAlwaysOnTop(true);
		
		visibility.setIncrement(0, 0.2f, 0, 0.02f);
		position.setIncrement(0.25f, 0.01f, 0.05f, 0.005f);
		
		AnimationTimer.addListener(this);
	}
	
	private void updateNormalBounds() {
		Rectangle sb = Util.getScreenBounds();
		int x = sb.x + sb.width - w - m;
		int y = sb.y + m;
		y += row * (h + 5);
		normalBounds.setRect(x, y, w, h);
		updatePositionAndSize();
	}
	
	public Rectangle getNormalBounds() {
		return normalBounds;
	}
	
	private void updatePositionAndSize() {
		double v = position.get();
//		int x = (int)(normalBounds.x + normalBounds.width * (1-v));
//		int w = (normalBounds.x + normalBounds.width) - x;
//		int h = (int)(normalBounds.height * v);
//		int y = normalBounds.y + (normalBounds.height - h)/2;
		//lbl.setFont(lbl.getFont().deriveFont((float)v*fontSize));
//		setBounds(x, y, w, h);
		int x = (int)(normalBounds.x + normalBounds.width * (1-v));
		int w = normalBounds.width;
		int h = normalBounds.height;
		int y = normalBounds.y;
		setLocation(x, y);
		repaint();
	}
	
	public void setRow(int row) {
		this.row = row;
		updateNormalBounds();
	}
	
	public int getRow() {
		return row;
	}
	
	public void showTitle() {
		visibility.set(1.0);
	}
	
	public void hideTitle() {
		visibility.directSet(0.9);
		visibility.set(0.0);
	}
	
	public void enterTitle() {
		position.set(1.0);
	}

	public void exitTitle() {
		position.set(0.0);
	}

	public void animateEvent() {
		
		if (visibility.changed()) {
			
			// Update opacity
			float op = (float)visibility.get();
			setOpacity(op);
			if (op == 0 && isVisible()) {
				super.setVisible(false);
			} else if (op > 0 && !isVisible()) {
				super.setVisible(true);
			}
		}
		
		if (position.changed()) {
			updatePositionAndSize();
		}
	}
}
