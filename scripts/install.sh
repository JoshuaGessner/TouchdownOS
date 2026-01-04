#!/bin/bash
# Installation script for TouchdownOS on Raspberry Pi

set -e

RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m'

echo -e "${GREEN}TouchdownOS Installation Script${NC}"
echo "===================================="
echo

# Check if running on Raspberry Pi
if [ ! -f /proc/device-tree/model ]; then
    echo -e "${RED}Error: Not running on Raspberry Pi${NC}"
    exit 1
fi

PI_MODEL=$(cat /proc/device-tree/model)
echo "Detected: $PI_MODEL"
echo

# Check for package
if [ ! -f touchdownos_*.deb ]; then
    echo -e "${RED}Error: TouchdownOS .deb package not found${NC}"
    echo "Please copy the package to this directory first"
    exit 1
fi

PACKAGE=$(ls touchdownos_*.deb | head -n 1)
echo -e "${YELLOW}Installing package: $PACKAGE${NC}"

# Install package
sudo dpkg -i $PACKAGE || {
    echo -e "${YELLOW}Installing missing dependencies...${NC}"
    sudo apt-get update
    sudo apt-get install -f -y
}

# Configure device tree overlay
echo -e "${YELLOW}Configuring device tree overlay...${NC}"

# Backup config.txt
sudo cp /boot/config.txt /boot/config.txt.backup

# Add TouchdownOS configuration if not already present
if ! grep -q "touchdown-overlay" /boot/config.txt; then
    echo -e "${YELLOW}Adding TouchdownOS configuration to /boot/config.txt${NC}"
    sudo tee -a /boot/config.txt > /dev/null <<EOF

# TouchdownOS Configuration
dtparam=spi=on
dtparam=i2c_arm=on
dtoverlay=touchdown-overlay
dtoverlay=vc4-kms-v3d,nohdmi
gpu_mem=64
EOF
fi

# Compile device tree overlay if needed
if [ -f /usr/share/touchdown/dts/touchdown-overlay.dts ]; then
    echo -e "${YELLOW}Compiling device tree overlay...${NC}"
    sudo dtc -@ -I dts -O dtb \
        -o /boot/overlays/touchdown-overlay.dtbo \
        /usr/share/touchdown/dts/touchdown-overlay.dts
fi

# Enable services
echo -e "${YELLOW}Enabling TouchdownOS services...${NC}"
sudo systemctl daemon-reload
sudo systemctl enable touchdown-power.service
sudo systemctl enable touchdown-input.service
sudo systemctl enable touchdown-shell.service

# Disable getty (console) on tty1 if needed
echo -e "${YELLOW}Disabling console on tty1...${NC}"
sudo systemctl disable getty@tty1.service 2>/dev/null || true

echo
echo -e "${GREEN}Installation complete!${NC}"
echo
echo "Next steps:"
echo "  1. Reboot the system: sudo reboot"
echo "  2. Services will start automatically after boot"
echo
echo "To check status after reboot:"
echo "  systemctl status touchdown-shell.service"
echo
echo "To view logs:"
echo "  journalctl -u touchdown-shell.service -f"
echo
echo -e "${YELLOW}Note: A reboot is required for device tree changes to take effect${NC}"
