#!/usr/bin/env python3
"""
KinetiX BLE GUI (Kivy + Bleak)

A re-implementation of kinetixGui.py (Tkinter) using Kivy.

Features:
- Connect to BLE device by name or address
- Send quick messages + custom message
- "Settings" popup reads a JSON payload from a settings characteristic and populates fields
- Settings are written ONLY when the user changes a field (not while loading from device)
"""

from __future__ import annotations

import asyncio
import json
import threading
import os
import sys
import traceback
from configparser import ConfigParser
from urllib.parse import urlparse, parse_qs

import requests
from dataclasses import dataclass
from typing import Any, Callable, Dict, Optional

from bleak import BleakClient, BleakScanner

from kivy.app import App as KivyApp
from kivy.clock import Clock
from kivy.lang import Builder
from kivy.metrics import dp
from kivy.properties import BooleanProperty, StringProperty, NumericProperty
from kivy.uix.boxlayout import BoxLayout
from kivy.uix.button import Button
from kivy.uix.checkbox import CheckBox
from kivy.uix.gridlayout import GridLayout
from kivy.uix.label import Label
from kivy.uix.popup import Popup
from kivy.uix.progressbar import ProgressBar
from kivy.uix.scrollview import ScrollView
from kivy.uix.textinput import TextInput
from kivy.uix.togglebutton import ToggleButton


# ----------------- BLE worker -----------------

@dataclass
class DeviceTarget:
    name: Optional[str] = None
    address: Optional[str] = None


class BleWorker:
    """
    Runs Bleak calls on its own asyncio loop in a background thread.
    Public methods return concurrent.futures.Future objects.
    """

    def __init__(self, on_status: Callable[[str], None]):
        self._on_status = on_status

        self._lock = threading.RLock()
        self._loop: Optional[asyncio.AbstractEventLoop] = None
        self._thread: Optional[threading.Thread] = None

        self._client: Optional[BleakClient] = None
        self._connected: bool = False
        self._char_uuid: Optional[str] = None

        self._start_loop_thread()

    @property
    def connected(self) -> bool:
        with self._lock:
            return self._connected

    @property
    def char_uuid(self) -> Optional[str]:
        with self._lock:
            return self._char_uuid

    # ---------- public API (thread-safe wrappers) ----------
    def connect(self, target: DeviceTarget, timeout: float = 10.0, preferred_uuid: Optional[str] = None):
        if not self._loop:
            raise RuntimeError("BLE loop not initialized.")
        return asyncio.run_coroutine_threadsafe(self._connect(target, timeout, preferred_uuid), self._loop)

    def disconnect(self):
        if not self._loop:
            raise RuntimeError("BLE loop not initialized.")
        return asyncio.run_coroutine_threadsafe(self._disconnect(), self._loop)

    def send_text(self, text: str, response: bool = False, newline: bool = False):
        if not self._loop:
            raise RuntimeError("BLE loop not initialized.")
        return asyncio.run_coroutine_threadsafe(self._send_text(text, response=response, newline=newline), self._loop)

    def read_char(self, uuid: str):
        if not self._loop:
            raise RuntimeError("BLE loop not initialized.")
        return asyncio.run_coroutine_threadsafe(self._read_char(uuid), self._loop)

    def write_text_to_uuid(self, uuid: str, text: str, response: bool = True, newline: bool = False):
        if not self._loop:
            raise RuntimeError("BLE loop not initialized.")
        return asyncio.run_coroutine_threadsafe(
            self._write_text_to_uuid(uuid, text, response=response, newline=newline), self._loop
        )

    def start_notify(self, uuid: str, callback: Callable[[bytes], None]):
        """
        Subscribe to notifications on a characteristic UUID.
        callback is invoked (in BLE thread) with raw bytes.
        """
        if not self._loop:
            raise RuntimeError("BLE loop not initialized.")
        return asyncio.run_coroutine_threadsafe(self._start_notify(uuid, callback), self._loop)

    def stop_notify(self, uuid: str):
        if not self._loop:
            raise RuntimeError("BLE loop not initialized.")
        return asyncio.run_coroutine_threadsafe(self._stop_notify(uuid), self._loop)

    # ---------- internals ----------
    def _status(self, msg: str) -> None:
        try:
            self._on_status(msg)
        except Exception:
            traceback.print_exc(file=sys.stdout)
            pass

    def _start_loop_thread(self) -> None:
        def runner():
            loop = asyncio.new_event_loop()
            asyncio.set_event_loop(loop)
            with self._lock:
                self._loop = loop
            loop.run_forever()
            # Cleanup after stop
            pending = asyncio.all_tasks(loop)
            for t in pending:
                t.cancel()
            try:
                loop.run_until_complete(asyncio.gather(*pending, return_exceptions=True))
            except Exception:
                traceback.print_exc(file=sys.stdout)
                pass
            loop.close()

        self._thread = threading.Thread(target=runner, daemon=True)
        self._thread.start()

    async def _find_device(self, target: DeviceTarget, timeout: float):
        if target.address:
            dev = await BleakScanner.find_device_by_address(target.address, timeout=timeout)
            if not dev:
                raise RuntimeError(f"Could not find device with address {target.address} (timeout={timeout}s).")
            return dev

        devices = await BleakScanner.discover(timeout=timeout)
        target_name = (target.name or "").strip()
        if not target_name:
            raise RuntimeError("Please enter a device name or address.")

        for d in devices:
            if (d.name or "").strip() == target_name:
                return d

        # fallback: case-insensitive match
        for d in devices:
            if (d.name or "").strip().lower() == target_name.lower():
                return d

        raise RuntimeError(f"Could not find device named '{target_name}' (timeout={timeout}s).")

    async def _ensure_services(self, client: BleakClient):
        # Bleak versions differ: some have async get_services, some sync property.
        if hasattr(client, "get_services"):
            try:
                await client.get_services()  # type: ignore[attr-defined]
            except TypeError:
                traceback.print_exc(file=sys.stdout)
                client.get_services()  # type: ignore[attr-defined]
        return client.services

    async def _pick_writeable_char_uuid(self, client: BleakClient, requested_uuid: Optional[str]) -> str:
        services = await self._ensure_services(client)

        if requested_uuid:
            requested_uuid = str(requested_uuid).lower()
            available = []
            for service in services:
                for ch in service.characteristics:
                    available.append(str(ch.uuid).lower())
                    if str(ch.uuid).lower() == requested_uuid:
                        props = set(ch.properties or [])
                        if "write" in props or "write-without-response" in props:
                            return str(ch.uuid)
                        raise RuntimeError(f"Characteristic UUID {requested_uuid} is not writeable.")
            raise RuntimeError(
                f"Characteristic UUID {requested_uuid} not found.\n"
                f"Available characteristic UUIDs:\n  " + "\n  ".join(sorted(set(available)))
            )

        for service in services:
            for ch in service.characteristics:
                props = set(ch.properties or [])
                if "write" in props or "write-without-response" in props:
                    return str(ch.uuid)

        raise RuntimeError("No writeable characteristic found. Please set Characteristic UUID explicitly.")

    async def _connect(self, target: DeviceTarget, timeout: float, preferred_uuid: Optional[str]) -> None:
        await self._disconnect()

        dev = await self._find_device(target, timeout)
        self._status(f"Connecting to {dev.address} ({dev.name}) ...")

        client = BleakClient(dev)
        await client.connect(timeout=timeout)

        char_uuid = await self._pick_writeable_char_uuid(client, preferred_uuid)

        with self._lock:
            self._client = client
            self._connected = True
            self._char_uuid = char_uuid

        self._status("Connected.")
        self._status(f"Using characteristic: {char_uuid}")

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
                traceback.print_exc(file=sys.stdout)
                pass
            self._status("Disconnected.")

    async def _start_notify(self, uuid: str, callback: Callable[[bytes], None]) -> None:
        with self._lock:
            client = self._client
        if not client:
            raise RuntimeError("Not connected.")

        def _cb(_sender: int, data: bytearray):
            try:
                callback(bytes(data))
            except Exception:
                traceback.print_exc(file=sys.stdout)
                pass

        await client.start_notify(uuid, _cb)

    async def _stop_notify(self, uuid: str) -> None:
        with self._lock:
            client = self._client
        if not client:
            return
        try:
            await client.stop_notify(uuid)
        except Exception:
            traceback.print_exc(file=sys.stdout)
            pass

    async def _send_text(self, text: str, response: bool = False, newline: bool = False) -> None:
        with self._lock:
            client = self._client
            char_uuid = self._char_uuid

        if not client or not char_uuid:
            raise RuntimeError("Not connected.")

        payload = text + ("\n" if newline else "")
        await client.write_gatt_char(char_uuid, payload.encode("utf-8"), response=response)

    async def _read_char(self, uuid: str) -> bytes:
        with self._lock:
            client = self._client
        if not client:
            raise RuntimeError("Not connected.")
        return await client.read_gatt_char(uuid)

    async def _write_text_to_uuid(self, uuid: str, text: str, response: bool = True, newline: bool = False) -> None:
        with self._lock:
            client = self._client
        if not client:
            raise RuntimeError("Not connected.")
        payload = text + ("\n" if newline else "")
        await client.write_gatt_char(uuid, payload.encode("utf-8"), response=response)


