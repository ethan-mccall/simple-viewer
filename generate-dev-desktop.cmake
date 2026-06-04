# This script is run at build time to generate a .desktop file
# with the actual absolute paths to the built executable and icon.

set(DESKTOP_FILE "${OUTPUT_DIR}/simple-viewer-dev.desktop")

file(WRITE "${DESKTOP_FILE}" "# Development .desktop with absolute paths.
# Copy this (or symlink) into ~/.local/share/applications/ for menu integration
# without doing a full install. Re-copy after rebuilds if the build dir changes.
# Run: update-desktop-database ~/.local/share/applications
[Desktop Entry]
Version=1.0
Type=Application
Name=Simple Viewer (dev)
GenericName=Image Viewer
Comment=Lightweight image viewer with crop, transforms, and folder navigation [DEV BUILD]
Exec=\"${BINARY}\" %F
Icon=${ICON}
Terminal=false
Categories=Graphics;Viewer;Photography;
MimeType=image/bmp;image/gif;image/jpeg;image/png;image/tiff;image/x-tga;image/webp;image/svg+xml;image/x-icon;image/heic;image/heif;image/avif;image/x-icns;
StartupNotify=true
StartupWMClass=SimpleViewer
Actions=
")

message(STATUS "Generated dev .desktop: ${DESKTOP_FILE}")