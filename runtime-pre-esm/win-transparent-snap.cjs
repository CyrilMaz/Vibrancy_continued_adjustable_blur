// Lightweight Snap replacement for transparent frameless BrowserWindows on Windows.
// Electron's transparent:true path cannot use native Aero Snap, so detect a completed
// manual move at the left/right/top edge and resize the window to the corresponding
// monitor work area. Coordinates returned by Electron's screen API are in DIP.

const SNAP_EDGE_DIP = 16;

/**
 * @param {import('electron').BrowserWindow} window
 * @param {import('electron').Screen} screen
 */
module.exports = function installTransparentWindowSnap(window, screen) {
  let applyingSnap = false;

  function onMoved() {
    if (applyingSnap || window.isDestroyed()) return;

    const cursor = screen.getCursorScreenPoint();
    const display = screen.getDisplayNearestPoint(cursor);
    const workArea = display.workArea;

    const nearLeft = cursor.x <= workArea.x + SNAP_EDGE_DIP;
    const nearRight = cursor.x >= workArea.x + workArea.width - SNAP_EDGE_DIP;
    const nearTop = cursor.y <= workArea.y + SNAP_EDGE_DIP;

    if (!nearLeft && !nearRight && !nearTop) return;

    const leftWidth = Math.floor(workArea.width / 2);
    const rightWidth = workArea.width - leftWidth;

    // Match the useful part of Windows' top-edge maximize gesture. Using setBounds()
    // instead of BrowserWindow.maximize() avoids relying on the native maximize path,
    // which is unreliable for transparent frameless HWNDs.
    const bounds = nearTop
      ? {
        x: workArea.x,
        y: workArea.y,
        width: workArea.width,
        height: workArea.height,
      }
      : nearLeft
        ? {
          x: workArea.x,
          y: workArea.y,
          width: leftWidth,
          height: workArea.height,
        }
        : {
          x: workArea.x + leftWidth,
          y: workArea.y,
          width: rightWidth,
          height: workArea.height,
        };

    applyingSnap = true;
    try {
      window.setBounds(bounds);
    } finally {
      // Some Electron versions emit a move/moved pair for setBounds(). Keep the
      // guard alive through the current turn so that cannot recursively re-snap.
      setImmediate(() => {
        applyingSnap = false;
      });
    }
  }

  window.on('moved', onMoved);
  window.on('closed', () => {
    window.removeListener('moved', onMoved);
  });
};
