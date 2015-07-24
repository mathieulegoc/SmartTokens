package fr.aviz.animation;
/*
 * Copyright (c) 2008 Pierre Dragicevic <dragice@lri.fr>
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

import java.awt.DisplayMode;
import java.awt.GraphicsDevice;
import java.awt.GraphicsEnvironment;
import java.util.Vector;

/**
 * 
 * This class does all the animation support.
 * 
 * The idea is simply to replace floats by AnimatedFloats, you use set() and get()
 * and that's basically it. I don't think you need to look deep into this code.
 * 
 * @author dragice
 *
 */ 
public class AnimatedFloat { //cobertura:ignore
	
	public static final double DEFAULT_QUADRATIC_INCREMENT = 0.2f;
	public static final double DEFAULT_LINEAR_INCREMENT = 0.001f;
	
	private double value;
	
	private double destValue;
	private double currentLinearIncrement = 0;
	private double currentQuadraticIncrement = 0;
	
	private double quadraticIncrement;
	private double linearIncrement;
	private double quadraticDecrement;
	private double linearDecrement;
	private boolean valueChanged = false;
	private boolean passive;

	private static Vector<AnimatedFloat> allFloats = new Vector<AnimatedFloat>();
	
	static boolean animationEnabled = true;
	static boolean framerateCorrectionEnabled = true;
	static private long lasttime = 0;
	
	public AnimatedFloat(double initialValue) {
		this(initialValue, DEFAULT_QUADRATIC_INCREMENT, DEFAULT_LINEAR_INCREMENT);
	}

	public AnimatedFloat(double initialValue, double quadraticIncrement, double linearIncrement) {
		this(initialValue, quadraticIncrement, linearIncrement, false);
	}
	
	public AnimatedFloat(double initialValue, double quadraticIncrement, double linearIncrement, boolean passive) {
		this(initialValue, quadraticIncrement, linearIncrement, quadraticIncrement, linearIncrement, passive);
	}
	
	public AnimatedFloat(double initialValue, double quadraticIncrement, double linearIncrement, double quadraticDecrement, double linearDecrement, boolean passive) {
		this.value = initialValue;
		this.destValue = value;
		this.quadraticIncrement = quadraticIncrement;
		this.linearIncrement = linearIncrement;
		this.quadraticDecrement = quadraticDecrement;
		this.linearDecrement = linearDecrement;
		this.passive = passive;
		if (!passive)
			allFloats.add(this);
	}
	
	public void dispose() {
		if (!passive)
			allFloats.remove(this);
	}
	
	public void setIncrement(double quadraticIncrement, double linearIncrement) {
		setIncrement(quadraticIncrement, linearIncrement, quadraticIncrement, linearIncrement);
	}
	
	public double getQuadraticIncrement() {
		return quadraticIncrement;
	}
	
	public double getLinearIncrement() {
		return linearIncrement;
	}

	public void setIncrement(double quadraticIncrement, double linearIncrement, double quadraticDecrement, double linearDecrement) {
		this.quadraticIncrement = quadraticIncrement;
		this.linearIncrement = linearIncrement;
		this.quadraticDecrement = quadraticDecrement;
		this.linearDecrement = linearDecrement;
		if (value != destValue) {
			boolean forward = (destValue > value);
			currentLinearIncrement = (destValue - value) * (forward ? linearIncrement : linearDecrement);
			currentQuadraticIncrement = forward ? quadraticIncrement : quadraticDecrement;
		}
	}
	
	@Override
	public AnimatedFloat clone() {
		AnimatedFloat clone = new AnimatedFloat(0, quadraticIncrement, linearIncrement, quadraticDecrement, linearDecrement, passive);
		clone.value = value;
		clone.destValue = value;
		clone.currentLinearIncrement = currentLinearIncrement;
		clone.currentQuadraticIncrement = currentQuadraticIncrement;
		clone.valueChanged = valueChanged;
		return clone;
	}
	
	public double get() {
		return value;
	}
	
	public double directGet() {
		return destValue;
	}
	
	public int getInt() {
		return (int)Math.round(value);
	}
	
