package fr.aviz.animation;

import java.awt.*;
import java.awt.event.*;
import java.awt.geom.*;

import javax.swing.*;

/**
 * This Java component illustrates the use of the class AnimatedFloat.
 * 
 * The component shows two rectangles (a non-animated one and an animated one)
 * whose positions are updated each time the user clicks in the window. The rectangles
 * also appear and disappear when the mouse cursor enters/leaves the window.
 * 
 * This code illustrates how to:
 * - Declare and initialize animated floats
 * - Set/get the value of animated floats
 * - Set up the repaint mechanism
 * - Change the speed of animations
 * 
 * Please read the code attentively!
 *  
 * Additional things that are not illustrated here:
 * - Call x.endAnimation() to end an animation (equivalent to x.directSet(x.directGet())
 * - Call AnimationTimer.isRunning() to find out if floats are being animated.
 * - Call AnimatedFloat.setAnimationEnabled(false) to disable animations. Your animated
 *   floats will then behave as regular floats.
 */
public class SampleAnimation extends JComponent implements AnimationListener {

	// Initializes the attributes of the upper rectangle -- not animated
	private double x1 = 200;
	private double y1 = 200;
	private boolean visible1 = false;
	
	// Initializes the attributes of the lower rectangle -- animated.
	// Note how the "visible" boolean has been turned into a real value.
	private AnimatedFloat x2 = new AnimatedFloat(200);
	private AnimatedFloat y2 = new AnimatedFloat(200);
	private AnimatedFloat visible2 = new AnimatedFloat(0);
	
	/**
	 * The important thing here is the call to AnimationTimer.addListener().
	 */
	public SampleAnimation() {
		super();
		setPreferredSize(new Dimension(400, 400));
		
		// Adds mouse listeners -- not interesting
		addMouseListener(new MouseAdapter() {
			public void mousePressed(MouseEvent e) {
				setObjectsPosition(e.getX(), e.getY());
			}
			public void mouseEntered(MouseEvent e) {
				setObjectsVisible(true);
			}
			public void mouseExited(MouseEvent e) {
				setObjectsVisible(false);
			}
		});
		
		// This piece of code is mandatory. It will tell the component to repaint
		// itself each time an animated value changes.
		AnimationTimer.addListener(this);
		
		// Here is an example of how to change the default animation speed for a 
		// given float.
		// The following line will slow down the default animation by a factor of 4.
		// You can also directly specify the speed in AnimatedFloat constructor.
		visible2.setIncrement(
				AnimatedFloat.DEFAULT_QUADRATIC_INCREMENT / 4,
				AnimatedFloat.DEFAULT_LINEAR_INCREMENT / 4);
	}
	
	/**
	 * Called when the user clicks inside the window.
	 */
	public void setObjectsPosition(int x, int y) {
		
		// Updates the position of the non-animated rectangle
		x1 = x;
		y1 = y;
		
		// Updates the position of the animated rectangle using set().
		// If you want the rectangle to *directly* jump to (x,y),
		// use directSet() instead of set().
		x2.set(x);
		y2.set(y);
		
		repaint();
	}
	
	/**
	 * Called when the mouse enters or exits the window.
	 */
	public void setObjectsVisible(boolean visible) {
		// Updates the visibility of the non-animated rectangle
		visible1 = visible;
		
		// Updates the visibility of the animated rectangle
		visible2.set(visible ? 1 : 0);
	}
	
	/**
	 * This will tell the component to repaint itself each time an animated float
	 * changes. See the call to AnimationTimer.addListener in the constructor.
	 */
	public void animateEvent() {
		// Here we call the changed() method so that the component is repainted
		// only if values are being animated. You don't really need to do this
		// in this example, because animateEvent() will be called only if one of
		// the AnimatedFloats you declared in your Java application has changed.
		// But you might want to test for changes if you only want to repaint
		// objects/regions/components that have changed.
		if (x2.changed() || y2.changed() || visible2.changed()) {
			repaint();
		}
	}
	
	/**
	 * The usual painting code.
	 */
	public void paint(Graphics g) {
		Graphics2D g2 = (Graphics2D)g;

		// (Optional) The following will make small animations look smoother in case
		// you use double-precision rendering methods from Graphics2D
		g2.setRenderingHint(RenderingHints.KEY_ANTIALIASING, RenderingHints.VALUE_ANTIALIAS_ON);
		
		// Paints the background
		g2.setColor(Color.white);
		g2.fill(getBounds());
		
		// Paints the non-animated rectangle.
		if (visible1) {
			g2.setColor(new Color(0f, 0f, 1f));
			g2.fill(new Rectangle2D.Double(x1-20, y1-20, 40, 20));
		}
		
		// Paints the animated rectangle using get().
		// If you need to access the final values -- i.e., the values you specified
		// in the last call to set() -- instead of the current values, call directGet()
		// instead of get().
		if (visible2.get() != 0) {
			// Here we elevate visible2 to the power of 2 to make fade-ins / fade-outs
			// look more linear.
			float alpha = (float)Math.pow(visible2.get(), 2);
			g2.setColor(new Color(0f, 0f, 1f, alpha));
			g2.fill(new Rectangle2D.Double(x2.get()-20, y2.get(), 40, 20));
		}
	}

	/**
	 * Puts this component in a window and shows it.
	 */
	public static void main(String[] args) {
		JFrame win = new JFrame();
		SampleAnimation comp = new SampleAnimation();
		win.getContentPane().add(comp);
		win.pack();
		win.setVisible(true);
	}
}