# ----------------- About popup -----------------

class AboutPopup(Popup):
    ABOUT_UUID = "b2a49d41-a2ac-48c3-b6c8-cfd05640654e"
    def __init__(self, worker: BleWorker, set_status: Callable[[str], None], **kwargs):
        super().__init__(**kwargs)
        self.title = "About"
        self.size_hint = (0.9, 0.9)

        self.worker = worker
        self._set_status = set_status

        self.content = self._build_content()

        # Load 'about' information
        self._read_from_device()


    def _build_content(self):
        root = BoxLayout(orientation="vertical", spacing=dp(10), padding=dp(10))

        # --- Scrollable content ---
        scroll = ScrollView(size_hint=(1, 1), do_scroll_x=False)
        inner = BoxLayout(
            orientation="vertical",
            size_hint_y=None,
            spacing=dp(10),
            padding=[0, 0, 0, dp(10)],
        )
        inner.bind(minimum_height=inner.setter("height"))

        # Make inner take the same width as the ScrollView viewport -> responsive wrapping
        inner.bind(minimum_width=inner.setter("width"))
        scroll.bind(width=lambda *_: setattr(inner, "width", scroll.width))

        label_w = dp(150)

        # --- Row: version ---
        rowVersion = BoxLayout(
            orientation="horizontal",
            size_hint_y=None,
            height=dp(40),
            spacing=dp(10),
            size_hint_x=1,
        )
        rowVersion.add_widget(Label(text="Firmware version:", size_hint_x=None, width=label_w))
        self.versionText = Label(
            text="(waiting for connection)",
            size_hint_x=1,          # <-- expand with window
            halign="left",
            valign="middle",
            text_size=(0, None),    # will be set by bind below
        )
        # Reflow / align when width changes
        self.versionText.bind(
            size=lambda w, *_: setattr(w, "text_size", (w.width, None))
        )
        rowVersion.add_widget(self.versionText)
        inner.add_widget(rowVersion)

        # --- Row: options ---
        rowOptions = BoxLayout(
            orientation="horizontal",
            size_hint_y=None,
            height=dp(40),
            spacing=dp(10),
            size_hint_x=1,
        )
        rowOptions.add_widget(Label(text="Firmware options:", size_hint_x=None, width=label_w))
        self.optionsText = Label(
            text="(waiting for connection)",
            size_hint_x=1,          # <-- expand with window (prevents overlap/scramble)
            halign="left",
            valign="middle",
            text_size=(0, None),
        )
        self.optionsText.bind(
            size=lambda w, *_: setattr(w, "text_size", (w.width, None))
        )
        rowOptions.add_widget(self.optionsText)
        inner.add_widget(rowOptions)

        # --- License block (centered + wraps nicely) ---
        licence_row = BoxLayout(
            orientation="horizontal",
            size_hint_y=None,
            spacing=dp(10),
            padding=[0, dp(220), 0, 0],
        )

        licenceText = Label(
            text=(
                "License\n"
                "Xavier Grosjean\n"
                "Creative Commons\n"
                "Attribution-NonCommercial-ShareAlike 4.0 International\n"
                "(CC BY-NC-SA 4.0) open-source license"
            ),
            size_hint_x=1,
            halign="center",   # <-- centered (fixes “shifted left”)
            valign="top",
            markup=False,
        )

        # Wrap at label width, and auto-grow height based on texture
        def _reflow(lbl, *_):
            lbl.text_size = (lbl.width, None)
            lbl.texture_update()
            lbl.height = lbl.texture_size[1]

        licenceText.bind(width=_reflow, text=_reflow)

        # Make the row height follow the label height
        licence_row.add_widget(licenceText)
        licence_row.bind(
            minimum_height=licence_row.setter("height"),
        )

        inner.add_widget(licence_row)

        scroll.add_widget(inner)
        root.add_widget(scroll)

        # --- Buttons ---
        btn_row = BoxLayout(orientation="horizontal", size_hint_y=None, height=dp(44), spacing=dp(10))
        btn_close = Button(text="Close", size_hint_x=1)
        btn_close.bind(on_release=lambda *_: self.dismiss())
        btn_row.add_widget(btn_close)
        root.add_widget(btn_row)

        return root


    # ----- read/populate -----
    def _read_from_device(self):
        if not self.worker.connected:
            self._set_status("Not connected.")
            return

        try:
            fut = self.worker.read_char(self.ABOUT_UUID)
        except Exception as e:
            traceback.print_exc(file=sys.stdout)
            self._set_status(f"Error: {e}")
            return

        def done():
            try:
                data = fut.result()
                text = data.decode("utf-8", errors="ignore").strip()
                if text.startswith('{'):
                    obj: Dict[str, Any] = json.loads(text) if text else {}
                else:
                    obj = {}
                    obj["git_rev"] =text
                    obj["options"] = ["N/A"]
            except Exception as e:
                traceback.print_exc(file=sys.stdout)
                msg = f"Error reading About: {e}"
                Clock.schedule_once(lambda _dt: self._set_status(msg), 0)
                return

            def apply(_dt):
                self._set_status("About loaded.")
                self.versionText.text = obj["git_rev"]
                # concatenate all strings in "options" array
                self.optionsText.text = ", ".join(obj["options"])
            


            Clock.schedule_once(apply, 0)

        threading.Thread(target=done, daemon=True).start()


