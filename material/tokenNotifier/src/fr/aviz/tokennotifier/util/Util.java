package fr.aviz.tokennotifier.util;

import java.awt.GraphicsDevice;
import java.awt.GraphicsEnvironment;
import java.awt.Rectangle;
import java.awt.Window;

public class Util {
	
	public static Rectangle getScreenBounds() {
		GraphicsDevice gd = GraphicsEnvironment.getLocalGraphicsEnvironment().getDefaultScreenDevice();
		return gd.getDefaultConfiguration().getBounds();
	}
	
	public static void placeWindow(Window win, double x, double y) {
		placeWindow(win, x, y, 0, 0);
	}
	
	public static void placeWindow(Window win, double x, double y, int dx, int dy) {
		int x0margin = 5;
		int x1margin = 5;
		int y0margin = 25;
		int y1margin = 5;
		Rectangle sb = getScreenBounds();
		int w = win.getWidth();
		int h = win.getHeight();
		int minx = sb.x + x0margin;
		int maxx = sb.x + sb.width - w - x1margin;
		int miny = sb.y + y0margin;
		int maxy = sb.y + sb.height - h - y1margin;
		win.setLocation((int)(minx + (maxx - minx) * x) + dx, (int)(miny + (maxy - miny) * y) + dy);
	}
	
	public static void placeWindowLeft(Window win, Window relative) {
		int margin = 5;
		int x = relative.getX() - win.getWidth() - margin;
		int y = relative.getY();
		win.setLocation(x, y);
	}
}
