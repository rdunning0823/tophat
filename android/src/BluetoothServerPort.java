/*
Copyright_License {

  XCSoar Glide Computer - http://www.xcsoar.org/
  Copyright (C) 2000-2012 The XCSoar Project
  A detailed list of copyright holders can be found in the file "AUTHORS".

  This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public License
  as published by the Free Software Foundation; either version 2
  of the License, or (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
}
*/

package org.tophat;

import java.util.UUID;
import java.util.Collection;
import java.util.Set;
import java.util.LinkedList;
import java.io.IOException;
import android.util.Log;
import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothServerSocket;
import android.bluetooth.BluetoothSocket;

/**
 * A utility class which wraps the Java API into an easier API for the
 * C++ code.
 */
final class BluetoothServerPort extends MultiPort
  implements Runnable, InputListener {

  private static final String TAG = "TopHat";

  private BluetoothServerSocket serverSocket;
  private Collection<BluetoothSocket> sockets =
    new LinkedList<BluetoothSocket>();

  private Thread thread = new Thread(this, "Bluetooth server");
  private InputListener listener;

  BluetoothServerPort(BluetoothAdapter adapter, UUID uuid)
    throws IOException {
    serverSocket = adapter.listenUsingRfcommWithServiceRecord("XCSoar", uuid);

    thread.start();
  }

  @Override public void run() {
    while (true) {
      try {
        BluetoothSocket socket = serverSocket.accept();
        Log.i(TAG, "Accepted Bluetooth connection from " +
              BluetoothHelper.getDisplayString(socket));
        BluetoothPort port = new BluetoothPort(socket);

        /* make writes non-blocking and potentially lossy, to avoid
           blocking when one of the peers doesn't receive quickly
           enough */
        port.setWriteTimeout(0);

        add(port);
      } catch (IOException e) {
        Log.e(TAG, "Bluetooth server socket has failed", e);
        close();
        break;
      }
    }
  }

  public synchronized void setListener(InputListener _listener) {
    listener = _listener;
  }

  public synchronized void close() {
    if (serverSocket != null) {
      try {
        serverSocket.close();
      } catch (IOException e) {
        Log.e(TAG, "Failed to close Bluetooth server socket", e);
      } finally {
        serverSocket = null;
      }
    }

    super.close();
  }

  public boolean isValid() {
    /* note: we're not checking MultiPort.isValid(), because we assume
       the AndroidPort is doing fine even if nobody has connected
       yet */
    return serverSocket != null;
  }
}