# ----------------- Settings popup -----------------
class AboutPopup(Popup):
    ABOUT_UUID = "b2a49d41-a2ac-48c3-b6c8-cfd05640654e"

    def __init__(self, worker: BleWorker, set_status: Callable[[str], None], **kwargs):
        super().__init__(**kwargs)
        self.title = "Settings"
        self.size_hint = (0.9, 0.9)

        self.worker = worker
        self._set_status = set_status
        self.content = self._build_content()

        # Load current settings immediately
        self._read_from_device()

        def _build_content(self):
            root = BoxLayout(orientation="vertical", spacing=dp(10), padding=dp(10))

    def _read_from_device(self):
        if not self.worker.connected:
            self._set_status("Not connected.")
            return

        try:
            fut = self.worker.read_char(self.SETTINGS_UUID)
        except Exception as e:
            traceback.print_exc(file=sys.stdout)
            self._set_status(f"Error: {e}")
            return

        def done():
            try:
                data = fut.result()
                text = data.decode("utf-8", errors="ignore").strip()
                obj: Dict[str, Any] = json.loads(text) if text else {}
            except Exception as e:
                traceback.print_exc(file=sys.stdout)
                msg = f"Error reading settings: {e}"
                Clock.schedule_once(lambda _dt: self._set_status(msg), 0)
                return

            def apply(_dt):
                # Block change handlers while we populate (fixes the original issue)
                self._loading = True
                try:
                    for i in range(1, 5):
                        k = f"b_{i}"
                finally:
                    msg=1

                            
