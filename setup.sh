#!/bin/bash
set -e

echo "🧱 Setting up GBA build environment..."

# --- 1. Install dependencies ---
echo "📦 Installing dependencies..."
sudo apt update -y
sudo apt install -y git build-essential curl

# --- 2. Prepare devkitPro directory ---
echo "📁 Preparing /opt/devkitpro..."
sudo mkdir -p /opt/devkitpro
sudo chown $USER /opt/devkitpro

# --- 3. Install devkitPro pacman if missing ---
if ! command -v dkp-pacman &> /dev/null; then
    echo "⬇️ Installing devkitPro pacman..."
    bash <(curl -L https://apt.devkitpro.org/install-devkitpro-pacman)
else
    echo "✅ devkitPro pacman already installed."
fi

# --- 4. Install GBA toolchain (devkitARM + libgba) ---
echo "🧰 Installing gba-dev (devkitARM + libgba)..."
sudo dkp-pacman -S --needed --noconfirm gba-dev

# --- 5. Ensure environment variables are persistent ---
if ! grep -q "DEVKITPRO" ~/.bashrc; then
    cat <<'EOF' >> ~/.bashrc

# >>> devkitPro environment setup >>>
export DEVKITPRO=/opt/devkitpro
export DEVKITARM=${DEVKITPRO}/devkitARM
export PATH=${DEVKITARM}/bin:$PATH
# <<< devkitPro environment setup <<<
EOF
    echo "✅ Added devkitPro environment variables to ~/.bashrc"
fi

# Apply for current shell
export DEVKITPRO=/opt/devkitpro
export DEVKITARM=${DEVKITPRO}/devkitARM
export PATH=${DEVKITARM}/bin:$PATH

# --- 6. Verify compiler works ---
if ! command -v arm-none-eabi-gcc &> /dev/null; then
    echo "❌ arm-none-eabi-gcc not found! Installation failed."
    exit 1
else
    echo "✅ Compiler found: $(arm-none-eabi-gcc --version | head -n 1)"
fi

# --- 7. Check for libgba includes ---
if [ ! -f "$DEVKITPRO/libgba/include/gba_video.h" ]; then
    echo "❌ libgba headers not found! Installing manually..."
    sudo dkp-pacman -S --noconfirm libgba
else
    echo "✅ libgba found."
fi

# --- 8. Patch Makefile if it exists ---
if [ -f "Makefile" ]; then
    echo "🧩 Patching Makefile to ensure libgba includes and libs..."

    # Backup first
    cp Makefile Makefile.backup

    # Remove absolute gcc paths if present
    sed -i 's|/opt/devkitpro/devkitARM/bin/||g' Makefile

    # Add proper include/lib flags if missing
    if ! grep -q "LIBGBA" Makefile; then
        cat <<'EOF' | cat - Makefile > temp && mv temp Makefile
LIBGBA := /opt/devkitpro/libgba
INCLUDES := -I$(LIBGBA)/include
LIBS := -L$(LIBGBA)/lib -lgba

EOF
    fi

    # Ensure compile command uses INCLUDES + LIBS
    sed -i 's|\$(CC) \(.*\) -o \(.*\)|$(CC) \1 $(INCLUDES) -o \2 $(LIBS)|g' Makefile

    echo "✅ Makefile patched successfully."
else
    echo "ℹ️ No Makefile found, skipping patch."
fi

# --- 9. Build project if Makefile present ---
if [ -f "Makefile" ]; then
    echo "🚀 Building your project..."
    make clean || true
    make
    echo "🎮 Build completed successfully!"
else
    echo "ℹ️ No Makefile detected — environment setup complete."
fi

echo "✅ All done! Restart terminal or run 'source ~/.bashrc' to finalize environment."

