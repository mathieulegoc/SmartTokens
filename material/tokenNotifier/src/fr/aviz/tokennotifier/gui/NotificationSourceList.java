package fr.aviz.tokennotifier.gui;

import java.awt.BorderLayout;
import java.util.ArrayList;

import javax.mail.Folder;
import javax.swing.JComponent;

import com.explodingpixels.macwidgets.SourceList;
import com.explodingpixels.macwidgets.SourceListCategory;
import com.explodingpixels.macwidgets.SourceListItem;
import com.explodingpixels.macwidgets.SourceListModel;

import fr.aviz.tokennotifier.mapping.NotificationSystem;
import fr.aviz.tokennotifier.sources.NotificationSource;
import fr.aviz.tokennotifier.sources.NotificationSourceFactory;
import fr.aviz.tokennotifier.sources.gmail.GmailReader;

public class NotificationSourceList extends MySourceList {
		
	// this inner class because SourceList requires a model in its constructor
	public static class ModelInitializer {
		static SourceListModel getModel(ArrayList<NotificationSourceFactory> factories) {
			SourceListModel model = new SourceListModel();

			for (NotificationSourceFactory factory : factories) {
				
				if (!factory.open()) continue;
				NotificationSystem.allNotificationFactories.add(factory);

				SourceListCategory categories = new SourceListCategory(factory.getNotificationFactoryName());			
				model.addCategory(categories);
				
				NotificationSource[] sources = factory.getAllNotificationSources();
				for (NotificationSource source : sources) {
					NotificationSystem.allNotificationObjects.add(source);
					SourceListItem item = source.getListItem();
					if (source.getParent() == null)
						model.addItemToCategory(item, categories);
					else
						model.addItemToItem(item, source.getParent().getListItem());
				}
			}			
			return model;
		}
	}
	
	SourceList sourceList;
	
	public NotificationSourceList(ArrayList<NotificationSourceFactory> factories) {
		super(ModelInitializer.getModel(factories));
	}	
}