class SettingsPopup(Popup):
    SETTINGS_UUID = "68b788da-819b-4feb-b478-8d237ef29f5f"

    def __init__(self, worker: BleWorker, set_status: Callable[[str], None], **kwargs):
        super().__init__(**kwargs)
        self.title = "Settings"
        self.size_hint = (0.9, 0.9)

        self.worker = worker
        self._set_status = set_status

        # Prevent writes while populating from device
        self._loading = False

        # debounce handles per field
        self._debounce_events: Dict[str, Any] = {}

        # state containers (widgets keyed by field)
        self._bool_widgets: Dict[str, CheckBox] = {}
        self._int_widgets: Dict[str, TextInput] = {}
        self._str_widgets: Dict[str, TextInput] = {}

        self.content = self._build_content()

        # Load current settings immediately
        self._read_from_device()

    def _build_content(self):
        root = BoxLayout(orientation="vertical", spacing=dp(10), padding=dp(10))

        scroll = ScrollView()
        inner = BoxLayout(orientation="vertical", size_hint_y=None, spacing=dp(10), size_hint_x=0.95)
        inner.bind(minimum_height=inner.setter("height"))

        # Booleans
        inner.add_widget(Label(text="Booleans", size_hint_y=None, height=dp(24), bold=True))
        bool_grid = GridLayout(cols=4, size_hint_y=None, height=dp(40), spacing=dp(10), size_hint_x=0.9)
        for i in range(1, 5):
            key = f"b_{i}"
            box = BoxLayout(orientation="horizontal", spacing=dp(6))
            cb = CheckBox()
            lbl = Label(text=key, size_hint_x=None, width=dp(40), halign="left", valign="middle")
            lbl.bind(size=lambda inst, *_: setattr(inst, "text_size", inst.size))
            cb.bind(active=lambda inst, val, k=key: self._on_bool_changed(k, val))
            self._bool_widgets[key] = cb
            box.add_widget(cb)
            box.add_widget(lbl)
            bool_grid.add_widget(box)
        inner.add_widget(bool_grid)

        # Integers
        inner.add_widget(Label(text="Integers", size_hint_y=None, height=dp(24), bold=True))
        for i in range(1, 5):
            key = f"i_{i}"
            row = BoxLayout(orientation="horizontal", size_hint_y=None, height=dp(40), 
                size_hint_x=0.9,
                spacing=dp(10))
            row.add_widget(Label(text=f"{key}:", size_hint_x=None, width=dp(50)))
            ti = TextInput(multiline=False, input_filter="int")
            ti.bind(text=lambda inst, val, k=key: self._on_text_changed(k, val, kind="int"))
            self._int_widgets[key] = ti
            row.add_widget(ti)
            inner.add_widget(row)

        # Strings
        inner.add_widget(Label(text="Strings", size_hint_y=None, height=dp(24), bold=True))
        for i in range(1, 5):
            key = f"s_{i}"
            row = BoxLayout(orientation="horizontal", size_hint_y=None, height=dp(40), spacing=dp(10), size_hint_x=0.9)
            row.add_widget(Label(text=f"{key}:", size_hint_x=None, width=dp(50)))
            ti = TextInput(multiline=False, size_hint_x=0.1)
            ti.bind(text=lambda inst, val, k=key: self._on_text_changed(k, val, kind="str"))
            self._str_widgets[key] = ti
            row.add_widget(ti)
            inner.add_widget(row)

        # Note + close
        inner.add_widget(Label(
            text="Changes are written immediately (after a short debounce).",
            size_hint_y=None,
            height=dp(30),
            halign="left",
            valign="middle",
        ))

        scroll.add_widget(inner)
        root.add_widget(scroll)

        btn_row = BoxLayout(orientation="horizontal", size_hint_y=None, height=dp(44), spacing=dp(10))
        btn_close = Button(text="Close")
        btn_close.bind(on_release=lambda *_: self.dismiss())
        btn_reload = Button(text="Reload from device")
        btn_reload.bind(on_release=lambda *_: self._read_from_device())
        btn_row.add_widget(btn_reload)
        btn_row.add_widget(btn_close)
        root.add_widget(btn_row)

        return root

    # ----- change handlers -----
    def _on_bool_changed(self, field: str, value: bool):
        if self._loading:
            return
        self._schedule_write(field)

    def _on_text_changed(self, field: str, value: str, kind: str):
        if self._loading:
            return

        # Enforce max length similar to Tk version
        if kind == "int":
            if len(value) > 8:
                # Trim without re-trigger storm
                self._loading = True
                self._int_widgets[field].text = value[:8]
                self._loading = False
        else:
            if len(value) > 20:
                self._loading = True
                self._str_widgets[field].text = value[:20]
                self._loading = False

        self._schedule_write(field)

    def _schedule_write(self, field: str):
        # Debounce: cancel prior scheduled call for this field
        ev = self._debounce_events.get(field)
        if ev is not None:
            try:
                ev.cancel()
            except Exception:
                traceback.print_exc(file=sys.stdout)
                pass
        self._debounce_events[field] = Clock.schedule_once(lambda _dt, f=field: self._write_field(f), 0.30)

    def _write_field(self, field: str):
        if not self.worker.connected:
            self._set_status("Not connected.")
            return

        if field.startswith("b_"):
            val = "true" if self._bool_widgets[field].active else "false"
        elif field.startswith("i_"):
            raw = (self._int_widgets[field].text or "").strip()
            val = raw if raw != "" else "0"
        else:
            val = self._str_widgets[field].text or ""

        payload = f"{field}={val}"
        try:
            fut = self.worker.write_text_to_uuid(self.SETTINGS_UUID, payload, response=True, newline=False)
        except Exception as e:
            traceback.print_exc(file=sys.stdout)
            self._set_status(f"Error: {e}")
            return

        def done():
            try:
                fut.result()
            except Exception as e:
                traceback.print_exc(file=sys.stdout)
                msg = f"Error: {e}"
                Clock.schedule_once(lambda _dt: self._set_status(msg), 0)

        threading.Thread(target=done, daemon=True).start()

    # ----- read/populate -----
    def _read_from_device(self):
        if not self.worker.connected:
            self._set_status("Not connected.")
            return

        try:
            fut = self.worker.read_char(self.SETTINGS_UUID)
        except Exception as e:
            traceback.print_exc(file=sys.stdout)
            self._set_status(f"Error: {e}")
            return

        def done():
            try:
                data = fut.result()
                text = data.decode("utf-8", errors="ignore").strip()
                obj: Dict[str, Any] = json.loads(text) if text else {}
            except Exception as e:
                traceback.print_exc(file=sys.stdout)
                msg = f"Error reading settings: {e}"
                Clock.schedule_once(lambda _dt: self._set_status(msg), 0)
                return

            def apply(_dt):
                # Block change handlers while we populate (fixes the original issue)
                self._loading = True
                try:
                    for i in range(1, 5):
                        k = f"b_{i}"
                        if k in obj:
                            self._bool_widgets[k].active = bool(obj[k])

                    for i in range(1, 5):
                        k = f"i_{i}"
                        if k in obj:
                            try:
                                self._int_widgets[k].text = str(int(obj[k]))[:8]
                            except Exception:
                                traceback.print_exc(file=sys.stdout)
                                self._int_widgets[k].text = ""

                    for i in range(1, 5):
                        k = f"s_{i}"
                        if k in obj:
                            self._str_widgets[k].text = str(obj[k])[:20]

                    self._set_status("Settings loaded.")
                finally:
                    self._loading = False

            Clock.schedule_once(apply, 0)

        threading.Thread(target=done, daemon=True).start()




# ----------------- OTA popup + helpers -----------------

