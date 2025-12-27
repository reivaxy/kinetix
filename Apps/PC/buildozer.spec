[app]
# (str) Title of your application
title = KinetiX
# android.add_src = android_src

# (str) Package name
package.name = kinetix

# (str) Package domain (needed for android/ios packaging)
package.domain = com.reivaxy

# (str) Source code where the main.py lives
source.dir = .

# (str) The main file to run (Buildozer will copy/rename to main.py internally)
source.include_exts = py,kv,png,jpg,jpeg,gif,svg,ttf,otf,atlas,json,ini,txt

# (str) Application versioning
version = 0.1.0

# (list) Application requirements
# Your code imports: kivy, requests, bleak, asyncio/threading/json/configparser (stdlib)
requirements =
    python3,
    kivy, 
    requests,
    bleak,
    typing_extensions,
    pyjnius,
    async_to_sync,
    async-timeout


# (str) Presplash and icon
icon.filename = ic_launcher.png
# presplash.filename = %(source.dir)s/presplash.png

# (str) Supported orientation (one of: landscape, portrait, all)
orientation = portrait

# (bool) Indicate if the application should be fullscreen or not
fullscreen = 0

# (str) The entry point file
# IMPORTANT: Buildozer expects a "main.py" entrypoint in the built app.
# Best practice: rename your script to main.py.
# If you do rename, nothing else needs changing here.
# (Leaving this as documentation comment.)
# entrypoint = main.py

# (str) List of service to declare (if any)
# services =

# (str) Set log level (1 = trace, 2 = debug, 3 = info, 4 = warning, 5 = error, 6 = critical)
log_level = 2

p4a.branch = develop
p4a.local_recipes = ../bleak/bleak/backends/p4android/recipes

# (str) Buildozer platform target
android.archs = arm64-v8a
android.accept_sdk_license = True

# (int) Android API target (SDK)
api = 34

# (int) Minimum API your APK will support
minapi = 23

# (str) Android NDK version (stable commonly used by python-for-android)
# If your setup uses a different one, adjust accordingly.
android.ndk = 27c

# (bool) Use AndroidX (recommended)
androidx = 1

# (list) Permissions
# BLE on Android 12+ requires BLUETOOTH_SCAN/CONNECT at runtime.
# Location permission is still commonly required for BLE discovery on many devices/OS versions.
# INTERNET needed for OTA upload via requests.post()
android.permissions =
    INTERNET,
    ACCESS_NETWORK_STATE,
    ACCESS_FINE_LOCATION,
    BLUETOOTH,
    BLUETOOTH_ADMIN,
    BLUETOOTH_SCAN,
    BLUETOOTH_CONNECT,
    FOREGROUND_SERVICE,
    ACCESS_FINE_LOCATION,
    ACCESS_COARSE_LOCATION,
    ACCESS_BACKGROUND_LOCATION

# (str) If you want to support BLE scanning on Android 12+, you may also want:
# permissions = ... , NEARBY_WIFI_DEVICES (not needed here), POST_NOTIFICATIONS (optional)

# (bool) Copy libraries instead of making a libpymodules.so
copy_libs = 1

# (str) Storage permissions (avoid unless you truly need them)
# android.permissions = WRITE_EXTERNAL_STORAGE,READ_EXTERNAL_STORAGE

# (str) Specify the bootstrap (sdl2 is standard for Kivy)
bootstrap = sdl2

# (list) Gradle dependencies (optional; usually not needed for Kivy + requests)
# android.gradle_dependencies = 

# (str) Enable/disable fullscreen immersive mode (optional)
# fullscreen = 0

# (list) Android manifest/application extra meta-data (optional)
# android.meta_data =

# (str) Add extra Java source dirs (optional)
android.add_src = android_src

# (bool) Use the latest supported build tools automatically
use_sdk_version = 1




[buildozer]
# (int) Log level for buildozer
log_level = 2
# (int) Display warning if buildozer is run as root (0 = False, 1 = True)
warn_on_root = 1



