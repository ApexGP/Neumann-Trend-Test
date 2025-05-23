#!/bin/bash
echo "Starting Neumann Trend Test CLI Tool..."

# Change to project root directory
cd "$(dirname "$0")"

# Check and create necessary directories
if [ ! -d "data" ]; then
    echo "Creating data directory..."
    mkdir -p data
fi

if [ ! -f "data/standard_values.json" ]; then
    echo "Warning: Standard values file not found, creating empty file..."
    echo "{}" >data/standard_values.json
fi

# Find CLI application
CLI_APP=""

if [ -f "bin/neumann_cli_app" ]; then
    CLI_APP="bin/neumann_cli_app"
elif [ -f "build/linux-release/bin/neumann_cli_app" ]; then
    echo "Warning: Using legacy build output path"
    CLI_APP="build/linux-release/bin/neumann_cli_app"
else
    echo "Error: CLI application executable not found"
    echo "Please ensure the project has been built successfully"
    read -p "Press Enter to continue..." KEY
    exit 1
fi

# Display summary information
echo "-----------------------------------------"
echo "Neumann Trend Test CLI Tool Launcher"
echo "-----------------------------------------"
echo "Executable: $CLI_APP"
echo "Working Directory: $(pwd)"
echo "Data Directory: $(pwd)/data"
echo "-----------------------------------------"
echo "Available command line options:"
echo "-f, --file [filepath]    Process CSV data file"
echo "-h, --help              Show help information"
echo "-----------------------------------------"

# Launch application
"$CLI_APP" "$@"

read -p "Press Enter to continue..." KEY