class OtaCredentialsPopup(Popup):
    def __init__(self, *, initial_ssid: str, initial_password: str, on_start: Callable[[str, str], None], **kwargs):
        super().__init__(**kwargs)
        self.title = "OTA Wi‑Fi credentials"
        self.size_hint = (0.85, 0.55)
        self.auto_dismiss = False

        self._on_start = on_start

        root = BoxLayout(orientation="vertical", spacing=dp(10), padding=dp(10))

        row1 = BoxLayout(orientation="horizontal", size_hint_y=None, height=dp(40), spacing=dp(10))
        row1.add_widget(Label(text="SSID:", size_hint_x=None, width=dp(80)))
        self.ssid_in = TextInput(text=initial_ssid or "", multiline=False)
        row1.add_widget(self.ssid_in)
        root.add_widget(row1)

        row2 = BoxLayout(orientation="horizontal", size_hint_y=None, height=dp(40), spacing=dp(10))
        row2.add_widget(Label(text="Password:", size_hint_x=None, width=dp(80)))
        self.pw_in = TextInput(text=initial_password or "", multiline=False, password=True)
        row2.add_widget(self.pw_in)
        root.add_widget(row2)

        # Show/hide password checkbox
        row_show = BoxLayout(orientation="horizontal", size_hint_y=None, height=dp(32), spacing=dp(10))
        row_show.add_widget(Label(text="", size_hint_x=None, width=dp(80)))
        self._show_pw_cb = CheckBox(active=False, size_hint=(None, None), size=(dp(24), dp(24)))
        row_show.add_widget(self._show_pw_cb)
        row_show.add_widget(Label(text="Show password", halign="left", valign="middle"))
        # Toggle masking
        self._show_pw_cb.bind(active=lambda _cb, val: setattr(self.pw_in, "password", not bool(val)))
        root.add_widget(row_show)


        self.err = Label(text="", size_hint_y=None, height=dp(24))
        root.add_widget(self.err)

        btns = BoxLayout(orientation="horizontal", size_hint_y=None, height=dp(44), spacing=dp(10))
        btn_cancel = Button(text="Cancel")
        btn_start = Button(text="Start")
        btn_cancel.bind(on_release=lambda *_: self.dismiss())
        btn_start.bind(on_release=lambda *_: self._validate_and_start())
        btns.add_widget(btn_cancel)
        btns.add_widget(btn_start)
        root.add_widget(btns)

        self.content = root

    def _validate_and_start(self):
        ssid = (self.ssid_in.text or "").strip()
        pw = (self.pw_in.text or "")
        pw_stripped = pw.strip()

        if not ssid:
            self.err.text = "SSID must not be empty."
            return
        if not pw_stripped:
            self.err.text = "Password must not be empty."
            return
        # Simple protocol uses spaces as separators; SSID is sent as a single token.
        if " " in ssid:
            self.err.text = "SSID must not contain spaces."
            return

        self.dismiss()
        self._on_start(ssid, pw)


class OtaFilePopup(Popup):
    def __init__(self, *, on_file: Callable[[str], None], initial_dir: str = "", **kwargs):
        super().__init__(**kwargs)
        self.title = "Select firmware file"
        self.size_hint = (0.95, 0.9)
        self.auto_dismiss = False
        self._on_file = on_file

        from kivy.uix.filechooser import FileChooserListView  # imported lazily

        root = BoxLayout(orientation="vertical", spacing=dp(10), padding=dp(10))
        start_dir = initial_dir.strip() if initial_dir and os.path.isdir(initial_dir) else os.getcwd()
        self.fc = FileChooserListView(path=start_dir, filters=["*.bin", "*.hex", "*.zip", "*.*"])
        root.add_widget(self.fc)

        self.err = Label(text="", size_hint_y=None, height=dp(24))
        root.add_widget(self.err)

        btns = BoxLayout(orientation="horizontal", size_hint_y=None, height=dp(44), spacing=dp(10))
        btn_cancel = Button(text="Cancel")
        btn_ok = Button(text="Next")
        btn_cancel.bind(on_release=lambda *_: self.dismiss())
        btn_ok.bind(on_release=lambda *_: self._pick())
        btns.add_widget(btn_cancel)
        btns.add_widget(btn_ok)
        root.add_widget(btns)

        self.content = root

    def _pick(self):
        selection = self.fc.selection or []
        if not selection:
            self.err.text = "Please select a file."
            return
        fp = selection[0]
        if not os.path.isfile(fp):
            self.err.text = "Selected item is not a file."
            return
        self.dismiss()
        self._on_file(fp)

# ----------------- Main UI -----------------

