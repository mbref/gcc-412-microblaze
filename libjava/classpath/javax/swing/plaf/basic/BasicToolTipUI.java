/* BasicToolTipUI.java --
   Copyright (C) 2004, 2005 Free Software Foundation, Inc.

This file is part of GNU Classpath.

GNU Classpath is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2, or (at your option)
any later version.

GNU Classpath is distributed in the hope that it will be useful, but
WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
General Public License for more details.

You should have received a copy of the GNU General Public License
along with GNU Classpath; see the file COPYING.  If not, write to the
Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
02110-1301 USA.

Linking this library statically or dynamically with other modules is
making a combined work based on this library.  Thus, the terms and
conditions of the GNU General Public License cover the whole
combination.

As a special exception, the copyright holders of this library give you
permission to link this library with independent modules to produce an
executable, regardless of the license terms of these independent
modules, and to copy and distribute the resulting executable under
terms of your choice, provided that you also meet, for each linked
independent module, the terms and conditions of the license of that
module.  An independent module is a module which is not derived from
or based on this library.  If you modify this library, you may extend
this exception to your version of the library, but you are not
obligated to do so.  If you do not wish to do so, delete this
exception statement from your version. */


package javax.swing.plaf.basic;

import java.awt.Color;
import java.awt.Dimension;
import java.awt.FontMetrics;
import java.awt.Graphics;
import java.awt.Insets;
import java.awt.Rectangle;
import java.awt.Toolkit;

import javax.swing.JComponent;
import javax.swing.JToolTip;
import javax.swing.LookAndFeel;
import javax.swing.SwingConstants;
import javax.swing.SwingUtilities;
import javax.swing.plaf.ComponentUI;
import javax.swing.plaf.ToolTipUI;

/**
 * This is the Basic Look and Feel UI class for JToolTip.
 */
public class BasicToolTipUI extends ToolTipUI
{

  /** The shared instance of BasicToolTipUI used for all ToolTips. */
  private static BasicToolTipUI shared;

  /** The tooltip's text */
  private String text;

  /**
   * Creates a new BasicToolTipUI object.
   */
  public BasicToolTipUI()
  {
    super();
  }

  /**
   * This method creates a new BasicToolTip UI for the given 
   * JComponent.
   *
   * @param c The JComponent to create a UI for.
   *
   * @return A BasicToolTipUI that can be used by the given JComponent.
   */
  public static ComponentUI createUI(JComponent c)
  {
    if (shared == null)
      shared = new BasicToolTipUI();
    return shared;
  }

  /**
   * This method returns the msximum size of the given JComponent.
   *
   * @param c The JComponent to find a maximum size for.
   *
   * @return The maximum size.
   */
  public Dimension getMaximumSize(JComponent c)
  {
    return getPreferredSize(c);
  }

  /**
   * This method returns the minimum size of the given JComponent.
   *
   * @param c The JComponent to find a minimum size for.
   *
   * @return The minimum size.
   */
  public Dimension getMinimumSize(JComponent c)
  {
    return getPreferredSize(c);
  }

  /**
   * This method returns the preferred size of the given JComponent.
   *
   * @param c The JComponent to find a preferred size for.
   *
   * @return The preferred size.
   */
  public Dimension getPreferredSize(JComponent c)
  {
    JToolTip tip = (JToolTip) c;
    FontMetrics fm;
    Toolkit g = tip.getToolkit();
    text = tip.getTipText();
    
    Rectangle vr = new Rectangle();
    Rectangle ir = new Rectangle();
    Rectangle tr = new Rectangle();
    Insets insets = tip.getInsets();
    fm = g.getFontMetrics(tip.getFont());
    SwingUtilities.layoutCompoundLabel(tip, fm, text, null,
                                       SwingConstants.CENTER,
                                       SwingConstants.CENTER,
                                       SwingConstants.CENTER,
                                       SwingConstants.CENTER, vr, ir, tr, 0);
    return new Dimension(insets.left + tr.width + insets.right,
                         insets.top + tr.height + insets.bottom);
  }

  /**
   * This method installs the defaults for the given JComponent.
   *
   * @param c The JComponent to install defaults for.
   */
  protected void installDefaults(JComponent c)
  {
    LookAndFeel.installColorsAndFont(c, "ToolTip.background",
                                     "ToolTip.foreground", "ToolTip.font");
    LookAndFeel.installBorder(c, "ToolTip.border");
  }

  /**
   * This method installs the listeners for the given JComponent.
   *
   * @param c The JComponent to install listeners for.
   */
  protected void installListeners(JComponent c)
  {
    // TODO: Implement this properly.
  }

  /**
   * This method installs the UI for the given JComponent.
   *
   * @param c The JComponent to install the UI for.
   */
  public void installUI(JComponent c)
  {
    c.setOpaque(true);
    installDefaults(c);
    installListeners(c);
  }

  /**
   * This method paints the given JComponent with the given Graphics object.
   *
   * @param g The Graphics object to paint with.
   * @param c The JComponent to paint.
   */
  public void paint(Graphics g, JComponent c)
  {
    JToolTip tip = (JToolTip) c;

    String text = tip.getTipText();
    Toolkit t = tip.getToolkit();
    if (text == null)
      return;

    Rectangle vr = new Rectangle();
    vr = SwingUtilities.calculateInnerArea(tip, vr);
    Rectangle ir = new Rectangle();
    Rectangle tr = new Rectangle();
    FontMetrics fm = t.getFontMetrics(tip.getFont());
    int ascent = fm.getAscent();
    SwingUtilities.layoutCompoundLabel(tip, fm, text, null,
                                       SwingConstants.CENTER,
                                       SwingConstants.CENTER,
                                       SwingConstants.CENTER,
                                       SwingConstants.CENTER, vr, ir, tr, 0);
    Color saved = g.getColor();
    g.setColor(Color.BLACK);

    g.drawString(text, vr.x, vr.y + ascent); 

    g.setColor(saved);   
  }

  /**
   * This method uninstalls the defaults for the given JComponent.
   *
   * @param c The JComponent to uninstall defaults for.
   */
  protected void uninstallDefaults(JComponent c)
  {
    c.setForeground(null);
    c.setBackground(null);
    c.setFont(null);
    c.setBorder(null);
  }

  /**
   * This method uninstalls listeners for the given JComponent.
   *
   * @param c The JComponent to uninstall listeners for.
   */
  protected void uninstallListeners(JComponent c)
  {
    // TODO: Implement this properly.
  }

  /**
   * This method uninstalls the UI for the given JComponent.
   *
   * @param c The JComponent to uninstall.
   */
  public void uninstallUI(JComponent c)
  {
    uninstallDefaults(c);
    uninstallListeners(c);
  }
}
