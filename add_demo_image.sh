#!/bin/bash

# Quick script to add demo image while app is running
echo "📸 Adding demo image to Aetherwave..."

# Create assets directory if it doesn't exist
mkdir -p build/bin/assets/images

# Copy test image if it exists
if [ -f "test_image.jpg" ]; then
    cp test_image.jpg build/bin/assets/images/sample.jpg
    echo "✅ Added sample.jpg to assets directory"
    echo "💡 Type 'r' in the running Aetherwave app to reload images!"
else
    echo "❌ test_image.jpg not found"
fi

# List what's in the directory
echo "📁 Current images in assets:"
ls -la build/bin/assets/images/
