TARGET := xkokokeys

CFLAGS += -Wall -Werror -ggdb
CFLAGS += `pkg-config --cflags xtst x11`
LDFLAGS += `pkg-config --libs xtst x11`
LDFLAGS += -pthread

all: $(TARGET)

$(TARGET): xkokokeys.cc xkokokeys.h Makefile
	$(CXX) $(CFLAGS) -o $@ $< $(LDFLAGS)
