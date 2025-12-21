#!/usr/bin/env python3
"""
Kinetix BLE GUI (Tkinter + Bleak)

- Default target mode: Name
- Connect button turns RED when disconnected, GREEN when connected
- Buttons send predefined strings to a specific GATT characteristic UUID

Requirements:
  pip install bleak
"""

import asyncio
import threading
import tkinter as tk
from tkinter import ttk, messagebox
from dataclasses import dataclass
from typing import Optional, Callable

from bleak import BleakClient, BleakScanner


# -----------------------------
# BLE worker running in background
# -----------------------------

@dataclass
class DeviceTarget:
    address: Optional[str] = None
    name: Optional[str] = None


class BleWorker:
    """
    Runs an asyncio event loop in a background thread.
    Tk calls worker methods; worker schedules coroutines onto its loop.
    """

    def __init__(self, on_status: Callable[[str], None]):
        self._on_status = on_status
        self._loop: Optional[asyncio.AbstractEventLoop] = None
        self._thread: Optional[threading.Thread] = None

        self._client: Optional[BleakClient] = None
        self._connected = False
        self._char_uuid: Optional[str] = None

        self._lock = threading.Lock()
        self._loop_ready = threading.Event()

    @property
    def connected(self) -> bool:
        with self._lock:
            return self._connected

    @property
    def char_uuid(self) -> Optional[str]:
        with self._lock:
            return self._char_uuid

    def start(self) -> None:
        if self._thread and self._thread.is_alive():
            return
        self._loop_ready.clear()
        self._thread = threading.Thread(target=self._run_loop, daemon=True)
        self._thread.start()
        # Wait briefly for loop to exist so connect-by-name doesn't race and fail
        if not self._loop_ready.wait(timeout=2.0):
            raise RuntimeError("BLE loop failed to initialize.")

    def stop(self) -> None:
        # Disconnect and stop loop
        if self._loop:
            try:
                fut = asyncio.run_coroutine_threadsafe(self._disconnect(), self._loop)
                fut.result(timeout=5)
            except Exception:
                pass
            try:
                self._loop.call_soon_threadsafe(self._loop.stop)
            except Exception:
                pass

    def _run_loop(self) -> None:
        self._loop = asyncio.new_event_loop()
        asyncio.set_event_loop(self._loop)
        self._loop_ready.set()
        self._loop.run_forever()

    def _status(self, msg: str) -> None:
        # Called from worker thread; caller forwards to Tk thread safely.
        self._on_status(msg)

    # ---------- public API (thread-safe) ----------
    def connect(self, target: DeviceTarget, timeout: float, preferred_uuid: Optional[str]):
        self.start()
        if not self._loop:
            raise RuntimeError("BLE loop not initialized.")
        return asyncio.run_coroutine_threadsafe(self._connect(target, timeout, preferred_uuid), self._loop)

    def disconnect(self):
        if not self._loop:
            # nothing to do
            f = asyncio.get_event_loop().create_future()  # type: ignore[attr-defined]
            f.set_result(None)
            return f
        return asyncio.run_coroutine_threadsafe(self._disconnect(), self._loop)

    def send_text(self, text: str, response: bool = False, newline: bool = False):
        if not self._loop:
            raise RuntimeError("BLE loop not initialized.")
        return asyncio.run_coroutine_threadsafe(self._send_text(text, response=response, newline=newline), self._loop)

    # ---------- internals ----------
    async def _find_device(self, target: DeviceTarget, timeout: float):
        if target.address:
            dev = await BleakScanner.find_device_by_address(target.address, timeout=timeout)
            if not dev:
                raise RuntimeError(f"Could not find device with address {target.address} (timeout={timeout}s).")
            return dev

        # find by name
        devices = await BleakScanner.discover(timeout=timeout)
        for d in devices:
            if d.name == target.name:
                return d
        raise RuntimeError(f"Could not find device named {target.name!r} (timeout={timeout}s).")

    @staticmethod
    def _pick_writeable_char_uuid(services, requested_uuid: Optional[str]) -> str:
        if requested_uuid:
            ch = services.get_characteristic(requested_uuid)
            if not ch:
                available = []
                for s in services:
                    for c in s.characteristics:
                        available.append(str(c.uuid))
                raise RuntimeError(
                    f"Characteristic UUID {requested_uuid} not found.\n"
                    f"Available characteristic UUIDs:\n  " + "\n  ".join(sorted(set(available)))
                )
            return str(ch.uuid)

        for service in services:
            for ch in service.characteristics:
                props = set(ch.properties or [])
                if "write" in props or "write-without-response" in props:
                    return str(ch.uuid)

        raise RuntimeError("No writeable characteristic found. Please set Characteristic UUID explicitly.")

    async def _ensure_services(self, client: BleakClient):
        # Bleak API compatibility across versions
        if hasattr(client, "get_services"):
            try:
                await client.get_services()  # type: ignore[attr-defined]
            except TypeError:
                client.get_services()  # type: ignore[attr-defined]
        return client.services

    async def _connect(self, target: DeviceTarget, timeout: float, preferred_uuid: Optional[str]) -> None:
        await self._disconnect()

        dev = await self._find_device(target, timeout)
        self._status(f"Found: {dev.name} [{dev.address}]")

        client = BleakClient(dev, timeout=timeout)
        self._status("Connecting...")
        await client.connect()

        if not client.is_connected:
            await client.disconnect()
            raise RuntimeError("Failed to connect.")

        services = await self._ensure_services(client)
        char_uuid = self._pick_writeable_char_uuid(services, preferred_uuid)

        with self._lock:
            self._client = client
            self._connected = True
            self._char_uuid = char_uuid

        self._status(f"Connected. Using characteristic: {char_uuid}")

    async def _disconnect(self) -> None:
        with self._lock:
            client = self._client
            self._client = None
            self._connected = False
            self._char_uuid = None

        if client:
            try:
                self._status("Disconnecting...")
                await client.disconnect()
            except Exception:
                pass
            self._status("Disconnected.")

    async def _send_text(self, text: str, response: bool = False, newline: bool = False) -> None:
        with self._lock:
            client = self._client
            char_uuid = self._char_uuid

        if not client or not client.is_connected or not char_uuid:
            raise RuntimeError("Not connected.")

        msg = text + ("\n" if newline else "")
        payload = msg.encode("utf-8")

        await client.write_gatt_char(char_uuid, payload, response=response)
        self._status(f"Sent: {msg!r}")