KV = r"""
<RootWidget>:
    orientation: "vertical"
    padding: dp(12)
    spacing: dp(10)

    BoxLayout:
        orientation: "vertical"
        size_hint_y: None
        height: self.minimum_height
        spacing: dp(6)

        Label:
            text: "Target"
            size_hint_y: None
            height: dp(24)
            bold: True

        BoxLayout:
            orientation: "horizontal"
            size_hint_y: None
            height: dp(40)
            spacing: dp(10)

            ToggleButton:
                id: mode_name
                text: "Name"
                group: "mode"
                state: "down" if root.mode == "name" else "normal"
                on_state:
                    if self.state == "down": root.mode = "name"

            ToggleButton:
                id: mode_addr
                text: "Address"
                group: "mode"
                state: "down" if root.mode == "address" else "normal"
                on_state:
                    if self.state == "down": root.mode = "address"

        BoxLayout:
            orientation: "horizontal"
            size_hint_y: None
            height: dp(40)
            spacing: dp(10)
            Label:
                text: "Device Name:"
                size_hint_x: None
                width: dp(110)
            TextInput:
                id: name_in
                text: root.device_name
                multiline: False
                disabled: root.mode != "name"
                on_text: root.device_name = self.text

        BoxLayout:
            orientation: "horizontal"
            size_hint_y: None
            height: dp(40)
            spacing: dp(10)
            Label:
                text: "BLE Address:"
                size_hint_x: None
                width: dp(110)
            TextInput:
                id: addr_in
                text: root.device_address
                multiline: False
                disabled: root.mode != "address"
                on_text: root.device_address = self.text

        BoxLayout:
            orientation: "horizontal"
            size_hint_y: None
            height: dp(40)
            spacing: dp(10)
            Label:
                text: "Timeout (s):"
                size_hint_x: None
                width: dp(110)
            TextInput:
                id: timeout_in
                text: root.timeout_s
                multiline: False
                input_filter: "int"
                on_text: root.timeout_s = self.text

    BoxLayout:
        orientation: "horizontal"
        size_hint_y: None
        height: dp(44)
        spacing: dp(10)

        Button:
            text: "Connect"
            disabled: root.connected
            on_release: root.on_connect()

        Button:
            text: "Disconnect"
            disabled: not root.connected
            on_release: root.on_disconnect()

        Button:
            text: "Settings"
            disabled: not root.connected
            on_release: root.on_settings()

        Button:
            text: "OTA"
            disabled: not root.connected
            on_release: root.on_ota()

        Button:
            text: "About"
            on_release: root.on_about()

    BoxLayout:
        orientation: "vertical"
        spacing: dp(6)

        Label:
            text: "Positions / Movements"
            size_hint_y: None
            height: dp(24)
            bold: True

        GridLayout:
            id: quick_grid
            cols: 4
            spacing: dp(6)
            size_hint_y: None
            height: self.minimum_height

            # buttons inserted dynamically by RootWidget._post_build()

        BoxLayout:
            orientation: "horizontal"
            size_hint_y: None
            height: dp(40)
            spacing: dp(10)
            Label:
                text: "Custom:"
                size_hint_x: None
                width: dp(80)
            TextInput:
                id: custom_in
                text: root.custom_text
                multiline: False
                disabled: not root.connected
                on_text: root.custom_text = self.text
            Button:
                text: "Send"
                disabled: not root.connected
                on_release: root.on_send_custom()

    BoxLayout:
        orientation: "vertical"
        size_hint_y: None
        height: dp(70)
        spacing: dp(4)
        opacity: 1 if root.ota_active else 0

        Label:
            text: root.ota_progress_text
            size_hint_y: None
            height: dp(24)
            halign: "left"
            valign: "middle"
            text_size: self.size

        ProgressBar:
            max: 100
            value: root.ota_progress_value

    Label:
        text: root.status
        size_hint_y: None
        height: dp(28)
        halign: "left"
        valign: "middle"
        text_size: self.size
"""


