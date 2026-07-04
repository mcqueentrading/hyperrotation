#include <src/plugins/PluginAPI.hpp>

#include <algorithm>
#include <any>
#include <cctype>
#include <charconv>
#include <format>
#include <string>
#include <string_view>

#ifdef HYPERROTATION_ENABLE_LUA
extern "C" {
#include <lua.h>
}
#endif

inline HANDLE PHANDLE = nullptr;

static bool notificationsMuted() {
    const auto* value = HyprlandAPI::getConfigValue(PHANDLE, "plugin:rotation:mute_notifications");
    if (!value)
        return false;

    try {
        return std::any_cast<Hyprlang::INT>(value->getValue()) != 0;
    } catch (...) {
        return false;
    }
}

static void notify(const std::string& text, const CHyprColor& color, float timeMs = 2500) {
    if (notificationsMuted())
        return;

    HyprlandAPI::addNotification(PHANDLE, text, color, timeMs);
}

static std::string configString(const std::string& name, const std::string& fallback) {
    const auto* value = HyprlandAPI::getConfigValue(PHANDLE, name);
    if (!value)
        return fallback;

    try {
        return std::any_cast<Hyprlang::STRING>(value->getValue());
    } catch (...) {
        return fallback;
    }
}

static int64_t configInt(const std::string& name, int64_t fallback) {
    const auto* value = HyprlandAPI::getConfigValue(PHANDLE, name);
    if (!value)
        return fallback;

    try {
        return std::any_cast<Hyprlang::INT>(value->getValue());
    } catch (...) {
        return fallback;
    }
}

static std::string trim(std::string value) {
    const auto first = value.find_first_not_of(" \t\r\n");
    if (first == std::string::npos)
        return "";
    const auto last = value.find_last_not_of(" \t\r\n");
    return value.substr(first, last - first + 1);
}

static std::string lower(std::string value) {
    std::ranges::transform(value, value.begin(), [](unsigned char c) { return std::tolower(c); });
    return value;
}

static bool parseTransform(std::string value, int& transform) {
    value = lower(trim(value));

    if (value == "normal" || value == "portrait" || value == "0") {
        transform = 0;
        return true;
    }
    if (value == "left" || value == "left-up" || value == "90" || value == "1") {
        transform = 1;
        return true;
    }
    if (value == "down" || value == "bottom-up" || value == "inverted" || value == "180" || value == "2") {
        transform = 2;
        return true;
    }
    if (value == "right" || value == "right-up" || value == "270" || value == "3") {
        transform = 3;
        return true;
    }

    int parsed = -1;
    const auto [ptr, ec] = std::from_chars(value.data(), value.data() + value.size(), parsed);
    if (ec == std::errc{} && ptr == value.data() + value.size() && parsed >= 0 && parsed <= 7) {
        transform = parsed;
        return true;
    }

    return false;
}

static bool keywordSucceeded(const std::string& result) {
    return result == "ok";
}

static SDispatchResult applyTransformValue(int transform) {
    if (transform < 0 || transform > 7)
        return {.success = false, .error = "Transform must be between 0 and 7"};

    const auto output   = configString("plugin:rotation:output", "DSI-1");
    const auto mode     = configString("plugin:rotation:mode", "1080x2340@60");
    const auto position = configString("plugin:rotation:position", "0x0");
    const auto scale    = configString("plugin:rotation:scale", "1");

    const auto monitorValue = std::format("{},{},{},{},transform,{}", output, mode, position, scale, transform);
    const auto monitorReply = HyprlandAPI::invokeHyprctlCommand("keyword", "monitor " + monitorValue);
    if (!keywordSucceeded(monitorReply))
        return {.success = false, .error = "monitor keyword failed: " + monitorReply};

    const int64_t touchTransform = configInt("plugin:rotation:touch_transform", -1);
    const int64_t effectiveTouch = touchTransform >= 0 ? touchTransform : transform;
    const auto    touchReply     = HyprlandAPI::invokeHyprctlCommand("keyword", "input:touchdevice:transform " + std::to_string(effectiveTouch));
    if (!keywordSucceeded(touchReply))
        return {.success = false, .error = "touch transform keyword failed: " + touchReply};

    notify(std::format("HyperRotation {} -> transform {}", output, transform), CHyprColor{0.3, 0.9, 0.55, 1.0});
    return {};
}

