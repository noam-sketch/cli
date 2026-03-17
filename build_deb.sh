#!/bin/bash
set -e

# Build the C executable
make clean
make

# Setup variables
VERSION="1.0.1"
APP_NAME="cli"
DEB_DIR="deb_build"

echo "Creating debian package structure..."
rm -rf ${DEB_DIR}
mkdir -p ${DEB_DIR}/DEBIAN
mkdir -p ${DEB_DIR}/opt/${APP_NAME}/assets/fonts
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
Depends: libgtk-3-0, libvte-2.91-0, fontconfig
Description: A native GTK/VTE terminal emulator for Linux with split views.
EOF

# Desktop file
cat <<EOF > ${DEB_DIR}/usr/share/applications/${APP_NAME}.desktop
[Desktop Entry]
Version=1.0
Name=Cli
GenericName=Terminal Emulator
Comment=Native GTK/VTE terminal emulator with split views
Terminal=false
Type=Application
Categories=Utility;TerminalEmulator;System;
Exec=/opt/${APP_NAME}/cli
Icon=cli
EOF

# Copy binaries and assets
cp cli ${DEB_DIR}/opt/${APP_NAME}/
cp assets/icon.png ${DEB_DIR}/opt/${APP_NAME}/assets/icon.png
cp assets/fonts/UbuntuMono-Regular.ttf ${DEB_DIR}/opt/${APP_NAME}/assets/fonts/UbuntuMono-Regular.ttf
cp assets/icon.png ${DEB_DIR}/usr/share/icons/hicolor/256x256/apps/cli.png

# Create wrapper script to set working directory for assets
cat <<EOF > ${DEB_DIR}/usr/bin/cli
#!/bin/bash
cd /opt/cli
./cli "\$@"
EOF

# Ensure permissions
chmod 755 ${DEB_DIR}/DEBIAN/control
find ${DEB_DIR}/opt/${APP_NAME} -type d -exec chmod 755 {} \;
find ${DEB_DIR}/opt/${APP_NAME} -type f -exec chmod 644 {} \;
chmod +x ${DEB_DIR}/opt/${APP_NAME}/cli
chmod +x ${DEB_DIR}/usr/bin/cli
chmod -R 755 ${DEB_DIR}/usr

# Build package
dpkg-deb --build ${DEB_DIR}
mv ${DEB_DIR}.deb ${APP_NAME}_${VERSION}_amd64.deb

echo "Done! Package generated: ${APP_NAME}_${VERSION}_amd64.deb"