# -----------------------------
# Tkinter GUI
# -----------------------------

class App(ttk.Frame):
    QUICK_MESSAGES = [
        "five", "four", "three", "two", "one",
        "ok", "rock", "love",
        "open", "fist", "scratch", "come",
    ]

    DEFAULT_CHAR_UUID = "39dea685-a63e-44b2-8819-9a202581f8fe"

    def __init__(self, master: tk.Tk):
        super().__init__(master)
        self.master = master

        self.status_var = tk.StringVar(value="Idle.")
        self.mode_var = tk.StringVar(value="name")  # default is NAME
        self.address_var = tk.StringVar(value="")
        self.name_var = tk.StringVar(value="Kinetix")
        self.uuid_var = tk.StringVar(value=self.DEFAULT_CHAR_UUID)
        self.timeout_var = tk.StringVar(value="30")
        self.response_var = tk.BooleanVar(value=False)
        self.newline_var = tk.BooleanVar(value=False)

        self._build_ui()

        # BleWorker posts status messages by scheduling back onto Tk thread.
        self.worker = BleWorker(on_status=lambda m: self.master.after(0, self._set_status, m))

        self._update_controls()
        self.master.protocol("WM_DELETE_WINDOW", self.on_close)

    def _build_ui(self):
        self.master.title("Kinetix BLE Sender")
        self.pack(fill="both", expand=True, padx=12, pady=12)

        # Target frame
        target = ttk.LabelFrame(self, text="Target")
        target.pack(fill="x", pady=(0, 10))

        mode_row = ttk.Frame(target)
        mode_row.pack(fill="x", pady=6)

        ttk.Radiobutton(
            mode_row, text="Name", variable=self.mode_var, value="name",
            command=self._update_controls
        ).pack(side="left")
        ttk.Radiobutton(
            mode_row, text="Address", variable=self.mode_var, value="address",
            command=self._update_controls
        ).pack(side="left", padx=(10, 0))

        name_row = ttk.Frame(target)
        name_row.pack(fill="x", pady=4)
        ttk.Label(name_row, text="Device Name:").pack(side="left")
        self.name_entry = ttk.Entry(name_row, textvariable=self.name_var, width=26)
        self.name_entry.pack(side="left", padx=8)

        addr_row = ttk.Frame(target)
        addr_row.pack(fill="x", pady=4)
        ttk.Label(addr_row, text="BLE Address:").pack(side="left")
        self.addr_entry = ttk.Entry(addr_row, textvariable=self.address_var, width=26)
        self.addr_entry.pack(side="left", padx=8)

        uuid_row = ttk.Frame(target)
        uuid_row.pack(fill="x", pady=4)
        ttk.Label(uuid_row, text="Characteristic UUID:").pack(side="left")
        ttk.Entry(uuid_row, textvariable=self.uuid_var, width=36).pack(side="left", padx=8)

        opts_row = ttk.Frame(target)
        opts_row.pack(fill="x", pady=4)
        ttk.Label(opts_row, text="Timeout (s):").pack(side="left")
        ttk.Entry(opts_row, textvariable=self.timeout_var, width=8).pack(side="left", padx=8)
        ttk.Checkbutton(opts_row, text="Write with response", variable=self.response_var).pack(side="left", padx=(8, 0))
        ttk.Checkbutton(opts_row, text="Append newline", variable=self.newline_var).pack(side="left", padx=(8, 0))

        # Connect/disconnect buttons
        conn_row = ttk.Frame(self)
        conn_row.pack(fill="x", pady=(0, 10))

        # Use tk.Button so background color works reliably
        self.connect_btn = tk.Button(conn_row, text="Connect", command=self.on_connect, width=12)
        self.connect_btn.pack(side="left")

        self.disconnect_btn = ttk.Button(conn_row, text="Disconnect", command=self.on_disconnect)
        self.disconnect_btn.pack(side="left", padx=8)

        # Quick messages
        msg_frame = ttk.LabelFrame(self, text="Send")
        msg_frame.pack(fill="x", pady=(0, 10))

        grid = ttk.Frame(msg_frame)
        grid.pack(fill="x", padx=8, pady=8)

        self.msg_buttons = []
        for i, txt in enumerate(self.QUICK_MESSAGES):
            btn = ttk.Button(grid, text=txt, command=lambda t=txt: self.on_send(t))
            btn.grid(row=i // 4, column=i % 4, sticky="ew", padx=4, pady=4)
            self.msg_buttons.append(btn)

        for c in range(4):
            grid.columnconfigure(c, weight=1)

        # Custom send
        custom = ttk.Frame(msg_frame)
        custom.pack(fill="x", padx=8, pady=(0, 8))
        ttk.Label(custom, text="Custom:").pack(side="left")
        self.custom_var = tk.StringVar(value="")
        self.custom_entry = ttk.Entry(custom, textvariable=self.custom_var)
        self.custom_entry.pack(side="left", fill="x", expand=True, padx=8)
        self.custom_send_btn = ttk.Button(custom, text="Send", command=self.on_send_custom)
        self.custom_send_btn.pack(side="left")

        # Status bar
        status = ttk.Label(self, textvariable=self.status_var, relief="sunken", anchor="w")
        status.pack(fill="x")

    def _set_status(self, msg: str):
        self.status_var.set(msg)
        self._update_controls()

    def _set_connect_button_color(self, connected: bool):
        if connected:
            self.connect_btn.configure(bg="green", activebackground="green", fg="white")
        else:
            self.connect_btn.configure(bg="red", activebackground="red", fg="white")

    def _update_controls(self):
        connected = self.worker.connected if hasattr(self, "worker") else False
        mode = self.mode_var.get()

        # Enable target entries based on mode
        if mode == "address":
            self.addr_entry.state(["!disabled"])
            self.name_entry.state(["disabled"])
        else:
            self.name_entry.state(["!disabled"])
            self.addr_entry.state(["disabled"])

        # Connect/Disconnect state
        self.connect_btn.configure(state=("disabled" if connected else "normal"))
        self.disconnect_btn.state(["!disabled"] if connected else ["disabled"])

        # Connect button color
        self._set_connect_button_color(connected)

        # Message buttons enabled only when connected
        state = ["!disabled"] if connected else ["disabled"]
        for b in getattr(self, "msg_buttons", []):
            b.state(state)

        if hasattr(self, "custom_send_btn"):
            self.custom_send_btn.state(state)
        if hasattr(self, "custom_entry"):
            self.custom_entry.state(["!disabled"] if connected else ["disabled"])

    def _get_target(self) -> DeviceTarget:
        if self.mode_var.get() == "address":
            addr = self.address_var.get().strip()
            if not addr:
                raise ValueError("Please enter a BLE address.")
            return DeviceTarget(address=addr)

        name = self.name_var.get().strip()
        if not name:
            raise ValueError("Please enter a device name.")
        return DeviceTarget(name=name)

    def on_connect(self):
        try:
            target = self._get_target()
            timeout = float(self.timeout_var.get().strip() or "30")
            uuid = (self.uuid_var.get().strip() or None)
        except Exception as e:
            messagebox.showerror("Input error", str(e))
            return

        self._set_status("Starting connect...")
        try:
            fut = self.worker.connect(target, timeout=timeout, preferred_uuid=uuid)
        except Exception as e:
            self._set_status(f"Error: {e}")
            messagebox.showerror("Connect error", str(e))
            return

        def done():
            try:
                fut.result()
            except Exception as e:
                self._set_status(f"Error: {e}")
                messagebox.showerror("Connect error", str(e))

        threading.Thread(target=done, daemon=True).start()

    def on_disconnect(self):
        try:
            fut = self.worker.disconnect()
        except Exception:
            return

        def done():
            try:
                fut.result()
            except Exception as e:
                self._set_status(f"Error: {e}")

        threading.Thread(target=done, daemon=True).start()

    def on_send(self, text: str):
        try:
            fut = self.worker.send_text(text, response=self.response_var.get(), newline=self.newline_var.get())
        except Exception as e:
            self._set_status(f"Error: {e}")
            messagebox.showerror("Send error", str(e))
            return

        def done():
            try:
                fut.result()
            except Exception as e:
                self._set_status(f"Error: {e}")
                messagebox.showerror("Send error", str(e))

        threading.Thread(target=done, daemon=True).start()

    def on_send_custom(self):
        text = self.custom_var.get()
        if text:
            self.on_send(text)

    def on_close(self):
        # Automatically disconnect when window is closed
        try:
            if hasattr(self, "worker"):
                self.worker.stop()
        finally:
            self.master.destroy()


def main():
    root = tk.Tk()
    try:
        style = ttk.Style()
        if "clam" in style.theme_names():
            style.theme_use("clam")
    except Exception:
        pass
    App(root)
    root.mainloop()


if __name__ == "__main__":
    main()

