package nbclient.gui;

import java.awt.datatransfer.DataFlavor;
import java.awt.datatransfer.Transferable;
import java.awt.datatransfer.UnsupportedFlavorException;
import java.io.IOException;
import java.util.ArrayList;

import javax.swing.JComponent;
import javax.swing.JTree;
import javax.swing.TransferHandler;
import javax.swing.event.TreeModelEvent;
import javax.swing.event.TreeModelListener;
import javax.swing.event.TreeSelectionEvent;
import javax.swing.event.TreeSelectionListener;
import javax.swing.tree.TreeModel;
import javax.swing.tree.TreePath;

import nbclient.data.SessionHandler;
import nbclient.data.SessionHandler.STATUS;
import nbclient.data.OpaqueLog;
import nbclient.io.FileIO;
import nbclient.util.N;
import nbclient.util.U;
import nbclient.util.N.EVENT;
import nbclient.util.N.NListener;

public class LCTreeModel implements TreeModel, TreeSelectionListener, NListener{

	/*
	 * DataModel:
	 * */
	
	private class Branch {
		String name;
		String dir; //if loading from FS, keep this around so that we can load log data later.
		
		ArrayList<OpaqueLog> leaves;
		
		public Branch() {
			leaves = new ArrayList<OpaqueLog>();
		}
		
		public String toString() {
			return name;
		}
	}
	
	String root;
	ArrayList<Branch> branches;
	Branch current;
	JTree tree;
	
	public OpaqueLog NS_macro = null;
	public OpaqueLog CS_macro = null;
	
	public LCTreeModel() {
		root = "";
		branches = new ArrayList<Branch>();
		current = null;
		
		N.listen(EVENT.STATUS, this);
		N.listen(EVENT.LOGS_ADDED, this);
	}
	
	public Object getRoot() {
		return root;
	}

	public Object getChild(Object parent, int index) {
		if (parent == root) {
			return branches.get(index);
		} else {
			Branch b = (Branch) parent;
			return b.leaves.get(index);
		}
	}

	@Override
	public int getChildCount(Object parent) {
		if (parent == root) {
			return branches.size();
		} else {
			return ((Branch) parent).leaves.size();
		}
	}

	@Override
	public boolean isLeaf(Object node) {
		if (node.getClass() == OpaqueLog.class) return true;
		else return false;
	}

	@Override
	public void valueForPathChanged(TreePath path, Object newValue) {
		U.w("ERROR: TCTreeModel asked to change value, TREE SHOULD NOT BE EDITABLE.");
	}

	@Override
	public int getIndexOfChild(Object parent, Object child) {
		if (parent == root) {
			return branches.indexOf(child);
		} else {
			return ((Branch) parent).leaves.indexOf(child);
		}
	}

	private ArrayList<TreeModelListener> listeners = new ArrayList<TreeModelListener>();
	public void addTreeModelListener(TreeModelListener l) {
		listeners.add(l);
	}
	public void removeTreeModelListener(TreeModelListener l) {
		listeners.remove(l);
	}
	
	public void valueChanged(TreeSelectionEvent e) {
		TreePath path = e.getPath();
		switch (path.getPathCount()) {
		case 0:
			//??
			U.w("ERROR: LCTreeModel path size was: " + path.getPathCount());
			break;
		case 1:
			//Root selected
			U.w("ERROR: LCTreeModel path size was: " + path.getPathCount() + "ROOT SHOULD NOT BE VISIBLE");
			break;
		case 2:
			//Branch selected, 
			NS_macro = CS_macro = null;
			break;
		case 3:
			//LOG SELECTED.
			
			Object[] path_objs = path.getPath();
			OpaqueLog lg = (OpaqueLog) path_objs[2];
			Branch b = (Branch) path_objs[1];
			
			if (lg.bytes == null) {
				try {
					assert(b.dir != null && !b.dir.isEmpty());
					FileIO.loadLog(lg, b.dir);
					
					N.notify(EVENT.LOG_LOADED, this, lg);
				} catch (IOException ex) {
					ex.printStackTrace();
					U.w("message: " + ex.getMessage());
					U.w("Could not load log data.");
					return;
				}
			}
			
			CS_macro = lg;
			int index = this.getIndexOfChild(b, lg);
			if (index + 1 < b.leaves.size()) {
				NS_macro = b.leaves.get(index + 1);
			} else {
				NS_macro = null;
			}
			
			N.notify(EVENT.SELECTION, this, lg);
			break;
		default:
				U.w("ERROR: LCTreeModel path size was: " + path.getPathCount());
		}
	}

	public void notified(EVENT e, Object src, Object... args) {
		switch (e) {
		case LOGS_ADDED:
			assert(current != null);
			
			for (Object _lg : args) {
				OpaqueLog lg = (OpaqueLog) _lg;
				current.leaves.add(lg);
			}
			
			TreeModelEvent tme = new TreeModelEvent(this, new Object[]{root, current});
			for (TreeModelListener l : listeners) {
				l.treeStructureChanged(tme);
			}
			break;
		case STATUS:
			SessionHandler hndlr = (SessionHandler) src;
			if (hndlr.status == STATUS.RUNNING) {
				Branch newb = new Branch();
				newb.dir = hndlr.log_directory; //Should be ok if null.
				newb.name = (String) args[1] + " --> " + (String) args[2];
				
				current = newb;
				branches.add(newb);
				
				TreeModelEvent tme2 = new TreeModelEvent(this, new Object[]{root},
						new int[]{branches.indexOf(current)}, new Object[]{current});
				for (TreeModelListener l : listeners) {
					l.treeNodesInserted(tme2);
				}
			}
			
			break;
			
		default:
			U.w("ERROR: LCTreeModel notified of unregistered event: " + e);
		}
	}
	
	public Exporter EXPORT_HANDLER = new Exporter();
	private class Exporter extends TransferHandler {
		
		private class LogTransfer implements Transferable {
			
			public OpaqueLog tp;

			public DataFlavor[] getTransferDataFlavors() {
				return new DataFlavor[]{U.treeFlavor};
			}

			@Override
			public boolean isDataFlavorSupported(DataFlavor flavor) {
				return flavor.equals(U.treeFlavor);
			}

			@Override
			public Object getTransferData(DataFlavor flavor)
					throws UnsupportedFlavorException, IOException {
				if (!flavor.equals(U.treeFlavor)) throw new UnsupportedFlavorException(flavor);
				else return tp;
			}
			
		}
		
		public int getSourceActions(JComponent c) {
		    return LINK;
		}

		public Transferable createTransferable(JComponent c) {
			
			TreePath p = tree.getSelectionPath();
			LogTransfer lt = new LogTransfer();
			
			if (p.getPathCount() != 3) return null;
			lt.tp = (OpaqueLog) p.getLastPathComponent();
			
		    return lt;
		}

		public void exportDone(JComponent c, Transferable t, int action) {}
	}
}
