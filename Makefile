PREFIX ?= /usr/local
BINDIR := $(PREFIX)/bin
DATADIR := $(PREFIX)/share/nodistractions
TARGET := nodistractions
SRCS   := TimerApp.cpp MainWindow.cpp
ASSETS := style.css $(wildcard character*.png character*.jpg)
CXX    ?= g++
CXXFLAGS ?= -std=c++17 $(shell pkg-config --cflags gtkmm-3.0)
CXXFLAGS += -DNO_DISTRACTIONS_DATADIR=$(DATADIR)
LDFLAGS  ?= $(shell pkg-config --libs gtkmm-3.0)

.PHONY: all clean install uninstall

all: $(TARGET)

$(TARGET): $(SRCS)
	$(CXX) $(CXXFLAGS) $^ $(LDFLAGS) -o $@

clean:
	rm -f $(TARGET) *.o

install: $(TARGET)
	install -d $(DESTDIR)$(BINDIR)
	install -m 755 $(TARGET) $(DESTDIR)$(BINDIR)/$(TARGET)
	install -d $(DESTDIR)$(DATADIR)
	@if [ "$(ASSETS)" != "" ]; then \
		install -m 644 $(ASSETS) $(DESTDIR)$(DATADIR)/; \
	fi

uninstall:
	rm -f $(DESTDIR)$(BINDIR)/$(TARGET)
	@if [ -d "$(DESTDIR)$(DATADIR)" ]; then \
		rm -f $(DESTDIR)$(DATADIR)/style.css $(DESTDIR)$(DATADIR)/character*.png $(DESTDIR)$(DATADIR)/character*.jpg; \
		rmdir --ignore-fail-on-non-empty $(DESTDIR)$(DATADIR) 2>/dev/null || true; \
	fi