	public void set(double v) {
		if (!animationEnabled)
			directSet(v);
		else {
			if (destValue == v) return;
			destValue = v;
			boolean forward = (destValue > value);
			currentLinearIncrement = (destValue - value) * (forward ? linearIncrement : linearDecrement);
			currentQuadraticIncrement = forward ? quadraticIncrement : quadraticDecrement;
			AnimationTimer.maybeAnimate();
		}
	}
	
	public void directSet(double v) {
		destValue = v;
		value = v;
		currentLinearIncrement = 0;
		currentQuadraticIncrement = 0;
		valueChanged = true;
		AnimationTimer.fireAnimationEvent();
	}
	
	public void add(double v) {
		set(destValue + v);
	}
	
	public void directAdd(double v) {
		directSet(destValue + v);
	}

	public void mul(double v) {
		set(destValue * v);
	}
	
	public void directMul(double v) {
		directSet(destValue * v);
	}

	public boolean changed() {
		return valueChanged;
	}
	
	protected boolean incrementAtTime(double seconds) {
		return increment(getTimeStretch((int)(seconds * 1000000000.0)));
	}
	
	public boolean increment(double timeStretch) {
		if (destValue == value) {
			currentLinearIncrement = 0;
			currentQuadraticIncrement = 0;
			valueChanged = false;
			return valueChanged;
		}
		
		double newValue = value + (destValue - value) * currentQuadraticIncrement * timeStretch + (timeStretch * currentLinearIncrement);
		boolean overshoot = (newValue - destValue) * (value - destValue) < 0;
		if (overshoot)
			value = destValue;
		else
			value = newValue;
		valueChanged = true;
		return valueChanged;
	}
	
	boolean _needsAnimation() {
		return value != destValue;
	}
	
	public boolean endAnimation() {
		if (destValue == value)
			return false;
		
		value = destValue;
		currentLinearIncrement = 0;
		currentQuadraticIncrement = 0;
		return true;
	}
	
	public static void setAnimationEnabled(boolean enabled) {
		animationEnabled = enabled;
	}
	
	public static boolean animateAll() {
		double timeStretch = getTimeStretch(System.nanoTime());
		int N = allFloats.size();
		AnimatedFloat f;
		boolean needsMoreAnimation = false;
		for (int i=0; i<N; i++) {
			f = allFloats.elementAt(i);
			needsMoreAnimation |= f.increment(timeStretch);
		}
		if (!needsMoreAnimation)
			lasttime = 0;
		return needsMoreAnimation;
	}
	
	public static boolean needsAnimation() {
		int N = allFloats.size();
		for (int i=0; i<N; i++)
			if (allFloats.elementAt(i)._needsAnimation())
				return true;
		return false;
	}
	
	public static void removeAllAnimations(){
	    allFloats.clear();
	}
	
	// This is the (little bit complicated) code for framerate correction
	public static double getTimeStretch(long nanoTime) {
		double timeStretch = 1;
		if (framerateCorrectionEnabled) { 
			long newtime = nanoTime;
			if (lasttime == 0) {
				lasttime = newtime;
				return 1;
			}
			double elapsed_seconds = (newtime-lasttime)/1000000000.0;
			timeStretch = getTimeStretchFromElapsed(elapsed_seconds);
			lasttime = newtime;
		}
		return timeStretch;
	}
	
	public static double getTimeStretchFromElapsed(double elapsed_seconds) {
		return elapsed_seconds / (AnimationTimer.ANIMATION_RATE / 1000.0);
	}
	
	public double estimateAnimationTime(float deltaValue) {
		if (deltaValue == 0)
			return 0;
		boolean forward = deltaValue > 0;
		double currentLinearIncrement_ = deltaValue * (forward ? linearIncrement : linearDecrement);
		double currentQuadraticIncrement_ = forward ? quadraticIncrement : quadraticDecrement;

		if (currentLinearIncrement_ == 0)
			return Double.MAX_VALUE;
//		if (currentQuadraticIncrement_ == 0)
//			return Double.MIN_VALUE;
		double ratio = currentLinearIncrement_ / currentQuadraticIncrement_;
		double time = - Math.log(ratio / (deltaValue + ratio)) / currentQuadraticIncrement_;
		return time * AnimationTimer.ANIMATION_RATE / 1000.0;
	}
}
