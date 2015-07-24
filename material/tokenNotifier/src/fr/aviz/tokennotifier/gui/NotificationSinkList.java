package fr.aviz.tokennotifier.gui;

import java.util.ArrayList;

import com.explodingpixels.macwidgets.SourceList;
import com.explodingpixels.macwidgets.SourceListCategory;
import com.explodingpixels.macwidgets.SourceListItem;
import com.explodingpixels.macwidgets.SourceListModel;

import fr.aviz.tokennotifier.mapping.NotificationSystem;
import fr.aviz.tokennotifier.sinks.NotificationSink;
import fr.aviz.tokennotifier.sinks.NotificationSinkFactory;

public class NotificationSinkList extends MySourceList {
		
	// this inner class because SourceList requires a model in its constructor
	public static class ModelInitializer {
		static SourceListModel getModel(ArrayList<NotificationSinkFactory> factories) {
			SourceListModel model = new SourceListModel();

			for (NotificationSinkFactory factory : factories) {
				
				if (!factory.open()) continue;
				NotificationSystem.allNotificationFactories.add(factory);

				SourceListCategory category = new SourceListCategory(factory.getNotificationFactoryName());		
				model.addCategory(category);
				
				NotificationSink[] sinks = factory.getAllNotificationSinks();
				for (NotificationSink sink : sinks) {
					NotificationSystem.allNotificationObjects.add(sink);
					SourceListItem item = sink.getListItem();
					if (sink.getParent() == null)
						model.addItemToCategory(item, category);
					else
						model.addItemToItem(item, sink.getParent().getListItem());
				}
			}			
			return model;
		}
	}
	
	SourceList sourceList;
	
	public NotificationSinkList(ArrayList<NotificationSinkFactory> factories) {
		super(ModelInitializer.getModel(factories));
	}	
}
