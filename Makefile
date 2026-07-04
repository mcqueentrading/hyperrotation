PLUGIN_NAME = hyperrotation.so
HYPRLAND_SRC ?=
WITH_LUA ?= 0

PKGS = pixman-1 libdrm pangocairo libinput libudev wayland-server xkbcommon
SRC = src/main.cpp

CXX ?= g++
CXXFLAGS += -shared -fPIC -g -std=c++2b -Wno-c++11-narrowing -O2
CXXFLAGS += $(shell pkg-config --cflags $(PKGS))
LDFLAGS += $(shell pkg-config --libs pangocairo)

ifeq ($(WITH_LUA),1)
  CXXFLAGS += -DHYPERROTATION_ENABLE_LUA $(shell pkg-config --cflags lua)
  LDFLAGS += $(shell pkg-config --libs lua)
endif

ifeq ($(strip $(HYPRLAND_SRC)),)
  HYPRLAND_CFLAGS := $(shell pkg-config --cflags hyprland 2>/dev/null)
  ifeq ($(strip $(HYPRLAND_CFLAGS)),)
    $(warning HYPRLAND_SRC is unset and pkg-config could not find hyprland.pc)
  endif
  CXXFLAGS += $(HYPRLAND_CFLAGS)
else
  CXXFLAGS += -I$(HYPRLAND_SRC) -I$(HYPRLAND_SRC)/src -I$(HYPRLAND_SRC)/protocols
endif

all: $(PLUGIN_NAME)

$(PLUGIN_NAME): $(SRC)
	$(CXX) $(CXXFLAGS) $^ -o $@ $(LDFLAGS)

clean:
	rm -f ./$(PLUGIN_NAME)

.PHONY: all clean
