# Aetherwave MCP Server for Application Evaluation

A Model Context Protocol (MCP) server for monitoring and evaluating the Aetherwave visual display application.

## Features

- **Process Monitoring**: Detect if Aetherwave is running
- **Window Detection**: Verify actual GUI windows are created
- **Performance Metrics**: Track FPS, memory usage, CPU utilization
- **Image Display Verification**: Confirm images are being rendered
- **Feature Testing**: Validate fullscreen, transitions, multi-monitor support
- **API Integration**: Test Python classification API connectivity

## Installation

```bash
# Install MCP server dependencies
cd mcp_server
npm install

# Start the MCP server
npm start
```

## Usage

The MCP server provides tools for evaluating the Aetherwave application:

### Process Tools

- `check_aetherwave_process` - Verify if application is running
- `get_application_windows` - List active application windows
- `monitor_performance` - Real-time performance metrics

### Display Tools

- `verify_image_display` - Confirm images are being rendered
- `test_transitions` - Validate transition effects
- `check_fullscreen` - Test fullscreen mode functionality
- `detect_multi_monitor` - Verify multi-monitor support

### Integration Tools

- `test_api_connection` - Check Python classification API
- `validate_asset_sync` - Confirm asset synchronization
- `run_feature_tests` - Complete feature validation suite

## Implementation

The server uses:

- **Node.js** for MCP protocol implementation
- **macOS APIs** for window detection and process monitoring
- **SDL2 integration** for graphics performance monitoring
- **HTTP clients** for API testing

This provides objective validation that the Aetherwave engine is working as intended.
