# HyperRotation

HyperRotation is a native Hyprland rotation plugin for the OnePlus 6T /
`oneplus-fajita` Arch Linux phone image, where display rotation must update the
panel transform and touch input transform together.

This repository used to be a small shell wrapper around `hyprctl`. It is now a
Hyprland plugin package. The old shell-wrapper approach is useful as history,
but the production path should keep compositor state changes inside Hyprland.

## What It Does

The plugin exposes rotation commands through Hyprland dispatchers:

- applies a monitor transform to a configured output
- updates `input:touchdevice:transform` to match the screen
- uses Hyprland's plugin API to run dynamic `keyword` updates inside the
  compositor instead of spawning a `hyprctl` shell process
- provides a small API that a phone sensor feed can call

The default target is the OnePlus 6T internal display:

```text
output:   DSI-1
mode:     1080x2340@60
position: 0x0
scale:    1
```

## Hyprland Config

Load the plugin from `hyprland.conf`:

```ini
plugin = ~/.config/hypr/plugins/hyperrotation.so

plugin:rotation:output = DSI-1
plugin:rotation:mode = 1080x2340@60
plugin:rotation:position = 0x0
plugin:rotation:scale = 1
plugin:rotation:touch_transform = -1
plugin:rotation:mute_notifications = 0
```

`touch_transform = -1` means "use the same transform as the monitor".

## Dispatchers

```ini
plugin:rotation:set
plugin:rotation:normal
plugin:rotation:right
plugin:rotation:down
plugin:rotation:left
```

Examples:

```sh
hyprctl dispatch plugin:rotation:set right-up
hyprctl dispatch plugin:rotation:set 3
hyprctl dispatch plugin:rotation:normal
hyprctl dispatch plugin:rotation:right
```

Supported transform names:

```text
normal      0
right       3
right-up    3
down        2
bottom-up   2
left        1
left-up     1
```

## Optional Lua API

Some Hyprland builds expose `HyprlandAPI::addLuaFunction`; others do not. The
default OnePlus 6T build uses dispatchers because they are available in the
current public Hyprland plugin API. When building against a Hyprland tree that
does provide `addLuaFunction`, build with:

```sh
WITH_LUA=1 ./build.sh
```

After loading a Lua-enabled build, Lua config can call:

```lua
hl.plugin.rotation.set("right-up")
hl.plugin.rotation.set(3)
hl.plugin.rotation.normal()
hl.plugin.rotation.right()
hl.plugin.rotation.down()
hl.plugin.rotation.left()
```

Lua should be treated as an optional control/config layer. The native plugin
owns the actual Hyprland monitor and touch transform update.

## Build

Build against the exact Hyprland headers for the Hyprland binary you will load
the plugin into. Hyprland plugins are ABI-sensitive.

Using an explicit source checkout:

```sh
HYPRLAND_SRC=/path/to/Hyprland ./build.sh
```

Using system Hyprland development headers, if your distro package provides
`hyprland.pc`:

```sh
./build.sh
```

The build produces:

```text
hyperrotation.so
```

## Install

```sh
mkdir -p ~/.config/hypr/plugins
cp hyperrotation.so ~/.config/hypr/plugins/hyperrotation.so
```

Restart Hyprland after installing or replacing the plugin.

## OnePlus 6T Phone Image Notes

For the OnePlus 6T image, HyperRotation is intended to replace direct
`hyprctl keyword monitor ... transform,...` calls in the auto-rotate path. A
sensor event feed can classify accelerometer/rotation-vector events and call the
plugin dispatcher, while the plugin applies the compositor-side monitor and
touch transform through Hyprland's plugin API.

The long-term target is a fully native event-driven sensor backend. The current
plugin API is the stable boundary for that work.

## License

MIT