static SDispatchResult applyTransform(std::string args) {
    int transform = 0;
    if (!parseTransform(args, transform))
        return {.success = false, .error = "Unknown transform: " + args};

    return applyTransformValue(transform);
}

static SDispatchResult applyNamedTransform(int transform) {
    return applyTransformValue(transform);
}

#ifdef HYPERROTATION_ENABLE_LUA
static int luaSet(lua_State* L) {
    int transform = 0;

    if (lua_isnumber(L, 1)) {
        transform = static_cast<int>(lua_tonumber(L, 1));
    } else if (const char* value = lua_tostring(L, 1); value) {
        if (!parseTransform(value, transform))
            return 0;
    } else {
        return 0;
    }

    applyTransformValue(transform);
    return 0;
}

static int luaNormal(lua_State*) {
    applyTransformValue(0);
    return 0;
}

static int luaRight(lua_State*) {
    applyTransformValue(3);
    return 0;
}

static int luaDown(lua_State*) {
    applyTransformValue(2);
    return 0;
}

static int luaLeft(lua_State*) {
    applyTransformValue(1);
    return 0;
}
#endif

APICALL EXPORT std::string PLUGIN_API_VERSION() {
    return HYPRLAND_API_VERSION;
}

APICALL EXPORT PLUGIN_DESCRIPTION_INFO PLUGIN_INIT(HANDLE handle) {
    PHANDLE = handle;

    HyprlandAPI::addConfigValue(PHANDLE, "plugin:rotation:output", Hyprlang::CConfigValue(Hyprlang::STRING{"DSI-1"}));
    HyprlandAPI::addConfigValue(PHANDLE, "plugin:rotation:mode", Hyprlang::CConfigValue(Hyprlang::STRING{"1080x2340@60"}));
    HyprlandAPI::addConfigValue(PHANDLE, "plugin:rotation:position", Hyprlang::CConfigValue(Hyprlang::STRING{"0x0"}));
    HyprlandAPI::addConfigValue(PHANDLE, "plugin:rotation:scale", Hyprlang::CConfigValue(Hyprlang::STRING{"1"}));
    HyprlandAPI::addConfigValue(PHANDLE, "plugin:rotation:touch_transform", Hyprlang::CConfigValue(Hyprlang::INT{-1}));
    HyprlandAPI::addConfigValue(PHANDLE, "plugin:rotation:mute_notifications", Hyprlang::CConfigValue(Hyprlang::INT{0}));

    HyprlandAPI::addDispatcherV2(PHANDLE, "plugin:rotation:set", ::applyTransform);
    HyprlandAPI::addDispatcherV2(PHANDLE, "plugin:rotation:normal", [](std::string) { return applyNamedTransform(0); });
    HyprlandAPI::addDispatcherV2(PHANDLE, "plugin:rotation:right", [](std::string) { return applyNamedTransform(3); });
    HyprlandAPI::addDispatcherV2(PHANDLE, "plugin:rotation:down", [](std::string) { return applyNamedTransform(2); });
    HyprlandAPI::addDispatcherV2(PHANDLE, "plugin:rotation:left", [](std::string) { return applyNamedTransform(1); });

#ifdef HYPERROTATION_ENABLE_LUA
    HyprlandAPI::addLuaFunction(PHANDLE, "rotation", "set", ::luaSet);
    HyprlandAPI::addLuaFunction(PHANDLE, "rotation", "normal", ::luaNormal);
    HyprlandAPI::addLuaFunction(PHANDLE, "rotation", "right", ::luaRight);
    HyprlandAPI::addLuaFunction(PHANDLE, "rotation", "down", ::luaDown);
    HyprlandAPI::addLuaFunction(PHANDLE, "rotation", "left", ::luaLeft);
#endif

    notify("HyperRotation plugin loaded", CHyprColor{0.3, 0.9, 0.55, 1.0}, 3500);
    return {"hyperrotation", "Native Hyprland monitor and touch rotation control", "Toni McQueen", "0.2.0"};
}

APICALL EXPORT void PLUGIN_EXIT() {
    PHANDLE = nullptr;
}
