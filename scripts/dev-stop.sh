#!/bin/bash
set -e

echo "🛑 Stopping Aetherwave development environment..."

docker-compose down

echo "✅ All services stopped."
echo "💡 Run 'scripts/dev-setup.sh' to start again."
