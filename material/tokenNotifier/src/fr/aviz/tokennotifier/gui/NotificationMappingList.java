package fr.aviz.tokennotifier.gui;

import java.awt.Color;
import java.awt.Graphics;
import java.awt.Rectangle;

import javax.swing.Icon;
import javax.swing.ImageIcon;
import javax.swing.JComponent;

import com.explodingpixels.macwidgets.SourceListItem;

import fr.aviz.tokennotifier.mapping.NotificationMapping;
import fr.aviz.tokennotifier.mapping.NotificationSystem;
import fr.aviz.tokennotifier.sinks.NotificationSink;
import fr.aviz.tokennotifier.sources.NotificationSource;
import fr.aviz.tokennotifier.util.GUIUtils;

public class NotificationMappingList extends JComponent {

	static Color bgColor = new Color(213, 221, 229);
	static Color fgColor = new Color(121, 136, 151);
	
	public NotificationMappingList() {
	}
	
	public void paint(Graphics g) {
		int w = getWidth();
		int h = getHeight();
		g.setColor(GUIUtils.mix(bgColor, Color.white, 0.3f));
		g.fillRect(0, 0, w, h);
		
		g.setColor(GUIUtils.multiplyAlpha(fgColor, 0.8f));
		g.fillRect(w - 2, 0, 2, h);
		
		NotificationSourceList sourceListView = NotificationSystem.getSourceListView();
		NotificationSinkList sinkListView = NotificationSystem.getSinkListView();
		if (sourceListView == null || sinkListView == null)
			return;
		int x;
		for (NotificationMapping mapping: NotificationSystem.getMappings()) {
			NotificationSource source = mapping.getSource();
			SourceListItem source_item = source.getListItem();
			if (source_item == null) continue;
			Rectangle source_bounds = sourceListView.getItemBounds(source_item);
			if (source_bounds == null) continue;
			x = 2;
			for (NotificationSink sink : mapping.getSinks()) {
				SourceListItem sink_item = sink.getListItem();
				if (sink_item == null) continue;
				ImageIcon sink_icon = (ImageIcon)sink_item.getIcon();
				if (sink_icon == null) continue;
				g.drawImage(sink_icon.getImage(), x, source_bounds.y, null);
				x += sink_icon.getIconWidth() + 2;
			}
		}
	}
	
	public void toggleConnection() {
		NotificationSourceList sourceListView = NotificationSystem.getSourceListView();
		NotificationSinkList sinkListView = NotificationSystem.getSinkListView();
		if (sourceListView == null || sinkListView == null)
			return;
		SourceListItem source_item = sourceListView.getSelectedItem();
		SourceListItem sink_item = sinkListView.getSelectedItem();
		if (source_item == null || sink_item == null) return;
		NotificationSource source = NotificationSource.getNotificationSourceForItem(source_item);
		NotificationSink sink = NotificationSink.getNotificationSinkForItem(sink_item);
		if (NotificationSystem.isConnected(source, sink))
			NotificationSystem.disconnect(source, sink);
		else
			NotificationSystem.connect(source, sink);
		repaint();
	}
	
}
