package nbtool.gui.logviews.images;

import java.awt.Graphics;
import java.awt.Color;
import java.awt.event.ComponentAdapter;
import java.awt.event.ComponentEvent;
import java.awt.event.MouseListener;
import java.awt.event.MouseEvent;
import java.awt.image.BufferedImage;
import java.io.ByteArrayInputStream;
import java.io.DataInputStream;
import java.util.Vector;

import nbtool.util.Logger;
import nbtool.data.Log;
import nbtool.gui.logviews.misc.ViewParent;
import nbtool.images.EdgeImage;
import nbtool.io.CommonIO.IOFirstResponder;
import nbtool.io.CommonIO.IOInstance;
import nbtool.io.CrossIO;
import nbtool.io.CrossIO.CrossFunc;
import nbtool.io.CrossIO.CrossInstance;
import nbtool.io.CrossIO.CrossCall;
import nbtool.util.Utility;

public class LineView extends ViewParent implements IOFirstResponder {
    
    final int width = 320;
    final int height = 240;
    
    final int displayw = 640;
    final int displayh = 480;

    final int fieldw = 640;
    final int fieldh = 554;

    final int buffer = 5;

    double resize = 1;

    // Field coordinate image upper left hand corder
    int fx0 = displayw + buffer;
    int fy0 = 0;

    // Center of field cordinate system
    int fxc = displayw + buffer + fieldw/2; 
    int fyc = fieldh;
    
    BufferedImage originalImage;
    BufferedImage edgeImage;
    Vector<Double> lines;


    @Override
    public void setLog(Log newlog) {
        CrossInstance ci = CrossIO.instanceByIndex(0);
        if (ci == null)
            return;
        CrossFunc func = ci.functionWithName("Vision");
        assert(func != null);
        
        CrossCall cc = new CrossCall(this, func, newlog);

        assert(ci.tryAddCall(cc));

        originalImage = Utility.biFromLog(newlog);
    }
    
    public void paintComponent(Graphics g) {
        if (edgeImage != null) { 
            g.drawImage(originalImage, 0, 0, displayw, displayh, null);
            g.drawImage(edgeImage, 0, displayh + buffer, displayw, displayh, null);
            

            g.setColor(new Color(90, 130, 90));
            g.fillRect(645, 0, fieldw, fieldh);

            int[] xPoints1 = {0 + fx0, 0 + fx0, fieldw/2 + fx0};
            int[] yPoints1 = {2, fieldh, fieldh};
            int[] xPoints2 = {fieldw + fx0, fieldw/2 + fx0, fieldw + fx0};
            int[] yPoints2 = {2, fieldh, fieldh};
            int n = 3;
            g.setColor(new Color(46, 99, 28));
            g.fillPolygon(xPoints1, yPoints1, n);
            g.fillPolygon(xPoints2, yPoints2, n);

            g.setColor(Color.lightGray);
            g.fillOval(fxc - 30, fyc - 20, 60, 40);

            for (int i = 0; i < lines.size(); i += 10) {
                double icR = lines.get(i);
                double icT = lines.get(i + 1);
                double icEP0 = lines.get(i + 2);
                double icEP1 = lines.get(i + 3);
                double houghIndex = lines.get(i + 4);
                double fieldIndex = lines.get(i + 5);
                double fcR = lines.get(i + 6);
                double fcT = lines.get(i + 7);
                double fcEP0 = lines.get(i + 8);
                double fcEP1 = lines.get(i + 9);

                // Draw it in image coordinates
                if (fieldIndex == -1)
                    g.setColor(Color.red);
                else
                    g.setColor(Color.blue);

                double x0 = 2*icR * Math.cos(icT) + originalImage.getWidth() / 2;
                double y0 = -2*icR * Math.sin(icT) + originalImage.getHeight() / 2;
                int x1 = (int) Math.round(x0 + 2*icEP0 * Math.sin(icT));
                int y1 = (int) Math.round(y0 + 2*icEP0 * Math.cos(icT));
                int x2 = (int) Math.round(x0 + 2*icEP1 * Math.sin(icT));
                int y2 = (int) Math.round(y0 + 2*icEP1 * Math.cos(icT));

                g.drawLine(x1, y1, x2, y2);

                // Image view line labels 
                double xstring = (x1 + x2) / 2;
                double ystring = (y1 + y2) / 2;

                double scale = 0;
                if (icR > 0)
                    scale = 10;
                else
                    scale = 3;
                xstring += scale*Math.cos(icT);
                ystring += scale*Math.sin(icT);

                g.drawString(Integer.toString((int) houghIndex) + "/" + Integer.toString((int) fieldIndex), 
                             (int) xstring, 
                             (int) ystring);

                // Calculate field coordinates to find resize value
                x0 =  2*fcR * Math.cos(fcT) + displayw + buffer + fieldw/2;
                y0 = -2*fcR * Math.sin(fcT) + fieldh;
                x1 = (int) Math.round(x0 + 2*fcEP0 * Math.sin(fcT));
                y1 = (int) Math.round(y0 + 2*fcEP0 * Math.cos(fcT));
                x2 = (int) Math.round(x0 + 2*fcEP1 * Math.sin(fcT));
                y2 = (int) Math.round(y0 + 2*fcEP1 * Math.cos(fcT));

                if (y1 < 0 && y1 > -2500) {
                    resize = Math.min(resize, (double)fieldh/(-y1 + fieldh));
                }

                if (y2 < 0 && y1 > -2500) {
                    resize = Math.min(resize, (double)fieldh/(-y1 + fieldh));
                }
            }

            for (int i = 0; i < lines.size(); i += 10) {
                double houghIndex = lines.get(i + 4);
                double fieldIndex = lines.get(i + 5);
                double fcR = lines.get(i + 6);
                double fcT = lines.get(i + 7);
                double fcEP0 = lines.get(i + 8);
                double fcEP1 = lines.get(i + 9);

                // Draw it in field coordinates
                if (fieldIndex >= 0)
                    g.setColor(Color.white);
                else
                    g.setColor(Color.red);

                // Recalculate with resize
                double x0 =  2*resize*fcR * Math.cos(fcT) + displayw + buffer + fieldw/2;
                double y0 = -2*resize*fcR * Math.sin(fcT) + fieldh;
                int x1 = (int) Math.round(x0 + 2*resize*fcEP0 * Math.sin(fcT));
                int y1 = (int) Math.round(y0 + 2*resize*fcEP0 * Math.cos(fcT));
                int x2 = (int) Math.round(x0 + 2*resize*fcEP1 * Math.sin(fcT));
                int y2 = (int) Math.round(y0 + 2*resize*fcEP1 * Math.cos(fcT));

                g.drawLine(x1, y1, x2, y2);

                g.setColor(Color.black);
                g.drawLine(fxc, fyc, fxc, (int)(fyc - (200 * resize)));
            }
        }
    }
    
