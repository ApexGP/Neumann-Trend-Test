# Neumann Trend Test Tool

> **Language / 语言**: **English** | [中文](README_zh.md)

[![Build Status](https://img.shields.io/badge/build-passing-brightgreen.svg)](https://github.com/Xyf0606/Neumann-Trend-Test)
[![Version](https://img.shields.io/badge/version-3.0.0-blue.svg)](https://github.com/Xyf0606/Neumann-Trend-Test/releases)
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

- **Modern Data Input**: Excel-style dual-pane data editor with real-time editing and validation
- **Intelligent Configuration**: Isolated user settings with permission-aware management
- **Batch Processing**: Directory-level batch analysis with smart file organization and bilingual export
- **Data Visualization**: ASCII/SVG charts with export capabilities (CSV/HTML)
- **Modern UI**: Colorized terminal output, status bars, formatted tables, and dynamic terminal adaptation

### 🔧 v3.0.0 New Features

- **Modern Data Input Interface**: Brand new Excel-style data editor

  - ✅ Dual-pane input: Time points and data values displayed side by side, like Excel worksheets
  - ✅ Modern keyboard navigation: ↑↓ move rows, ←→ switch panes, Enter to edit, Ctrl+S to save
  - ✅ Smart terminal adaptation: Dynamic row count adjustment, auto-scroll for large datasets
  - ✅ Real-time status feedback: Visual editing status, colorized interface showing current operations
  - ✅ Operation convenience: Built-in help system with clear operation guidelines

- **User Experience Optimization**: Complete reconstruction of data input workflow

  - ✅ Removed traditional command-line individual input mode, providing modern editing interface
  - ✅ Enhanced file browser with fixed dynamic display line limitations
  - ✅ Smart data validation ensuring input data integrity and correctness
  - ✅ Friendly shortcut operations reducing learning curve and improving efficiency

- **Technical Architecture Enhancement**: Modern terminal interface based on FTXUI

  - ✅ No complex third-party libraries required, implemented with standard FTXUI
  - ✅ Cross-platform compatibility supporting Windows, Linux, macOS
  - ✅ Efficient memory management supporting smooth editing of large datasets
  - ✅ Modular design with configurable and extensible architecture

## 🎬 Feature Preview

### 📋 Modern Data Editor - Excel Lovers' Dream Come True

_Say goodbye to traditional command-line input, embrace Excel-style data input experience_ ⚡

<video width="100%" height="100%" autoplay loop muted>
  <source src="assets/zh_CN/editor.mp4" type="video/mp4">
</video>

https://github.com/user-attachments/assets/8382115a-d032-4bd2-a3fa-71074b3ec57b

<details>
<summary>📋 <strong>Modern Data Input Demo Details</strong></summary>

- **Dual-pane side-by-side input**: Time points and data values displayed simultaneously, like Excel worksheets
- **Modern keyboard navigation**: ↑↓ move rows, ←→ switch panes, Enter to edit, Ctrl+S to save
- **Smart terminal adaptation**: Dynamic row count adjustment, auto-scroll browsing for large datasets
- **Real-time status feedback**: Visualized editing state, colorized interface showing current operations
- **Operation convenience**: Built-in help system with clear, intuitive operation guidelines

</details>

---

### 📁 Smart File Browser - Dynamic Adaptive Modern Navigation

_File browsing and selection made simple, say goodbye to endless clicking_ 🗂️

<video width="100%" height="100%" autoplay loop muted>
  <source src="assets/zh_CN/file-browser.mp4" type="video/mp4">
</video>

https://github.com/user-attachments/assets/9a4461a6-d64a-4491-a2bc-2f87475af353

<details>
<summary>📋 <strong>Smart File Browser Demo Details</strong></summary>

- **Dynamic row count**: Automatically adjusts displayed items based on terminal size (15-45 rows)
- **Modern keyboard navigation**: ↑↓ select files, ←→ navigate directories, Enter to confirm, q to quit
- **File type recognition**: Colorized icons distinguish directories, files, archives, and other types
- **Scroll indicators**: Clear display of current viewing range and total file count

</details>

---

### 🚀 One-Click Launch - Zero to Analysis in 3 Seconds!

_So simple, even your boss can use it_ 😎

<video width="100%" height="100%" autoplay loop muted>
  <source src="assets/zh_CN/startup.mp4" type="video/mp4">
</video>

https://github.com/user-attachments/assets/1c2286bc-bd08-4dd2-b907-ede47b4e40c2

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

https://github.com/user-attachments/assets/59129d8d-25f2-4558-8cb3-a2d56b55b6a2

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

https://github.com/user-attachments/assets/10787d10-9b9e-473b-853b-9d516a224c20

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

https://github.com/user-attachments/assets/61fa2714-9015-4f53-a02e-9bda550ec2cc

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

https://github.com/user-attachments/assets/af3c8542-f1c7-40ba-852a-625bccbe95d4

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

https://github.com/user-attachments/assets/cb455ee6-dd2e-411f-8ad6-776ac35370bf

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

https://github.com/user-attachments/assets/771c70a0-bec0-417c-ab7a-f08ef48cc9b8

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

https://github.com/user-attachments/assets/af128832-268c-41ef-bbd7-c13a06287c6b

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
- **Modern Manual Input**: Brand new Excel-style dual-pane data editor

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
| Data editor not responding       | Use v3.0.0, brand new modern input interface             |

### Performance Recommendations

- **Large Datasets**: Use batch processing mode
- **Frequent Analysis**: Save commonly used datasets
- **Web Performance**: CLI recommended for large datasets
- **Data Input**: Use the new data editor for the best experience

For more issues, please check [FAQ](https://github.com/Xyf0606/Neumann-Trend-Test/blob/main/docs/FAQ.md) or submit an [Issue](https://github.com/Xyf0606/Neumann-Trend-Test/issues).

## 📦 Version Information

**Current Version**: v3.0.0  
**Release Date**: June 2, 2025

**Major Updates**:

- 🚀 **Modern Data Input Revolution**: Introduced Excel-style dual-pane data input interface, completely transforming data input experience
- ✨ **Significant User Experience Enhancement**: Modern keyboard navigation, smart terminal adaptation, real-time status feedback
- 🛠️ **Technical Architecture Modernization**: New DataEditor component based on FTXUI framework with complete internationalization support

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
