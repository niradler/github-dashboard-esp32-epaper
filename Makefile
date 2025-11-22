.PHONY: help install verify lint compile upload monitor clean all setup check intellisense

SKETCH_NAME = github-dash-epaper
FQBN = esp32:esp32:esp32
PORT = COM6
BAUD_RATE = 115200

ARDUINO_CLI := $(shell command -v arduino-cli 2> /dev/null)
ARDUINO_LINT := $(shell command -v arduino-lint 2> /dev/null)

help:
	@echo "GitHub Dashboard E-Paper - Makefile"
	@echo "===================================="
	@echo ""
	@echo "Available targets:"
	@echo "  make setup         - Install Arduino CLI and required cores/libraries"
	@echo "  make check         - Verify all tools are installed"
	@echo "  make intellisense  - Generate compile_commands.json for IDE IntelliSense"
	@echo "  make lint          - Run arduino-lint on the project"
	@echo "  make verify        - Compile without uploading"
	@echo "  make compile       - Alias for verify"
	@echo "  make analyze       - Deep code analysis for unused code and warnings"
	@echo "  make upload        - Compile and upload to board"
	@echo "  make monitor       - Open serial monitor"
	@echo "  make all           - Lint, compile, and upload"
	@echo "  make clean         - Clean build cache"
	@echo "  make info          - Show board and project info"
	@echo ""

check:
	@echo "Checking required tools..."
ifndef ARDUINO_CLI
	@echo "❌ arduino-cli not found. Install from: https://arduino.github.io/arduino-cli/latest/installation/"
	@exit 1
else
	@echo "✓ arduino-cli found: $(ARDUINO_CLI)"
	@arduino-cli version
endif
ifndef ARDUINO_LINT
	@echo "⚠ arduino-lint not found (optional). Install from: https://arduino.github.io/arduino-lint/latest/installation/"
else
	@echo "✓ arduino-lint found: $(ARDUINO_LINT)"
	@arduino-lint --version
endif
	@echo ""
	@echo "✓ All required tools found"

setup: check
	@echo "Setting up Arduino environment..."
	@echo ""
	@echo "Updating core index..."
	arduino-cli core update-index
	@echo ""
	@echo "Installing ESP32 core (this may take a while)..."
	arduino-cli core install esp32:esp32 || echo "ESP32 core already installed"
	@echo ""
	@echo "Installing required libraries..."
	arduino-cli lib install "GxEPD2" || echo "GxEPD2 already installed"
	arduino-cli lib install "U8g2_for_Adafruit_GFX" || echo "U8g2_for_Adafruit_GFX already installed"
	arduino-cli lib install "ArduinoJson" || echo "ArduinoJson already installed"
	@echo ""
	@echo "✓ Setup complete!"

lint:
ifdef ARDUINO_LINT
	@echo "Running arduino-lint..."
	arduino-lint --compliance strict --library-manager update .
else
	@echo "⚠ arduino-lint not installed, skipping lint check"
	@echo "Install from: https://arduino.github.io/arduino-lint/latest/installation/"
endif

verify: check
	@echo "Compiling $(SKETCH_NAME)..."
	arduino-cli compile --fqbn $(FQBN) --warnings all .
	@echo ""
	@echo "✓ Compilation successful!"

compile: verify

analyze: check
	@echo "Analyzing code for unused variables, functions, and warnings..."
	arduino-cli compile --fqbn $(FQBN) --warnings more --verbose .

upload: check
	@echo "Compiling and uploading $(SKETCH_NAME) to $(PORT)..."
	arduino-cli compile --fqbn $(FQBN) --upload --port $(PORT) .
	@echo ""
	@echo "✓ Upload successful!"

monitor:
	@echo "Opening serial monitor on $(PORT) at $(BAUD_RATE) baud..."
	@echo "Press Ctrl+C to exit"
	arduino-cli monitor --port $(PORT) --config baudrate=$(BAUD_RATE)

clean:
	@echo "Cleaning build cache..."
	arduino-cli cache clean
	@echo "✓ Cache cleaned"

info: check
	@echo "Project Information"
	@echo "==================="
	@echo "Sketch:      $(SKETCH_NAME)"
	@echo "FQBN:        $(FQBN)"
	@echo "Port:        $(PORT)"
	@echo "Baud Rate:   $(BAUD_RATE)"
	@echo ""
	@echo "Detecting boards..."
	arduino-cli board list
	@echo ""
	@echo "Installed cores:"
	arduino-cli core list
	@echo ""
	@echo "Installed libraries:"
	arduino-cli lib list

all: lint verify upload
	@echo ""
	@echo "✓ All tasks completed successfully!"

boards:
	@echo "Detecting connected boards..."
	arduino-cli board list

cores:
	@echo "Installed Arduino cores:"
	arduino-cli core list

libs:
	@echo "Installed libraries:"
	arduino-cli lib list

intellisense:
	@echo "Generating compile_commands.json for IDE IntelliSense..."
	@arduino-cli compile --fqbn $(FQBN) --build-path ./build --only-compilation-database .
	@if exist "build\compile_commands.json" ( \
		copy /Y build\compile_commands.json . >nul && \
		echo ✓ compile_commands.json generated successfully! && \
		echo   Reload VS Code/Cursor window for IntelliSense to pick it up. \
	) else ( \
		echo ✗ Failed to generate compile_commands.json \
	)


