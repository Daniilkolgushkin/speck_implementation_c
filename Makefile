PREFIX = /usr/local/bin
TARGET = speck_wrap
CXXFLAGS = -O3

.PHONY: all clean install uninstall hexifator

all: $(TARGET)

$(TARGET): crypto_wrap.cpp hexifator
	$(CXX) $(CXXFLAGS) $< -o $@
hexifator: hexifator.cpp
	$(CXX) $(CXXFLAGS) $^ -o $@
	install $@ $(PREFIX)
clean: $(TARGET) hexifator
	rm $^
install: $(TARGET)
	install $(TARGET) $(PREFIX)
uninstall:
	rm $(PREFIX)/$(TARGET)
	rm $(PREFIX)/hexifator
