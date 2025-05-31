# Neumann Trend Test Tool

> **Language / 语言**: **English** | [中文](README_zh.md)

[![Build Status](https://img.shields.io/badge/build-passing-brightgreen.svg)](https://github.com/Xyf0606/Neumann-Trend-Test)
[![Version](https://img.shields.io/badge/version-2.9.0-blue.svg)](https://github.com/Xyf0606/Neumann-Trend-Test/releases)
[![Language](https://img.shields.io/badge/language-C%2B%2B-orange.svg)](https://isocpp.org/)
[![License](https://img.shields.io/badge/license-MIT-green.svg)](LICENSE)

A professional statistical analysis tool implementing the Neumann trend test for detecting systematic trends in datasets. Widely used in pharmaceutical stability studies and quality control applications.

## 📋 Table of Contents

- [✨ Features](#-features)
- [🎬 Feature Preview](#-feature-preview)
- [🚀 Quick Start](#-quick-start)
- [📖 User Guide](#-user-guide)
- [🛠️ Building from Source](#️-building-from-source)
- [🔧 Configuration](#-configuration)
- [📊 Data Format](#-data-format)
- [🐛 Troubleshooting](#-troubleshooting)
- [📦 Version Information](#-version-information)
- [🤝 Contributing](#-contributing)
- [📄 License](#-license)

## ✨ Features

### 🔬 Core Functionality

- **Statistical Analysis**: Support for 95%, 99%, 99.9% confidence levels with 4-60 data points
- **Bilingual Interface**: Complete Chinese/English support with runtime language switching
- **Cross-Platform**: Native support for Windows, Linux, and macOS
- **Dual Interface**: Both command-line and web-based interfaces to suit different workflows

### 🎯 User Experience

- **Intelligent Configuration**: Isolated user settings with permission-aware management
- **Batch Processing**: Directory-level batch analysis with smart file organization and bilingual export
- **Data Visualization**: ASCII/SVG charts with export capabilities (CSV/HTML)
- **Modern UI**: Colorized terminal output, status bars, and formatted tables

### 🔧 v2.9.0 New Features

- **Enhanced Excel File Support**: True Excel file import functionality

  - ✅ Complete support for .xlsx file format with real Excel file reading
  - ✅ ZIP-based Excel file parsing supporting complex Excel structures
  - ✅ Smart worksheet detection with multi-sheet file and worksheet selection support
  - ✅ Automatic column type recognition with intelligent time and data column detection
  - ✅ Shared string table support for proper handling of text content in Excel
  - ✅ Data preview functionality to view file content before import
  - ✅ Comprehensive error handling with fallback to CSV format suggestions

- **User Experience Optimization**: Complete reconstruction of Excel import workflow

  - ✅ Removed hardcoded "unsupported" messages, providing true Excel support
  - ✅ Enhanced file format detection automatically recognizing .xlsx, .xls, .csv formats
  - ✅ Smart data validation ensuring import data integrity and correctness
  - ✅ Friendly error messages with clear solution suggestions

- **Technical Architecture Enhancement**: Modern Excel processing engine

  - ✅ No third-party Excel libraries required, implemented with standard C++
  - ✅ Cross-platform compatibility supporting Windows, Linux, macOS
  - ✅ Efficient memory management supporting large Excel file processing
  - ✅ Automatic temporary file cleanup preventing disk space occupation

## 🎬 Feature Preview

### 🚀 One-Click Launch - Zero to Analysis in 3 Seconds!

_So simple, even your boss can use it_ 😎

<video width="100%" height="100%" autoplay loop muted>
  <source src="assets/zh_CN/startup.mp4" type="video/mp4">
</video>

![Startup Demo](assets/zh_CN/startup.mp4)

<details>
<summary>📋 <strong>Startup Demo Details</strong></summary>

- Double-click `start.bat` to launch the application
- Automatic configuration detection and initialization
- Choose between Web interface or CLI interface
- Web interface automatically opens in browser

</details>

### 💻 CLI Interface - Command Line Paradise for Geeks

_Hierarchical menus, one-click navigation, beloved by professional data analysts_ ⚡

<video width="100%" height="100%" autoplay loop muted>
  <source src="assets/zh_CN/cli-interface.mp4" type="video/mp4">
</video>

![CLI Interface](assets/zh_CN/cli-interface.mp4)

<details>
<summary>📋 <strong>CLI Demo Details</strong></summary>

- Colorized terminal interface with Chinese/English switching support
- Real-time status bar showing current confidence level settings
- Tabular result display with ASCII art charts
- Keyboard shortcuts for efficient data analysis

</details>

### 🌐 Web Interface - Modern Analysis Experience

_Beauty meets functionality, making data analysis a visual delight_ ✨

<video width="100%" height="100%" autoplay loop muted>
  <source src="assets/zh_CN/web-interface.mp4" type="video/mp4">
</video>

![Web Interface](assets/zh_CN/web-interface.mp4)

<details>
<summary>📋 <strong>Web Demo Details</strong></summary>

- Responsive modern UI design
- Drag-and-drop file upload
- Real-time chart visualization
- Interactive result presentation

</details>

### 📊 Data Analysis - The Magic of Turning Data into Insights

_Witness the moment when data tells its story_ 🔮

<video width="100%" height="100%" autoplay loop muted>
  <source src="assets/zh_CN/data-analysis.mp4" type="video/mp4">
</video>

![Data Analysis](assets/zh_CN/data-analysis.mp4)

<details>
<summary>📋 <strong>Analysis Demo Details</strong></summary>

- Import CSV data files
- Real-time Neumann trend test calculations
- PG value visualization chart generation
- Trend determination result display

</details>

### ⚡ Batch Processing - Efficiency Enthusiast's Dream

_Process an entire folder at once, say goodbye to repetitive work_ 🎯

<video width="100%" height="100%" autoplay loop muted>
  <source src="assets/zh_CN/batch-processing.mp4" type="video/mp4">
</video>

![Batch Processing](assets/zh_CN/batch-processing.mp4)

<details>
<summary>📋 <strong>Batch Processing Demo Details</strong></summary>

- Select directories containing multiple data files
- Parallel processing of multiple datasets
- Real-time progress bar showing processing status
- Smart file categorization: CSV and HTML automatically saved to dedicated directories
- Complete bilingual support: identical Chinese and English export content
- UTF-8 encoding optimization: ensures proper Chinese display in Excel

</details>

### 🌍 Internationalization Support - Professional Tool for Global Use

_One-click Chinese/English switching, internationally sophisticated_ 🌏

<video width="100%" height="100%" autoplay loop muted>
  <source src="assets/zh_CN/i18n.mp4" type="video/mp4">
</video>

![Internationalization Support](assets/zh_CN/i18n.mp4)

<details>
<summary>📋 <strong>Internationalization Demo Details</strong></summary>

- Runtime language switching (Chinese ↔ English)
- Complete translation of all interface elements
- Persistent configuration storage
- No restart required for language changes

</details>

### 🎨 Data Visualization - Bringing Data to Life

_ASCII art + SVG charts, a dual visual feast_ 📈

<video width="100%" height="100%" autoplay loop muted>
  <source src="assets/zh_CN/visualization.mp4" type="video/mp4">
</video>

![Data Visualization](assets/zh_CN/visualization.mp4)

<details>
<summary>📋 <strong>Visualization Demo Details</strong></summary>

- ASCII art charts in CLI
- Dynamic SVG charts in web interface
- Multiple chart types (trend charts, distribution charts)
- High-quality export functionality

</details>

### ⚙️ Smart Configuration - Seamless Cross-Platform Sync

_Set once, use forever_ 💝

<video width="100%" height="100%" autoplay loop muted>
  <source src="assets/zh_CN/config-management.mp4" type="video/mp4">
</video>

![Configuration Management](assets/zh_CN/config-management.mp4)

<details>
<summary>📋 <strong>Configuration Management Demo Details</strong></summary>

- Visual confidence level setting interface
- Intelligent configuration file management
- Separation of user and system configurations
- Configuration import/export functionality

</details>

## 🚀 Quick Start

### Method 1: Download Pre-compiled Release (Recommended)

1. **Download Release Package**

   - Visit the [Releases page](https://github.com/Xyf0606/Neumann-Trend-Test/releases)
   - Download the latest `NTT-x86_64-windows` archive
   - ⚠️ Linux packages are not currently available, please build from source
   - Extract files to any location

2. **Launch Application**

   ```bash
   # Windows
   start.bat

   # Linux/macOS
   ./start.sh
   ```

3. **Start Using**
   - CLI interface: Begin data analysis immediately
   - Web interface: Select "Start Web Server" in CLI

### Method 2: Docker (Coming Soon)

```bash
docker run -p 8080:8080 neumann-trend-test:latest
```

## 📖 User Guide

### Basic Workflow

1. **Data Preparation**

   ```csv
   Time,Value
   0,99.95
   1,99.88
   2,99.82
   3,99.76
   ```

2. **Run Analysis**

   - **Interactive**: Launch application → Select "Run New Neumann Trend Test"
   - **Batch Processing**: Select "Batch Data Processing" → Specify directory
   - **Command Line**: `./bin/neumann_cli_app.exe -f data.csv`

3. **View Results**
   - Trend determination: Whether significant trend exists
   - PG value analysis: Statistical values for each test point
   - Visualization charts: Trend and distribution plots

### Using the Web Interface

1. Select "Start Web Server" in CLI
2. Navigate to `http://localhost:8080` in browser
3. Upload data files or manually input data
4. View real-time analysis results and charts

### Configuration Management

- **Confidence Level Settings**: Settings → Confidence Configuration → Select or customize
- **Language Switching**: Settings → Language Settings → Chinese/English
- **Configuration File**: Automatically saved to `data/usr/config_private.json`

## 🛠️ Building from Source

### System Requirements

- C++17 compatible compiler
- CMake 3.15+
- vcpkg (recommended for Windows)

### Windows Build

```bash
# 1. Install dependencies
vcpkg install nlohmann-json:x64-windows ftxui:x64-windows crow:x64-windows

# 2. Build project
build.bat mingw release    # MinGW
build.bat msvc release     # MSVC
```

### Linux/macOS Build

```bash
# 1. Install dependencies
sudo apt install nlohmann-json3-dev    # Ubuntu
brew install nlohmann-json             # macOS

# 2. Manually build FTXUI and Crow (refer to official documentation)

# 3. Build project
./build.sh
```

For detailed build instructions, please refer to [Build Documentation](https://github.com/Xyf0606/Neumann-Trend-Test/blob/main/docs/BUILD.md).

## 🔧 Configuration

### Configuration File Locations

- **User Configuration**: `data/usr/config_private.json` (Priority)
- **System Configuration**: `config/config.json` (Fallback)

### Main Configuration Options

```json
{
  "language": "zh", // Interface language
  "defaultConfidenceLevel": 0.95, // Default confidence level
  "dataDirectory": "data", // Data directory
  "defaultWebPort": 8080, // Web port
  "enableColorOutput": true, // Color output
  "maxDataPoints": 1000 // Data point limit
}
```

### Smart Configuration System

- **Configuration Isolation**: User settings separated from system defaults
- **Permission Detection**: Automatic fallback to read-only mode when permissions are insufficient
- **Automatic Migration**: Old version configurations automatically upgraded

## 📊 Data Format

### Supported Formats

- **CSV Files**: Fully supported, recommended format, compatible with Excel exports
- **Excel Files**: ✅ Complete support for .xlsx format, ⚠️ .xls format recommended to convert to .xlsx

### Data Requirements

- **Minimum Data Points**: 4 (statistical test minimum requirement)
- **Maximum Data Points**: 60 (based on standard value table coverage)
- **Data Type**: Numeric (integers/decimals)
- **Time Column**: Optional, defaults to incremental sequence

### Sample Data

```csv
Time,Drug Content
0,100.00
3,99.85
6,99.72
9,99.58
12,99.43
```

## 🐛 Troubleshooting

### Common Issues

| Issue                            | Solution                                                 |
| -------------------------------- | -------------------------------------------------------- |
| Standard values file not found   | Ensure `ref/standard_values.json` exists                 |
| Web interface inaccessible       | Check port usage: `netstat -an \| grep 8080`             |
| Cannot save configuration        | Check `data/usr/` directory permissions                  |
| Color output issues              | Set `"enableColorOutput": false`                         |
| Confidence level display error   | Update to v2.2.1 or later                                |
| CSV Chinese character corruption | Use v2.8.0, UTF-8 BOM support added                      |
| Batch processing files not found | Check `data/csv/` and `data/html/` directory permissions |
| Excel file import failure        | Use v2.9.0, complete support for .xlsx format added      |

### Performance Recommendations

- **Large Datasets**: Use batch processing mode
- **Frequent Analysis**: Save commonly used datasets
- **Web Performance**: CLI recommended for large datasets

For more issues, please check [FAQ](https://github.com/Xyf0606/Neumann-Trend-Test/blob/main/docs/FAQ.md) or submit an [Issue](https://github.com/Xyf0606/Neumann-Trend-Test/issues).

## 📦 Version Information

**Current Version**: v2.9.0  
**Release Date**: January 2, 2025

**Major Updates**:

- 🔧 **Enhanced Excel File Support**: Implemented true .xlsx file reading functionality supporting complex Excel structures
- ✨ **Enhanced User Experience**: Removed hardcoded limitations providing complete Excel import workflow
- 🛠️ **Technical Architecture Enhancement**: Modern cross-platform Excel processing engine using C++

**Update History**: [CHANGELOG.md](CHANGELOG.md)

## 🤝 Contributing

We welcome all forms of contributions!

### How to Contribute

1. **Report Issues**: [Submit an Issue](https://github.com/Xyf0606/Neumann-Trend-Test/issues)
2. **Feature Suggestions**: Describe requirements and use cases
3. **Code Contributions**: Fork → Develop → Test → Pull Request

### Development Environment

```bash
git clone https://github.com/Xyf0606/Neumann-Trend-Test.git
cd Neumann-Trend-Test
./build.sh debug
ctest --preset linux-debug
```

### Contribution Guidelines

- Follow existing code style
- Add appropriate test cases
- Update relevant documentation
- Ensure CI passes

Detailed guidelines: [CONTRIBUTING.md](https://github.com/Xyf0606/Neumann-Trend-Test/blob/main/docs/CONTRIBUTING.md)

## 📄 License

This project is licensed under the [MIT License](LICENSE).

---

<div align="center">

**⭐ If this project helps you, please give us a Star! ⭐**

[🏠 Homepage](https://github.com/Xyf0606/Neumann-Trend-Test) •
[📖 Documentation](https://github.com/Xyf0606/Neumann-Trend-Test/blob/main/docs/) •
[🐛 Report Issues](https://github.com/Xyf0606/Neumann-Trend-Test/issues) •
[💬 Discussions](https://github.com/Xyf0606/Neumann-Trend-Test/discussions)

</div>
