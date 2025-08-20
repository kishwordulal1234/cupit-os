# 🔥 CUPIT Demon OS v1

A sophisticated web-based terminal operating system for ESP8266 devices that provides both serial and web-based command interfaces with file system capabilities.

![CUPIT Demon OS](https://img.shields.io/badge/Version-v1-blue)
![Platform](https://img.shields.io/badge/Platform-ESP8266-green)
![License](https://img.shields.io/badge/License-MIT-yellow)

## Features

### 🖥️ Dual Interface Support
- **Serial Terminal**: Connect via USB for direct command-line access
- **Web Terminal**: Access through any modern web browser via WiFi

### 📁 File System Operations
- SPIFFS (SPI Flash File System) integration
- Create, read, delete files (`touch`, `cat`, `rm`)
- Directory listing (`ls`, `dir`)
- File redirection with `echo > file`
- Disk usage monitoring (`df`)

### 🌐 Network Capabilities
- WiFi Access Point mode
- Web server on port 80
- Real-time command execution through web interface
- WiFi status monitoring

### 📊 System Monitoring
- Memory usage information (`free`)
- Process simulation (`ps`, `top`)
- System information display (`neofetch`)
- Uptime tracking
- Debug mode toggle

### 🎨 Responsive Web Interface
- Mobile-friendly responsive design
- Terminal-like appearance with customizable styling
- Quick command buttons for common operations
- Command history with arrow key navigation
- Real-time status updates

## Requirements

### Hardware
- ESP8266 Development Board (NodeMCU, Wemos D1 Mini, etc.)
- Micro USB cable for programming and power

### Software
- Arduino IDE with ESP8266 support
- Required Libraries:
  - ESP8266WiFi
  - ESP8266WebServer
  - FS (SPIFFS)

## Installation

### 1. Prepare Arduino IDE
1. Install Arduino IDE if you haven't already
2. Add ESP8266 board support:
   - **File** > **Preferences** > **Additional Boards Manager URLs**:
   ```
   http://arduino.esp8266.com/stable/package_esp8266com_index.json
   ```
   - **Tools** > **Board** > **Boards Manager** > Install **"ESP8266"**
   - Select your board: **Tools** > **Board** > **ESP8266 Boards** > **[Your Board Model]**

### 2. Upload the Sketch
1. Copy the code to your Arduino IDE
2. Connect your ESP8266 to your computer
3. Select the correct port in **Tools** > **Port**
4. Upload the sketch

### 3. First Boot
1. After upload, open Serial Monitor (115200 baud)
2. Wait for the system to boot (you'll see the boot messages)
3. The system will create a WiFi access point named **"DemonOS"**

## Usage

### Serial Interface
1. Connect to your ESP8266 via USB
2. Open Serial Monitor at 115200 baud
3. You'll see the CUPIT Demon OS prompt:
   ```
   root@CUPIT-demon:~# 
   ```
4. Type commands and press Enter to execute

### Web Interface
1. Connect your device to the **"DemonOS"** WiFi network
   - Password: `demon1234`
2. Open a web browser and navigate to `http://192.168.4.1`
3. You'll see the web terminal interface
4. Type commands in the input field and click Execute or press Enter

## Command Reference

### File System Commands
| Command | Description | Example |
|---------|-------------|---------|
| `ls` / `dir` | List files and directories | `ls` |
| `touch <file>` | Create a new file | `touch test.txt` |
| `rm <file>` | Delete a file | `rm test.txt` |
| `cat <file>` | Display file contents | `cat test.txt` |
| `echo <text>` | Display text or write to file | `echo Hello World` or `echo Hello > test.txt` |
| `mount` | Mount the SPIFFS filesystem | `mount` |
| `df` | Show disk usage | `df` |

### System Information Commands
| Command | Description |
|---------|-------------|
| `neofetch` | Display system information with ASCII art |
| `uname` | Show system version |
| `ps` | List running processes |
| `top` | System monitor |
| `free` | Show memory usage |
| `date` | Show system uptime |
| `whoami` | Show current user |
| `pwd` | Show current directory |

### Network Commands
| Command | Description |
|---------|-------------|
| `wifi` / `iwconfig` | Show WiFi status and connected clients |

### System Control Commands
| Command | Description |
|---------|-------------|
| `reboot` | Restart the system |
| `clear` | Clear the screen |
| `debug` | Toggle debug mode |
| `help` / `man` | Show available commands |

## Technical Details

### Architecture
- **CUPITKernel**: Handles serial input and command processing
- **Web Server**: Serves the HTML interface and processes commands
- **File System**: SPIFFS for persistent storage
- **Command Processor**: Interprets and executes commands

### Memory Management
- Optimized for ESP8266's limited memory
- Efficient string handling
- Smart buffer management

### Web Interface
- Single-page application with AJAX communication
- Responsive design for desktop and mobile
- Real-time command execution

## Configuration

### WiFi Settings
To change the WiFi credentials, modify these lines in the code:

```cpp
const char* ssid = "DemonOS";
const char* password = "demon1234";
IPAddress apIP(192, 168, 4, 1);
```

### IP Address
The default IP address is `192.168.4.1`. You can change it by modifying:

```cpp
IPAddress apIP(192, 168, 4, 1);
```

## Troubleshooting

### Common Issues

**Problem**: Can't connect to the WiFi access point
- **Solution**:
  - Check that the ESP8266 is powered properly
  - Verify the password is "demon1234"
  - Restart the ESP8266

**Problem**: Web interface not loading
- **Solution**:
  - Ensure you're connected to the "DemonOS" network
  - Try accessing `http://192.168.4.1`
  - Check Serial Monitor for error messages

**Problem**: File system commands not working
- **Solution**:
  - Run `mount` first to initialize the file system
  - Check if SPIFFS is properly formatted

**Problem**: Out of memory errors
- **Solution**:
  - Reduce the number of files
  - Keep file sizes small
  - Reboot the system

### Debug Mode
Enable debug mode by typing `debug` in the terminal. This will provide additional diagnostic information.

## Project Structure

```
CUPIT-Demon-OS/
├── CUPIT-Demon-OS.ino    # Main sketch file
├── README.md             # This file
└── data/                 # Files for SPIFFS (optional)
```

## License

This project is open source and available under the MIT License.

## Contributing

Contributions are welcome! Please feel free to submit a Pull Request.

## Acknowledgments

- ESP8266 community for the excellent libraries and support
- Arduino team for the great platform
- All the open-source contributors who made this project possible

## Author

Created by [unknone hart] - feel free to contact for questions or suggestions.

---

**Note**: This project is for educational and hobbyist purposes. Use responsibly and ensure you have permission before deploying on any network.
