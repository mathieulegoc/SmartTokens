package fr.aviz.tokennotifier.gui;

import java.awt.Point;
import java.awt.Rectangle;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.awt.event.KeyEvent;
import java.awt.event.MouseEvent;
import java.awt.event.MouseMotionListener;
import java.util.Enumeration;

import javax.swing.JPanel;
import javax.swing.JScrollPane;
import javax.swing.JTree;
import javax.swing.JViewport;
import javax.swing.TransferHandler;
import javax.swing.tree.DefaultMutableTreeNode;
import javax.swing.tree.DefaultTreeModel;
import javax.swing.tree.TreeCellRenderer;
import javax.swing.tree.TreeNode;
import javax.swing.tree.TreePath;

import com.explodingpixels.macwidgets.SourceList;
import com.explodingpixels.macwidgets.SourceListItem;
import com.explodingpixels.macwidgets.SourceListModel;

import fr.aviz.tokennotifier.util.GUIUtils;

public class MySourceList extends SourceList {

	JTree jtree;
	DefaultTreeModel treeModel;
	DefaultMutableTreeNode treeRoot;
	
	public MySourceList(SourceListModel model) {
		super(model);
		JPanel panel = (JPanel)getComponent();
		JScrollPane scrollpane = (JScrollPane)panel.getComponent(0);
		JViewport viewport = (JViewport)scrollpane.getComponent(0);
		CustomJTree tree = (CustomJTree)viewport.getComponent(0); 
		//tree.setDragEnabled(true);
		jtree = tree;
		treeModel = (DefaultTreeModel)jtree.getModel();
		treeRoot = (DefaultMutableTreeNode)treeModel.getRoot();
	}
	
	private DefaultMutableTreeNode getTreeNodeForObject(Object obj) {
		for (Enumeration e = treeRoot.depthFirstEnumeration(); e.hasMoreElements();) {
		    DefaultMutableTreeNode node = (DefaultMutableTreeNode)e.nextElement();
		    if (node.getUserObject() == obj) {
		        return node;
		    }
		}
		return null;
	}
	
	public Rectangle getItemBounds(SourceListItem item) {
		DefaultMutableTreeNode node = getTreeNodeForObject(item);
		if (node == null)
			return null;
		TreePath path = new TreePath(node.getPath());
		int row = jtree.getRowForPath(path);
		if (row == -1)
			return null;
		return jtree.getRowBounds(row); 
	}	
}
