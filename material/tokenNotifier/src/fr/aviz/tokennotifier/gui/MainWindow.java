package fr.aviz.tokennotifier.gui;

import java.awt.BorderLayout;
import java.awt.Color;
import java.awt.Container;
import java.awt.FlowLayout;
import java.awt.Frame;
import java.awt.GridBagConstraints;
import java.awt.GridBagLayout;
import java.awt.GridLayout;
import java.awt.Toolkit;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.awt.event.ComponentAdapter;
import java.awt.event.ComponentEvent;
import java.awt.event.KeyEvent;
import java.awt.event.WindowAdapter;
import java.awt.event.WindowEvent;
import java.util.ArrayList;

import javax.swing.JFrame;
import javax.swing.JLabel;
import javax.swing.SwingConstants;
import javax.swing.event.TreeExpansionEvent;
import javax.swing.event.TreeExpansionListener;

import com.explodingpixels.macwidgets.HudWidgetFactory;
import com.explodingpixels.macwidgets.HudWindow;

import fr.aviz.tokennotifier.mapping.NotificationSystem;
import fr.aviz.tokennotifier.sinks.NotificationSink;
import fr.aviz.tokennotifier.sinks.NotificationSinkFactory;
import fr.aviz.tokennotifier.sources.NotificationSource;
import fr.aviz.tokennotifier.sources.NotificationSourceFactory;
import fr.aviz.tokennotifier.util.GUIUtils;

public class MainWindow extends JFrame {

	ArrayList<NotificationSourceFactory> sourceFactories = new ArrayList<NotificationSourceFactory>(); 
	ArrayList<NotificationSinkFactory> sinkFactories = new ArrayList<NotificationSinkFactory>(); 
		
	public MainWindow() {
		super ("Notification Machine");
		setSize(400, 450);
		setDefaultCloseOperation(JFrame.EXIT_ON_CLOSE);
		JLabel newlabel = new JLabel("Please wait...", SwingConstants.CENTER);
//		newlabel.setBackground(Color.black);
		getContentPane().add(newlabel, BorderLayout.CENTER);		
	}
	
	public void addSources(NotificationSourceFactory source) {
		sourceFactories.add(source);
	}
	
	public void addSinks(NotificationSinkFactory sink) {
		sinkFactories.add(sink);
	}
	
	public void connect(NotificationSource source, NotificationSink sink) {
		NotificationSystem.connect(source, sink);
	}

	public void setup() {
	
		getContentPane().remove(getContentPane().getComponent(0));
		Container contentPane = getContentPane();
		
		NotificationSourceList sourceList = new NotificationSourceList(sourceFactories);		
		NotificationSinkList sinkList = new NotificationSinkList(sinkFactories);
		final NotificationMappingList mappingList = new NotificationMappingList();
		NotificationSystem.setViews(mappingList, sourceList, sinkList);
		
		contentPane.setLayout(new GridBagLayout());
		GridBagConstraints c = new GridBagConstraints();
		c.weightx = 0.425;
		c.fill = GridBagConstraints.BOTH;
		c.gridx = 0;
		c.gridy = 0;
		c.weighty = 1.0;
		c.anchor = GridBagConstraints.CENTER;
		contentPane.add(sourceList.getComponent(), c);
		c.weightx = 0.15;
		c.gridx = 1;
		contentPane.add(mappingList, c);
		c.weightx = 0.425;
		c.gridx = 2;
		contentPane.add(sinkList.getComponent(), c);
		
		contentPane.revalidate();
		
		// INSTALL LISTENERS
		
		// Make sure the mapping list if updated if the source tree is.
		sourceList.jtree.addTreeExpansionListener(new TreeExpansionListener() {
			
			@Override
			public void treeExpanded(TreeExpansionEvent event) {
				mappingList.repaint();
			}
			
			@Override
			public void treeCollapsed(TreeExpansionEvent event) {
				mappingList.repaint();
			}
		});
		
		// Command to create / remove connections 
		GUIUtils.addGlobalKeyListener(KeyEvent.VK_SPACE, new ActionListener() {
			@Override
			public void actionPerformed(ActionEvent e) {
				mappingList.toggleConnection();
			}
		}, "");
		
		// Close
		addWindowListener(new WindowAdapter() {
			@Override
			public void windowClosing(WindowEvent e) {
				NotificationSystem.closeAll();
			};
		});

	}
	
}
