#!/bin/bash
set -e

# Run flutter build
flutter build linux --release

# Setup variables
VERSION="1.0.0"
APP_NAME="cli"
DEB_DIR="deb_build"
BUNDLE_DIR="build/linux/x64/release/bundle"

echo "Creating debian package structure..."
rm -rf ${DEB_DIR}
mkdir -p ${DEB_DIR}/DEBIAN
mkdir -p ${DEB_DIR}/opt/${APP_NAME}
mkdir -p ${DEB_DIR}/usr/bin
mkdir -p ${DEB_DIR}/usr/share/applications
mkdir -p ${DEB_DIR}/usr/share/icons/hicolor/256x256/apps

# Control file
cat <<EOF > ${DEB_DIR}/DEBIAN/control
Package: cli
Version: ${VERSION}
Section: utils
Priority: optional
Architecture: amd64
Maintainer: Your Name <you@example.com>
Description: A GPU accelerated terminal emulator for Linux with split views.
EOF

# Desktop file
cat <<EOF > ${DEB_DIR}/usr/share/applications/${APP_NAME}.desktop
[Desktop Entry]
Version=1.0
Name=Cli
GenericName=Terminal Emulator
Comment=GPU accelerated terminal emulator with split views
Terminal=false
Type=Application
Categories=Utility;TerminalEmulator;System;
Exec=/usr/bin/cli
Icon=cli
EOF

# Copy bundle
cp -r ${BUNDLE_DIR}/* ${DEB_DIR}/opt/${APP_NAME}/

# Copy icon
cp assets/icon.png ${DEB_DIR}/usr/share/icons/hicolor/256x256/apps/cli.png

# Create symlink
ln -s /opt/${APP_NAME}/cli ${DEB_DIR}/usr/bin/cli

# Ensure permissions
chmod 755 ${DEB_DIR}/DEBIAN/control
find ${DEB_DIR}/opt/${APP_NAME} -type d -exec chmod 755 {} \;
find ${DEB_DIR}/opt/${APP_NAME} -type f -exec chmod 644 {} \;
chmod +x ${DEB_DIR}/opt/${APP_NAME}/cli
chmod -R 755 ${DEB_DIR}/usr

# Build package
dpkg-deb --build ${DEB_DIR}
mv ${DEB_DIR}.deb ${APP_NAME}_${VERSION}_amd64.deb

echo "Done! Package generated: ${APP_NAME}_${VERSION}_amd64.deb"