	public LineView() {
		super();
		setLayout(null);
        lines = new Vector<Double>();

        this.addMouseListener(new DistanceGetter());
	}

    class DistanceGetter implements MouseListener {

      public void mouseClicked(MouseEvent e) {
      }

      public void mousePressed(MouseEvent e) {
      }

      public void mouseReleased(MouseEvent e) {
      }

      public void mouseEntered(MouseEvent e) {
      }

      public void mouseExited(MouseEvent e) {
      }
        
    }

    @Override
    public void ioFinished(IOInstance instance) {}

    @Override
    public void ioReceived(IOInstance inst, int ret, Log... out) {
        EdgeImage ei = new EdgeImage(width, height,  out[5].bytes);
        edgeImage = ei.toBufferedImage();
        repaint();

        // TODO refactor into hough line class
        byte[] lineBytes = out[6].bytes;
        int numLines = lineBytes.length / (18 * 4);
        Logger.logf(Logger.INFO, "%d field lines expected.", numLines);
        try {
            DataInputStream dis = new DataInputStream(new ByteArrayInputStream(lineBytes));
            for (int i = 0; i < numLines; ++i) {
                lines.add(dis.readDouble()); // image coord r
                lines.add(dis.readDouble()); // image coord t
                lines.add(dis.readDouble()); // image coord ep0
                lines.add(dis.readDouble()); // image coord ep1
                lines.add((double)dis.readInt()); // hough index
                lines.add((double)dis.readInt()); // fieldline index
                lines.add(dis.readDouble()); // field coord r
                lines.add(dis.readDouble()); // field coord t
                lines.add(dis.readDouble()); // field coord ep0
                lines.add(dis.readDouble()); // field coord ep1
            }
        } catch (Exception e) {
            Logger.logf(Logger.ERROR, "Conversion from bytes to hough coord lines in LineView failed.");
            e.printStackTrace();
        }
    }

	@Override
	public boolean ioMayRespondOnCenterThread(IOInstance inst) {
		return false;
	}
}
