/* Encoding.java --
   Copyright (C) 2005 Free Software Foundation, Inc.

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


package org.omg.IOP;

import org.omg.CORBA.portable.IDLEntity;

import java.io.Serializable;

/**
 * Defines the encoding format of the {@link Codec}, including the major
 * and minor version numbers. The only currently supported encodings are
 * ENCODING_CDR_ENCAPS versions 1.1 - 1.2. Vendors can implement additional
 * encodings.
 *
 * @author Audrius Meskauskas, Lithuania (AudriusA@Bioinformatics.org)
 */
public class Encoding
  implements IDLEntity, Serializable
{
  /**
   * Use serialVersionUID (v1.4) for interoperability.
   */
  private static final long serialVersionUID = -1489257079856841992L;

  /**
   * The format of encoding. For instance, {@link ENCODING_CDR_ENCAPS#value}.
   */
  public short format;

  /**
   * The major version number of this encoding format.
   */
  public byte major_version;

  /**
   * The minor version number of this encoding format.
   */
  public byte minor_version;

  /**
   * Create the unitialised instance.
   */
  public Encoding()
  {
  }

  /**
   * Create the instance, initialising field to the passed values.
   *
   * @param _format the format of encoding, like
   *  {@link ENCODING_CDR_ENCAPS#value}.
   *
   * @param _major_version the major format version.
   * @param _minor_version the minor format version.
   */
  public Encoding(short _format, byte _major_version, byte _minor_version)
  {
    format = _format;
    major_version = _major_version;
    minor_version = _minor_version;
  }
}