class RootWidget(BoxLayout):
    QUICK_MESSAGES = [
        "five", "four", "three", "two", "one",
        "ok", "rock", "love",
        "fist", "scratch", "come",
    ]

    DEFAULT_CHAR_UUID = "39dea685-a63e-44b2-8819-9a202581f8fe"

    OTA_CHAR_UUID = "3168e56f-6ea1-420d-98f8-08a3b34afc9b"

    status = StringProperty("Idle.")
    mode = StringProperty("name")  # "name" or "address"
    device_name = StringProperty("KinetiX")
    device_address = StringProperty("")
    timeout_s = StringProperty("60")
    write_with_response = BooleanProperty(False)
    append_newline = BooleanProperty(False)
    custom_text = StringProperty("")
    connected = BooleanProperty(False)

    ota_active = BooleanProperty(False)
    ota_progress_text = StringProperty("")
    ota_progress_value = NumericProperty(0)


    # --- persisted preferences (.ini) ---
    _prefs_path: Optional[str] = None
    _prefs = None  # type: Optional[ConfigParser]
    _ota_last_dir: str = ""

    def load_prefs(self, ini_path: str):
        """Load persisted preferences from an .ini file."""
        self._prefs_path = ini_path
        cp = ConfigParser()
        try:
            cp.read(ini_path, encoding="utf-8")
        except Exception:
            traceback.print_exc(file=sys.stdout)
            # ignore broken ini
            cp = ConfigParser()
        self._prefs = cp

        sec = cp["prefs"] if cp.has_section("prefs") else None
        if not sec:
            return

        try:
            mode = sec.get("mode", "").strip()
            if mode in ("name", "address"):
                self.mode = mode
        except Exception:
            traceback.print_exc(file=sys.stdout)
            pass
        try:
            self.device_address = sec.get("ble_address", fallback="").strip()
        except Exception:
            traceback.print_exc(file=sys.stdout)
            pass
        try:
            self._ota_ssid = sec.get("ssid", fallback="").strip()
        except Exception:
            traceback.print_exc(file=sys.stdout)
            pass
        try:
            self._ota_last_dir = sec.get("last_fw_dir", fallback="").strip()
        except Exception:
            traceback.print_exc(file=sys.stdout)
            pass

    def _save_prefs(self):
        """Persist key preferences to the ini file (best-effort)."""
        if not self._prefs_path:
            return
        cp = self._prefs if isinstance(self._prefs, ConfigParser) else ConfigParser()
        if not cp.has_section("prefs"):
            cp.add_section("prefs")

        # Only persist mode/address when using address mode (requested behavior)
        if (self.mode or "").strip() == "address" and (self.device_address or "").strip():
            cp.set("prefs", "mode", "address")
            cp.set("prefs", "ble_address", (self.device_address or "").strip())

        # SSID + last firmware directory
        if getattr(self, "_ota_ssid", ""):
            cp.set("prefs", "ssid", (self._ota_ssid or "").strip())
        if getattr(self, "_ota_last_dir", ""):
            cp.set("prefs", "last_fw_dir", (self._ota_last_dir or "").strip())

        self._prefs = cp
        try:
            os.makedirs(os.path.dirname(self._prefs_path), exist_ok=True)
        except Exception:
            traceback.print_exc(file=sys.stdout)
            pass
        try:
            with open(self._prefs_path, "w", encoding="utf-8") as f:
                cp.write(f)
        except Exception:
            traceback.print_exc(file=sys.stdout)
            pass

    def __init__(self, **kwargs):
        super().__init__(**kwargs)
        self._worker = BleWorker(on_status=lambda m: Clock.schedule_once(lambda _dt: self._set_status(m), 0))
        self._settings_popup: Optional[SettingsPopup] = None
        self._about_popup: Optional[AboutPopup] = None

        # OTA state
        self._ota_ssid: str = ""
        self._ota_password: str = ""
        self._ota_file: str = ""
        self._ota_ready_url: str = ""
        self._ota_notify_on: bool = False

        # Populate quick buttons after kv is applied
        Clock.schedule_once(self._post_build, 0)

    def _post_build(self, _dt):
        grid = self.ids.get("quick_grid")
        if not grid:
            return

        grid.clear_widgets()
        for txt in self.QUICK_MESSAGES:
            btn = Button(text=txt, disabled=not self.connected, size_hint_y=None, height=dp(36))
            btn.bind(on_release=lambda inst, t=txt: self.on_send(t))
            grid.add_widget(btn)

        # Keep a reference for enable/disable updates
        self._quick_grid = grid
        self._update_connected_ui()

    def _set_status(self, msg: str):
        self.status = msg
        self.connected = self._worker.connected
        self._update_connected_ui()

    def _update_connected_ui(self):
        # Update quick buttons disabled state
        grid = getattr(self, "_quick_grid", None)
        if grid:
            for child in grid.children:
                if isinstance(child, Button):
                    child.disabled = not self.connected

    def on_connect(self):
        try:
            timeout = float((self.timeout_s or "30").strip() or "30")
            uuid = self.DEFAULT_CHAR_UUID
            if self.mode == "address":
                addr = (self.device_address or "").strip()
                if not addr:
                    raise ValueError("Please enter a BLE address.")
                target = DeviceTarget(address=addr)
            else:
                name = (self.device_name or "").strip()
                if not name:
                    raise ValueError("Please enter a device name.")
                target = DeviceTarget(name=name)
        except Exception as e:
            traceback.print_exc(file=sys.stdout)
            self._set_status(f"Input error: {e}")
            return

        self._save_prefs()
        self._set_status("Starting connect...")
        try:
            fut = self._worker.connect(target, timeout=timeout, preferred_uuid=uuid)
        except Exception as e:
            traceback.print_exc(file=sys.stdout)
            self._set_status(f"Error: {e}")
            return

        def done():
            try:
                fut.result()
            except Exception as e:
                traceback.print_exc(file=sys.stdout)
                msg = f"Error: {e}"
                Clock.schedule_once(lambda _dt: self._set_status(msg), 0)

        threading.Thread(target=done, daemon=True).start()

    def on_disconnect(self):
        try:
            fut = self._worker.disconnect()
        except Exception:
            traceback.print_exc(file=sys.stdout)
            return
        # Stop OTA notifications (best-effort)
        if getattr(self, "_ota_notify_on", False):
            try:
                futn = self._worker.stop_notify(self.OTA_CHAR_UUID)
                threading.Thread(target=lambda: futn.result(), daemon=True).start()
            except Exception:
                traceback.print_exc(file=sys.stdout)
                pass
            self._ota_notify_on = False
        self.ota_active = False
        self.ota_progress_value = 0
        self.ota_progress_text = ""

        self._close_settings()

        def done():
            try:
                fut.result()
            except Exception as e:
                traceback.print_exc(file=sys.stdout)
                msg = f"Error: {e}"
                Clock.schedule_once(lambda _dt: self._set_status(msg), 0)

        threading.Thread(target=done, daemon=True).start()

    def on_about(self):
        if self._about_popup and self._about_popup.parent:
            self._about_popup.dismiss()

        self._about_popup = AboutPopup(worker=self._worker, set_status=self._set_status)
        self._about_popup.bind(on_dismiss=lambda *_: self._set_status(self.status))
        self._about_popup.open()

    def _close_about(self):
        if self._about_popup and self._about_popup.parent:
            self._about_popup.dismiss()
        self._about_popup = None


    def on_settings(self):
        if not self.connected:
            self._set_status("Not connected.")
            return
        if self._settings_popup and self._settings_popup.parent:
            self._settings_popup.dismiss()

        self._settings_popup = SettingsPopup(worker=self._worker, set_status=self._set_status)
        self._settings_popup.bind(on_dismiss=lambda *_: self._set_status(self.status))
        self._settings_popup.open()

    def _close_settings(self):
        if self._settings_popup and self._settings_popup.parent:
            self._settings_popup.dismiss()
        self._settings_popup = None


    def on_ota(self):
        if not self.connected:
            self._set_status("Not connected.")
            return

        def got_file(fp: str):
            self._ota_file = fp
            try:
                self._ota_last_dir = os.path.dirname(fp) if fp else ""
                self._save_prefs()
            except Exception:
                traceback.print_exc(file=sys.stdout)
                pass
            cred = OtaCredentialsPopup(
                initial_ssid=self._ota_ssid,
                initial_password=self._ota_password,
                on_start=lambda ssid, pw: self._ota_start(ssid, pw),
            )
            cred.open()

        OtaFilePopup(on_file=got_file, initial_dir=getattr(self, "_ota_last_dir", "")).open()

    def _ota_start(self, ssid: str, password: str):
        # Remember in memory for subsequent uploads
        self._ota_ssid = ssid
        self._ota_password = password

        # Persist SSID (requested) and last firmware directory
        try:
            self._save_prefs()
        except Exception:
            traceback.print_exc(file=sys.stdout)
            pass

        if not self._ota_file:
            self._set_status("No file selected.")
            return

        self.ota_active = True
        self.ota_progress_value = 0
        self.ota_progress_text = f"Preparing OTA: {os.path.basename(self._ota_file)}"

        # Start notifications for OTA characteristic (best-effort)
        if not self._ota_notify_on:
            try:
                futn = self._worker.start_notify(self.OTA_CHAR_UUID, self._on_ota_notify_bytes)
                threading.Thread(target=lambda: futn.result(), daemon=True).start()
                self._ota_notify_on = True
            except Exception as e:
                traceback.print_exc(file=sys.stdout)
                self._set_status(f"OTA notify error: {e}")

        # Send OTA_START over BLE
        msg = f"OTA_START {ssid};{password}"
        # Log outgoing OTA command (console + UI status)
        try:
            print(f"[BLE TX] {msg}")
        except Exception:
            traceback.print_exc(file=sys.stdout)
            pass
        Clock.schedule_once(lambda _dt: self._set_status(f"TX: {msg}"), 0)

        try:
            fut = self._worker.write_text_to_uuid(self.OTA_CHAR_UUID, msg, response=True, newline=False)
        except Exception as e:
            traceback.print_exc(file=sys.stdout)
            self._set_status(f"Error: {e}")
            return

        def done():
            try:
                fut.result()
                Clock.schedule_once(lambda _dt: self._set_status("OTA_START sent. Waiting for OTA_READY..."), 0)
            except Exception as e:
                traceback.print_exc(file=sys.stdout)
                msg = f"Error: {e}"
                Clock.schedule_once(lambda _dt: self._set_status(msg), 0)

        threading.Thread(target=done, daemon=True).start()

    def _on_ota_notify_bytes(self, data: bytes):
        # Called in BLE thread; marshal to UI thread
        try:
            text = data.decode("utf-8", errors="ignore").strip()
        except Exception:
            traceback.print_exc(file=sys.stdout)
            return
        Clock.schedule_once(lambda _dt, t=text: self._handle_ota_message(t), 0)

    def _handle_ota_message(self, msg: str):
        if not msg:
            return

        if msg.startswith("OTA_READY"):
            parts = msg.split(maxsplit=1)
            if len(parts) < 2:
                self._set_status("OTA_READY received without URL.")
                return
            url = parts[1].strip()
            self._ota_ready_url = url
            self._set_status(f"OTA_READY: uploading to {url}")
            self.ota_progress_text = "Uploading..."
            threading.Thread(target=self._upload_firmware, daemon=True).start()
            return

        if msg.startswith("OTA_PROGRESS"):
            try:
                _pfx, pct_s, frac = msg.split(maxsplit=2)
                pct = int(pct_s)
                sent_s, total_s = frac.split("/", 1)
                sent = int(sent_s)
                total = int(total_s)
            except Exception:
                traceback.print_exc(file=sys.stdout)
                self._set_status(f"Unparsed OTA_PROGRESS: {msg}")
                return
            self.ota_progress_value = max(0, min(100, pct))
            self.ota_progress_text = f"OTA {pct}%  {sent}/{total} bytes"
            return

        if msg.startswith("OTA_"):
            self._set_status(msg)

    def _upload_firmware(self):
        url = self._ota_ready_url
        fp = self._ota_file

        if not url or not fp:
            Clock.schedule_once(lambda _dt: self._set_status("OTA upload missing URL or file."), 0)
            return

        try:
            parsed = urlparse(url)
            qs = parse_qs(parsed.query or "")
            token = (qs.get("token") or [""])[0]
            if not token:
                raise ValueError("No token in URL.")
        except Exception as e:
            traceback.print_exc(file=sys.stdout)
            msg = f"Bad OTA URL: {e}"
            Clock.schedule_once(lambda _dt: self._set_status(msg), 0)
            return

        total = os.path.getsize(fp)

        class _ProgressFile:
            def __init__(self, f, total_bytes: int, on_progress: Callable[[int], None]):
                self._f = f
                self._total = total_bytes
                self._sent = 0
                self._on_progress = on_progress

            def read(self, n=-1):
                chunk = self._f.read(n)
                if chunk:
                    self._sent += len(chunk)
                    try:
                        self._on_progress(self._sent)
                    except Exception:
                        traceback.print_exc(file=sys.stdout)
                        pass
                return chunk

            def __getattr__(self, name):
                return getattr(self._f, name)

        def on_prog(sent: int):
            if total > 0:
                pct = int((sent * 100) / total)
                Clock.schedule_once(lambda _dt: self._set_local_upload_progress(pct, sent, total), 0)

        try:
            with open(fp, "rb") as f:
                pf = _ProgressFile(f, total, on_prog)
                files = {"binary": (os.path.basename(fp), pf, "application/octet-stream")}
                headers = {"x-ota-token": token}
                r = requests.post(url, files=files, headers=headers, timeout=(10, 300))
                r.raise_for_status()
        except Exception as e:
            traceback.print_exc(file=sys.stdout)
            msg = f"OTA upload failed: {e}"
            Clock.schedule_once(lambda _dt: self._set_status(msg), 0)
            return

        Clock.schedule_once(lambda _dt: self._set_status("OTA upload finished (HTTP OK). Waiting for device..."), 0)

    def _set_local_upload_progress(self, pct: int, sent: int, total: int):
        if self.ota_progress_value < pct:
            self.ota_progress_value = max(0, min(100, pct))
        self.ota_progress_text = f"Uploading (local) {pct}%  {sent}/{total} bytes"

    def on_send_custom(self):
        self.on_send(self.custom_text or "")

    def on_send(self, text: str):
        if not self.connected:
            self._set_status("Not connected.")
            return
        text = (text or "").strip()
        if not text:
            self._set_status("Nothing to send.")
            return

        try:
            fut = self._worker.send_text(text, response=self.write_with_response, newline=self.append_newline)
        except Exception as e:
            traceback.print_exc(file=sys.stdout)
            self._set_status(f"Error: {e}")
            return

        def done():
            try:
                msg = f"Sent: {text}"
                fut.result()
                Clock.schedule_once(lambda _dt: self._set_status(msg), 0)
            except Exception as e:
                traceback.print_exc(file=sys.stdout)
                msg = f"Error: {e}"
                Clock.schedule_once(lambda _dt: self._set_status(msg), 0)

        threading.Thread(target=done, daemon=True).start()


class KinetixKivyApp(KivyApp):
    def build(self):
        Builder.load_string(KV)
        root = RootWidget()
        # Load persisted preferences
        try:
            ini_path = os.path.join(self.user_data_dir, "kinetix.ini")
            print(f"ini_path {ini_path}")
            root.load_prefs(ini_path)
        except Exception:
            traceback.print_exc(file=sys.stdout)
            pass
        return root

    def on_stop(self):
        # Best-effort disconnect
        try:
            root = self.root
            if root and getattr(root, "_worker", None):
                root.on_disconnect()
        except Exception:
            traceback.print_exc(file=sys.stdout)
            pass


if __name__ == "__main__":
    KinetixKivyApp().run()
