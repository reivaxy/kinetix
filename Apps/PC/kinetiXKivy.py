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
from dataclasses import dataclass
from typing import Any, Callable, Dict, Optional

from bleak import BleakClient, BleakScanner

from kivy.app import App as KivyApp
from kivy.clock import Clock
from kivy.lang import Builder
from kivy.metrics import dp
from kivy.properties import BooleanProperty, StringProperty
from kivy.uix.boxlayout import BoxLayout
from kivy.uix.button import Button
from kivy.uix.checkbox import CheckBox
from kivy.uix.gridlayout import GridLayout
from kivy.uix.label import Label
from kivy.uix.popup import Popup
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

    # ---------- public API (thread-safe) ----------
    def connect(self, target: DeviceTarget, timeout: float, preferred_uuid: Optional[str] = None):
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
            self._write_text_to_uuid(uuid, text, response=response, newline=newline),
            self._loop,
        )

    # ---------- internals ----------
    def _status(self, msg: str) -> None:
        try:
            self._on_status(msg)
        except Exception:
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


# ----------------- Settings popup -----------------

class SettingsPopup(Popup):
    SETTINGS_UUID = "b2a49d41-a2ac-48c3-b6c8-cfd05640654e"

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
        inner = BoxLayout(orientation="vertical", size_hint_y=None, spacing=dp(10))
        inner.bind(minimum_height=inner.setter("height"))

        # Booleans
        inner.add_widget(Label(text="Booleans", size_hint_y=None, height=dp(24), bold=True))
        bool_grid = GridLayout(cols=4, size_hint_y=None, height=dp(40), spacing=dp(10))
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
            row = BoxLayout(orientation="horizontal", size_hint_y=None, height=dp(40), spacing=dp(10))
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
            row = BoxLayout(orientation="horizontal", size_hint_y=None, height=dp(40), spacing=dp(10))
            row.add_widget(Label(text=f"{key}:", size_hint_x=None, width=dp(50)))
            ti = TextInput(multiline=False)
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
            self._set_status(f"Error: {e}")
            return

        def done():
            try:
                fut.result()
            except Exception as e:
                Clock.schedule_once(lambda _dt: self._set_status(f"Error: {e}"), 0)

        threading.Thread(target=done, daemon=True).start()

    # ----- read/populate -----
    def _read_from_device(self):
        if not self.worker.connected:
            self._set_status("Not connected.")
            return

        try:
            fut = self.worker.read_char(self.SETTINGS_UUID)
        except Exception as e:
            self._set_status(f"Error: {e}")
            return

        def done():
            try:
                data = fut.result()
                text = data.decode("utf-8", errors="ignore").strip()
                obj: Dict[str, Any] = json.loads(text) if text else {}
            except Exception as e:
                Clock.schedule_once(lambda _dt: self._set_status(f"Error reading settings: {e}"), 0)
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
            CheckBox:
                id: resp_cb
                active: root.write_with_response
                on_active: root.write_with_response = self.active
            Label:
                text: "Write with response"
                size_hint_x: None
                width: dp(160)
            CheckBox:
                id: nl_cb
                active: root.append_newline
                on_active: root.append_newline = self.active
            Label:
                text: "Append newline"
                size_hint_x: None
                width: dp(120)

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

    BoxLayout:
        orientation: "vertical"
        spacing: dp(6)

        Label:
            text: "Send"
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

    status = StringProperty("Idle.")
    mode = StringProperty("name")  # "name" or "address"
    device_name = StringProperty("KinetiX")
    device_address = StringProperty("")
    timeout_s = StringProperty("30")
    write_with_response = BooleanProperty(False)
    append_newline = BooleanProperty(False)
    custom_text = StringProperty("")
    connected = BooleanProperty(False)

    def __init__(self, **kwargs):
        super().__init__(**kwargs)
        self._worker = BleWorker(on_status=lambda m: Clock.schedule_once(lambda _dt: self._set_status(m), 0))
        self._settings_popup: Optional[SettingsPopup] = None

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
            self._set_status(f"Input error: {e}")
            return

        self._set_status("Starting connect...")
        try:
            fut = self._worker.connect(target, timeout=timeout, preferred_uuid=uuid)
        except Exception as e:
            self._set_status(f"Error: {e}")
            return

        def done():
            try:
                fut.result()
            except Exception as e:
                Clock.schedule_once(lambda _dt: self._set_status(f"Error: {e}"), 0)

        threading.Thread(target=done, daemon=True).start()

    def on_disconnect(self):
        try:
            fut = self._worker.disconnect()
        except Exception:
            return
        self._close_settings()

        def done():
            try:
                fut.result()
            except Exception as e:
                Clock.schedule_once(lambda _dt: self._set_status(f"Error: {e}"), 0)

        threading.Thread(target=done, daemon=True).start()

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
            self._set_status(f"Error: {e}")
            return

        def done():
            try:
                fut.result()
                Clock.schedule_once(lambda _dt: self._set_status(f"Sent: {text}"), 0)
            except Exception as e:
                Clock.schedule_once(lambda _dt: self._set_status(f"Error: {e}"), 0)

        threading.Thread(target=done, daemon=True).start()


class KinetixKivyApp(KivyApp):
    def build(self):
        Builder.load_string(KV)
        root = RootWidget()
        return root

    def on_stop(self):
        # Best-effort disconnect
        try:
            root = self.root
            if root and getattr(root, "_worker", None):
                root.on_disconnect()
        except Exception:
            pass


if __name__ == "__main__":
    KinetixKivyApp().run()