#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <FS.h>

const char* ssid = "DemonOS";
const char* password = "demon1234";
IPAddress apIP(192, 168, 4, 1);
ESP8266WebServer server(80);
char command[128] = {0};
unsigned long bootTime = 0;
bool debug = false;
bool spiffsReady = false;

char* formatUptime(unsigned long ms, char* buf, size_t len) {
  unsigned long seconds = ms / 1000;
  unsigned long minutes = seconds / 60;
  unsigned long hours = minutes / 60;
  unsigned long days = hours / 24;
  snprintf(buf, len, "%lud %luh %lum %lus", days, hours % 24, minutes % 60, seconds % 60);
  return buf;
}

void printOutput(const char* text, bool isSerial, Stream &output) {
  if (text && *text) {
    if (isSerial && debug) Serial.print("[DEBUG] ");
    output.println(text);
  }
}

void processCommand(const char* cmd, bool isSerial, Stream &output) {
  char buf[256];
  char uptimeBuf[32];

  // Trim whitespace
  while (*cmd == ' ') cmd++;
  if (!*cmd) return;

  if (strcmp(cmd, "ls") == 0 || strcmp(cmd, "dir") == 0) {
    if (!spiffsReady) {
      printOutput("Filesystem not ready. Run 'mount' first.", isSerial, output);
      return;
    }
    Dir dir = SPIFFS.openDir("/");
    bool found = false;
    while (dir.next()) {
      found = true;
      snprintf(buf, sizeof(buf), "%-20s %6d bytes", dir.fileName().c_str(), dir.fileSize());
      printOutput(buf, isSerial, output);
    }
    if (!found) {
      printOutput("Directory is empty", isSerial, output);
    }
  } else if (strncmp(cmd, "touch ", 6) == 0) {
    if (!spiffsReady) {
      printOutput("Filesystem not ready. Run 'mount' first.", isSerial, output);
      return;
    }
    const char* filename = cmd + 6;
    while (*filename == ' ') filename++; // trim spaces
    if (*filename) {
      String fullPath = String("/") + filename;
      File f = SPIFFS.open(fullPath, "w");
      if (f) {
        f.println("# Created by CUPIT Demon OS");
        f.close();
        snprintf(buf, sizeof(buf), "Created file: %s", filename);
        printOutput(buf, isSerial, output);
      } else {
        printOutput("Error: Could not create file", isSerial, output);
      }
    } else {
      printOutput("Usage: touch <filename>", isSerial, output);
    }
  } else if (strncmp(cmd, "rm ", 3) == 0) {
    if (!spiffsReady) {
      printOutput("Filesystem not ready. Run 'mount' first.", isSerial, output);
      return;
    }
    const char* filename = cmd + 3;
    while (*filename == ' ') filename++; // trim spaces
    if (*filename) {
      String fullPath = String("/") + filename;
      if (SPIFFS.exists(fullPath) && SPIFFS.remove(fullPath)) {
        snprintf(buf, sizeof(buf), "Deleted file: %s", filename);
        printOutput(buf, isSerial, output);
      } else {
        snprintf(buf, sizeof(buf), "Error: Could not delete '%s' (file may not exist)", filename);
        printOutput(buf, isSerial, output);
      }
    } else {
      printOutput("Usage: rm <filename>", isSerial, output);
    }
  } else if (strncmp(cmd, "cat ", 4) == 0) {
    if (!spiffsReady) {
      printOutput("Filesystem not ready. Run 'mount' first.", isSerial, output);
      return;
    }
    const char* filename = cmd + 4;
    while (*filename == ' ') filename++; // trim spaces
    if (*filename) {
      String fullPath = String("/") + filename;
      File f = SPIFFS.open(fullPath, "r");
      if (f) {
        while (f.available()) {
          String line = f.readStringUntil('\n');
          printOutput(line.c_str(), isSerial, output);
        }
        f.close();
      } else {
        snprintf(buf, sizeof(buf), "Error: File '%s' not found", filename);
        printOutput(buf, isSerial, output);
      }
    } else {
      printOutput("Usage: cat <filename>", isSerial, output);
    }
  } else if (strncmp(cmd, "echo ", 5) == 0) {
    const char* text = cmd + 5;
    while (*text == ' ') text++; // trim spaces
    
    // Check if it's a redirect
    char* redirect = strstr((char*)text, " > ");
    if (redirect && spiffsReady) {
      *redirect = 0; // Split the string
      const char* content = text;
      const char* filename = redirect + 3;
      while (*filename == ' ') filename++; // trim spaces
      
      if (*filename) {
        String fullPath = String("/") + filename;
        File f = SPIFFS.open(fullPath, "w");
        if (f) {
          f.println(content);
          f.close();
          snprintf(buf, sizeof(buf), "Content written to: %s", filename);
          printOutput(buf, isSerial, output);
        } else {
          printOutput("Error: Could not write to file", isSerial, output);
        }
      }
    } else {
      printOutput(text, isSerial, output);
    }
  } else if (strcmp(cmd, "pwd") == 0) {
    printOutput("/root", isSerial, output);
  } else if (strcmp(cmd, "whoami") == 0) {
    printOutput("root", isSerial, output);
  } else if (strcmp(cmd, "mount") == 0) {
    if (SPIFFS.begin()) {
      spiffsReady = true;
      printOutput("Filesystem mounted successfully", isSerial, output);
      FSInfo fs_info;
      SPIFFS.info(fs_info);
      snprintf(buf, sizeof(buf), "Total: %d bytes, Used: %d bytes, Free: %d bytes", 
               fs_info.totalBytes, fs_info.usedBytes, fs_info.totalBytes - fs_info.usedBytes);
      printOutput(buf, isSerial, output);
    } else {
      printOutput("Error: Failed to mount filesystem", isSerial, output);
    }
  } else if (strcmp(cmd, "df") == 0) {
    if (!spiffsReady) {
      printOutput("Filesystem not ready. Run 'mount' first.", isSerial, output);
      return;
    }
    FSInfo fs_info;
    SPIFFS.info(fs_info);
    printOutput("Filesystem     Size   Used  Avail Use%", isSerial, output);
    snprintf(buf, sizeof(buf), "/dev/spiffs   %5dK %5dK %5dK %3d%%", 
             fs_info.totalBytes/1024, fs_info.usedBytes/1024, 
             (fs_info.totalBytes-fs_info.usedBytes)/1024,
             (fs_info.usedBytes*100)/fs_info.totalBytes);
    printOutput(buf, isSerial, output);
  } else if (strcmp(cmd, "neofetch") == 0) {
    printOutput("", isSerial, output);
    printOutput("       ██████╗██╗   ██╗██████╗ ██╗████████╗", isSerial, output);
    printOutput("      ██╔════╝██║   ██║██╔══██╗██║╚══██╔══╝", isSerial, output);
    printOutput("      ██║     ██║   ██║██████╔╝██║   ██║   ", isSerial, output);
    printOutput("      ██║     ██║   ██║██╔═══╝ ██║   ██║   ", isSerial, output);
    printOutput("      ╚██████╗╚██████╔╝██║     ██║   ██║   ", isSerial, output);
    printOutput("       ╚═════╝ ╚═════╝ ╚═╝     ╚═╝   ╚═╝   ", isSerial, output);
    printOutput("", isSerial, output);
    printOutput("  root@CUPIT-demon", isSerial, output);
    printOutput("  ─────────────────", isSerial, output);
    printOutput("  OS: CUPIT Demon OS v1", isSerial, output);
    printOutput("  Host: ESP8266 Development Board", isSerial, output);
    printOutput("  Kernel: CUPIT Core 1.0", isSerial, output);
    formatUptime(millis() - bootTime, uptimeBuf, sizeof(uptimeBuf));
    snprintf(buf, sizeof(buf), "  Uptime: %s", uptimeBuf);
    printOutput(buf, isSerial, output);
    printOutput("  Shell: CUPIT-sh", isSerial, output);
    snprintf(buf, sizeof(buf), "  CPU: ESP8266 @ %dMHz", ESP.getCpuFreqMHz());
    printOutput(buf, isSerial, output);
    snprintf(buf, sizeof(buf), "  Memory: %dKB free", ESP.getFreeHeap() / 1024);
    printOutput(buf, isSerial, output);
  } else if (strcmp(cmd, "uname -a") == 0 || strcmp(cmd, "uname") == 0) {
    printOutput("CUPIT Demon OS v1 ESP8266 CUPIT Core 1.0", isSerial, output);
  } else if (strcmp(cmd, "ps") == 0 || strcmp(cmd, "ps aux") == 0) {
    printOutput("PID  %CPU %MEM    VSZ   RSS TTY      STAT START   TIME COMMAND", isSerial, output);
    printOutput("  1   0.0  0.0      0     0 ?        S    00:00   0:00 CUPIT-kernel", isSerial, output);
    printOutput("  2   0.0  0.0      0     0 ?        S    00:00   0:00 wifi-daemon", isSerial, output);
    printOutput("  3   0.0  0.0      0     0 ?        S    00:00   0:00 http-server", isSerial, output);
  } else if (strcmp(cmd, "top") == 0) {
    formatUptime(millis() - bootTime, uptimeBuf, sizeof(uptimeBuf));
    snprintf(buf, sizeof(buf), "up %s, load average: 0.01", uptimeBuf);
    printOutput(buf, isSerial, output);
    printOutput("Tasks: 3 total, 1 running, 2 sleeping", isSerial, output);
    snprintf(buf, sizeof(buf), "CPU: %dMHz, Mem: %dKB free", ESP.getCpuFreqMHz(), ESP.getFreeHeap() / 1024);
    printOutput(buf, isSerial, output);
    printOutput("", isSerial, output);
    printOutput("PID USER      PR  NI    VIRT    RES    SHR S  %CPU %MEM     TIME+ COMMAND", isSerial, output);
    printOutput("  1 root      20   0       0      0      0 S   0.0  0.0   0:00.01 CUPIT-kernel", isSerial, output);
  } else if (strcmp(cmd, "free") == 0 || strcmp(cmd, "free -h") == 0) {
    int totalKB = 80; // Approximate ESP8266 RAM
    int freeKB = ESP.getFreeHeap() / 1024;
    int usedKB = totalKB - freeKB;
    printOutput("              total        used        free      shared  buff/cache   available", isSerial, output);
    snprintf(buf, sizeof(buf), "Mem:         %6dK    %6dK    %6dK         0K         0K    %6dK", 
             totalKB, usedKB, freeKB, freeKB);
    printOutput(buf, isSerial, output);
  } else if (strcmp(cmd, "date") == 0) {
    formatUptime(millis(), uptimeBuf, sizeof(uptimeBuf));
    snprintf(buf, sizeof(buf), "System uptime: %s", uptimeBuf);
    printOutput(buf, isSerial, output);
  } else if (strcmp(cmd, "wifi") == 0 || strcmp(cmd, "iwconfig") == 0) {
    snprintf(buf, sizeof(buf), "WiFi AP Status: %s", WiFi.getMode() == WIFI_AP ? "Running" : "Failed");
    printOutput(buf, isSerial, output);
    snprintf(buf, sizeof(buf), "SSID: %s", ssid);
    printOutput(buf, isSerial, output);
    snprintf(buf, sizeof(buf), "IP: %s", WiFi.softAPIP().toString().c_str());
    printOutput(buf, isSerial, output);
    snprintf(buf, sizeof(buf), "Connected Clients: %d", WiFi.softAPgetStationNum());
    printOutput(buf, isSerial, output);
  } else if (strcmp(cmd, "reboot") == 0) {
    printOutput("System is rebooting...", isSerial, output);
    delay(1000);
    ESP.restart();
  } else if (strcmp(cmd, "clear") == 0) {
    if (isSerial) {
      Serial.print("\033[2J\033[H"); // ANSI escape codes to clear screen
    }
    printOutput("", isSerial, output);
  } else if (strcmp(cmd, "debug") == 0) {
    debug = !debug;
    printOutput(debug ? "Debug mode enabled" : "Debug mode disabled", isSerial, output);
  } else if (strcmp(cmd, "help") == 0 || strcmp(cmd, "man") == 0) {
    printOutput("🔥 CUPIT Demon OS v1 - Available Commands:", isSerial, output);
    printOutput("", isSerial, output);
    printOutput("File System:", isSerial, output);
    printOutput("  ls, dir          - List files and directories", isSerial, output);
    printOutput("  cat <file>       - Display file contents", isSerial, output);
    printOutput("  touch <file>     - Create new file", isSerial, output);
    printOutput("  rm <file>        - Delete file", isSerial, output);
    printOutput("  echo <text>      - Display text (use > file to redirect)", isSerial, output);
    printOutput("  mount            - Mount filesystem", isSerial, output);
    printOutput("  df               - Show disk usage", isSerial, output);
    printOutput("", isSerial, output);
    printOutput("System Info:", isSerial, output);
    printOutput("  neofetch         - System information", isSerial, output);
    printOutput("  uname            - System version", isSerial, output);
    printOutput("  ps               - Running processes", isSerial, output);
    printOutput("  top              - System monitor", isSerial, output);
    printOutput("  free             - Memory usage", isSerial, output);
    printOutput("  date             - System uptime", isSerial, output);
    printOutput("  whoami           - Current user", isSerial, output);
    printOutput("  pwd              - Current directory", isSerial, output);
    printOutput("", isSerial, output);
    printOutput("Network:", isSerial, output);
    printOutput("  wifi, iwconfig   - WiFi status", isSerial, output);
    printOutput("", isSerial, output);
    printOutput("System Control:", isSerial, output);
    printOutput("  reboot           - Restart system", isSerial, output);
    printOutput("  clear            - Clear screen", isSerial, output);
    printOutput("  debug            - Toggle debug mode", isSerial, output);
  } else {
    snprintf(buf, sizeof(buf), "Command not found: %s", cmd);
    printOutput(buf, isSerial, output);
    printOutput("Type 'help' for available commands", isSerial, output);
  }
}

void ICACHE_FLASH_ATTR CUPITKernel() {
  if (Serial.available()) {
    static uint8_t idx = 0;
    char c = Serial.read();
    if (c == '\n' || c == '\r') {
      if (idx > 0) {
        command[idx] = 0;
        Serial.println(); // New line
        processCommand(command, true, Serial);
        Serial.print("root@CUPIT-demon:~# ");
        idx = 0;
      }
    } else if (c == 127 || c == 8) { // Backspace
      if (idx > 0) {
        idx--;
        Serial.print("\b \b"); // Erase character
      }
    } else if (c >= 32 && c < 127 && idx < sizeof(command) - 1) {
      command[idx++] = c;
      Serial.print(c); // Echo character
    }
  }
}

void ICACHE_FLASH_ATTR handleRoot() {
  String html = R"html(
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0, user-scalable=no">
    <title>🔥 CUPIT Demon OS v1 - Terminal</title>
    <style>
        * {
            margin: 0;
            padding: 0;
            box-sizing: border-box;
        }
        
        body {
            font-family: 'Courier New', monospace;
            background: linear-gradient(45deg, #0a0a0a, #1a0a0a);
            color: #00ff41;
            height: 100vh;
            overflow: hidden;
            -webkit-user-select: none;
            -moz-user-select: none;
            -ms-user-select: none;
            user-select: none;
            -webkit-touch-callout: none;
            -webkit-tap-highlight-color: transparent;
        }
        
        .container {
            display: flex;
            flex-direction: column;
            height: 100vh;
            padding: 10px;
            min-height: 0;
        }
        
        .header {
            text-align: center;
            margin-bottom: 10px;
            border-bottom: 2px solid #00ff41;
            padding-bottom: 8px;
            flex-shrink: 0;
        }
        
        .header h1 {
            color: #ff4444;
            text-shadow: 0 0 10px #ff4444;
            font-size: clamp(1.2rem, 4vw, 2.2rem);
            margin-bottom: 5px;
            line-height: 1.1;
        }
        
        .header p {
            color: #00ff41;
            font-size: clamp(0.7rem, 2.5vw, 1rem);
            padding: 0 5px;
            line-height: 1.2;
        }
        
        .terminal {
            flex: 1;
            background: rgba(0, 0, 0, 0.8);
            border: 2px solid #00ff41;
            border-radius: 10px;
            padding: 8px;
            display: flex;
            flex-direction: column;
            box-shadow: 0 0 30px rgba(0, 255, 65, 0.3);
            min-height: 0;
            overflow: hidden;
        }
        
        .output {
            flex: 1;
            overflow-y: auto;
            margin-bottom: 8px;
            padding: 6px;
            background: rgba(0, 20, 0, 0.5);
            border-radius: 5px;
            border: 1px solid #003300;
            white-space: pre-wrap;
            font-size: clamp(9px, 2.5vw, 13px);
            line-height: 1.3;
            word-wrap: break-word;
            -webkit-overflow-scrolling: touch;
            min-height: 0;
        }
        
        .input-area {
            display: flex;
            align-items: center;
            gap: 5px;
            flex-shrink: 0;
        }
        
        .prompt {
            color: #00ff41;
            font-weight: bold;
            white-space: nowrap;
            font-size: clamp(9px, 2.5vw, 13px);
            flex-shrink: 0;
        }
        
        .cmd-input {
            flex: 1;
            background: rgba(0, 50, 0, 0.3);
            color: #00ff41;
            border: 2px solid #004400;
            border-radius: 5px;
            padding: 10px 8px;
            font-family: 'Courier New', monospace;
            font-size: clamp(11px, 3vw, 15px);
            outline: none;
            transition: all 0.3s ease;
            min-width: 0;
            -webkit-user-select: text;
            -moz-user-select: text;
            -ms-user-select: text;
            user-select: text;
        }
        
        .cmd-input:focus {
            border-color: #00ff41;
            box-shadow: 0 0 10px rgba(0, 255, 65, 0.3);
        }
        
        .execute-btn {
            background: linear-gradient(45deg, #00ff41, #00aa2a);
            color: #000;
            border: none;
            border-radius: 5px;
            padding: 10px 12px;
            font-family: 'Courier New', monospace;
            font-weight: bold;
            cursor: pointer;
            transition: all 0.3s ease;
            text-transform: uppercase;
            font-size: clamp(9px, 2.5vw, 13px);
            touch-action: manipulation;
            -webkit-tap-highlight-color: transparent;
            min-width: 50px;
            flex-shrink: 0;
        }
        
        .execute-btn:hover, .execute-btn:active {
            transform: translateY(-1px);
            box-shadow: 0 3px 10px rgba(0, 255, 65, 0.4);
        }
        
        .quick-commands {
            display: grid;
            grid-template-columns: repeat(auto-fit, minmax(60px, 1fr));
            gap: 4px;
            margin-bottom: 8px;
            flex-shrink: 0;
        }
        
        .quick-cmd {
            background: rgba(255, 68, 68, 0.2);
            color: #ff4444;
            border: 1px solid #ff4444;
            border-radius: 3px;
            padding: 6px 4px;
            font-family: 'Courier New', monospace;
            font-size: clamp(8px, 2vw, 11px);
            cursor: pointer;
            transition: all 0.3s ease;
            touch-action: manipulation;
            -webkit-tap-highlight-color: transparent;
            text-align: center;
        }
        
        .quick-cmd:hover, .quick-cmd:active {
            background: rgba(255, 68, 68, 0.4);
            transform: scale(1.02);
        }
        
        .scrollbar {
            scrollbar-width: thin;
            scrollbar-color: #00ff41 #001100;
        }
        
        .scrollbar::-webkit-scrollbar {
            width: 6px;
        }
        
        .scrollbar::-webkit-scrollbar-track {
            background: #001100;
        }
        
        .scrollbar::-webkit-scrollbar-thumb {
            background: #00ff41;
            border-radius: 3px;
        }
        
        .status-bar {
            display: flex;
            justify-content: space-between;
            align-items: center;
            padding: 6px;
            background: rgba(0, 255, 65, 0.1);
            border-radius: 5px;
            margin-bottom: 8px;
            font-size: clamp(8px, 2vw, 11px);
            flex-wrap: wrap;
            gap: 5px;
            flex-shrink: 0;
        }
        
        .status-item {
            white-space: nowrap;
            overflow: hidden;
            text-overflow: ellipsis;
            min-width: 0;
        }
        
        .loading {
            color: #ffaa00;
        }
        
        /* Mobile-first responsive design */
        @media (max-width: 600px) {
            .container {
                padding: 5px;
            }
            
            .header {
                margin-bottom: 8px;
                padding-bottom: 6px;
            }
            
            .terminal {
                padding: 6px;
            }
            
            .output {
                padding: 5px;
                margin-bottom: 6px;
            }
            
            .input-area {
                gap: 3px;
            }
            
            .cmd-input {
                padding: 8px 6px;
            }
            
            .execute-btn {
                padding: 8px 10px;
                min-width: 45px;
            }
            
            .quick-commands {
                grid-template-columns: repeat(4, 1fr);
                gap: 3px;
                margin-bottom: 6px;
            }
            
            .quick-cmd {
                padding: 5px 2px;
            }
            
            .status-bar {
                padding: 5px;
                margin-bottom: 6px;
            }
            
            .prompt {
                display: none;
            }
        }
        
        @media (max-width: 400px) {
            .status-bar {
                flex-direction: column;
                text-align: center;
            }
            
            .status-item {
                width: 100%;
            }
            
            .quick-commands {
                grid-template-columns: repeat(3, 1fr);
            }
        }
        
        @media (max-height: 500px) {
            .header h1 {
                font-size: 1.2rem;
            }
            
            .header p {
                font-size: 0.7rem;
            }
            
            .container {
                padding: 3px;
            }
            
            .terminal {
                padding: 5px;
            }
        }
    </style>
</head>
<body>
    <div class="container">
        <div class="header">
            <h1>🔥 CUPIT DEMON OS v1 🔥</h1>
            <p>ESP8266 Terminal Interface - Enter commands to interact with the system</p>
        </div>
        
        <div class="status-bar">
            <span class="status-item">Status: <span id="status">Ready</span></span>
            <span class="status-item">User: root@CUPIT-demon</span>
            <span class="status-item">Path: /root</span>
        </div>
        
        <div class="quick-commands">
            <button class="quick-cmd" onclick="executeQuick('help')">help</button>
            <button class="quick-cmd" onclick="executeQuick('neofetch')">neofetch</button>
            <button class="quick-cmd" onclick="executeQuick('mount')">mount</button>
            <button class="quick-cmd" onclick="executeQuick('ls')">ls</button>
            <button class="quick-cmd" onclick="executeQuick('wifi')">wifi</button>
            <button class="quick-cmd" onclick="executeQuick('ps')">ps</button>
            <button class="quick-cmd" onclick="executeQuick('free')">free</button>
            <button class="quick-cmd" onclick="executeQuick('clear')">clear</button>
        </div>
        
        <div class="terminal">
            <div class="output scrollbar" id="output">Welcome to CUPIT Demon OS v1 Web Terminal!
Type 'help' to see available commands or use the quick buttons above.

root@CUPIT-demon:~# </div>
            
            <div class="input-area">
                <span class="prompt">root@CUPIT-demon:~#</span>
                <input type="text" id="cmdInput" class="cmd-input" placeholder="Enter command..." autocomplete="off" autocorrect="off" autocapitalize="off" spellcheck="false">
                <button class="execute-btn" onclick="executeCommand()">▶</button>
            </div>
        </div>
    </div>

    <script>
        const output = document.getElementById('output');
        const cmdInput = document.getElementById('cmdInput');
        const status = document.getElementById('status');
        let commandHistory = [];
        let historyIndex = -1;
        
        // Focus input on page load
        window.addEventListener('load', () => {
            cmdInput.focus();
        });
        
        // Prevent zoom on double tap
        let lastTouchEnd = 0;
        document.addEventListener('touchend', function (event) {
            const now = (new Date()).getTime();
            if (now - lastTouchEnd <= 300) {
                event.preventDefault();
            }
            lastTouchEnd = now;
        }, false);
        
        function scrollToBottom() {
            output.scrollTop = output.scrollHeight;
        }
        
        function appendOutput(text) {
            output.textContent += text + '\n';
            scrollToBottom();
        }
        
        function executeQuick(cmd) {
            cmdInput.value = cmd;
            executeCommand();
        }
        
        function executeCommand() {
            const cmd = cmdInput.value.trim();
            if (!cmd) return;
            
            // Add to history
            if (commandHistory[commandHistory.length - 1] !== cmd) {
                commandHistory.push(cmd);
                if (commandHistory.length > 50) {
                    commandHistory.shift();}
            }
            historyIndex = -1;
            
            // Show command in output
            appendOutput('root@CUPIT-demon:~# ' + cmd);
            
            // Clear input
            cmdInput.value = '';
            
            // Handle special commands
            if (cmd === 'clear') {
                output.textContent = 'root@CUPIT-demon:~# ';
                return;
            }
            
            // Set status to loading
            status.textContent = 'Processing...';
            status.className = 'loading';
            
            // Send command to ESP8266
            fetch('/execute', {
                method: 'POST',
                headers: {
                    'Content-Type': 'application/x-www-form-urlencoded',
                },
                body: 'cmd=' + encodeURIComponent(cmd)
            })
            .then(response => response.text())
            .then(data => {
                if (data.trim()) {
                    appendOutput(data);
                }
                appendOutput('root@CUPIT-demon:~# ');
                status.textContent = 'Ready';
                status.className = '';
            })
            .catch(error => {
                appendOutput('Error: Network request failed');
                appendOutput('root@CUPIT-demon:~# ');
                status.textContent = 'Error';
                status.className = '';
                console.error('Error:', error);
            });
        }
        
        // Handle Enter key
        cmdInput.addEventListener('keydown', function(e) {
            if (e.key === 'Enter') {
                e.preventDefault();
                executeCommand();
            } else if (e.key === 'ArrowUp') {
                e.preventDefault();
                if (commandHistory.length > 0) {
                    if (historyIndex === -1) {
                        historyIndex = commandHistory.length - 1;
                    } else if (historyIndex > 0) {
                        historyIndex--;
                    }
                    cmdInput.value = commandHistory[historyIndex];
                }
            } else if (e.key === 'ArrowDown') {
                e.preventDefault();
                if (historyIndex !== -1) {
                    historyIndex++;
                    if (historyIndex >= commandHistory.length) {
                        historyIndex = -1;
                        cmdInput.value = '';
                    } else {
                        cmdInput.value = commandHistory[historyIndex];
                    }
                }
            }
        });
        
        // Keep input focused (mobile friendly)
        document.addEventListener('click', function(e) {
            if (e.target !== cmdInput && !e.target.classList.contains('quick-cmd') && !e.target.classList.contains('execute-btn')) {
                cmdInput.focus();
            }
        });
        
        // Handle mobile keyboard
        cmdInput.addEventListener('blur', function() {
            setTimeout(() => {
                if (document.activeElement !== cmdInput) {
                    cmdInput.focus();
                }
            }, 100);
        });
        
        // Auto-scroll output on new content
        const observer = new MutationObserver(scrollToBottom);
        observer.observe(output, { childList: true, subtree: true });
    </script>
</body>
</html>
)html";
  
  server.send(200, "text/html", html);
}

void ICACHE_FLASH_ATTR handleExecute() {
  if (server.hasArg("cmd")) {
    String cmd = server.arg("cmd");
    cmd.trim();
    
    if (cmd.length() > 0) {
      String response = "";
      
      // Capture output in a string
      class StringStream : public Stream {
        public:
          String buffer;
          size_t write(uint8_t c) override {
            buffer += (char)c;
            return 1;
          }
          size_t write(const uint8_t *data, size_t len) override {
            for (size_t i = 0; i < len; i++) {
              buffer += (char)data[i];
            }
            return len;
          }
          int available() override { return 0; }
          int read() override { return -1; }
          int peek() override { return -1; }
          void flush() override {}
      } stringStream;
      
      processCommand(cmd.c_str(), false, stringStream);
      server.send(200, "text/plain", stringStream.buffer);
    } else {
      server.send(400, "text/plain", "No command provided");
    }
  } else {
    server.send(400, "text/plain", "Missing cmd parameter");
  }
}

void setup() {
  Serial.begin(115200);
  bootTime = millis();
  
  Serial.println();
  Serial.println("🔥 CUPIT Demon OS v1 - Booting...");
  Serial.println("====================================");
  
  // Set up WiFi Access Point
  WiFi.mode(WIFI_AP);
  WiFi.softAPConfig(apIP, apIP, IPAddress(255, 255, 255, 0));
  WiFi.softAP(ssid, password);
  
  Serial.println("WiFi Access Point started");
  Serial.print("SSID: ");
  Serial.println(ssid);
  Serial.print("IP Address: ");
  Serial.println(apIP);
  Serial.println();
  
  // Set up web server
  server.on("/", handleRoot);
  server.on("/execute", HTTP_POST, handleExecute);
  server.begin();
  
  Serial.println("Web server started on port 80");
  Serial.println("====================================");
  Serial.println("🔥 CUPIT Demon OS v1 - Ready! 🔥");
  Serial.println("Type 'help' for available commands");
  Serial.print("root@CUPIT-demon:~# ");
}

void loop() {
  server.handleClient();
  CUPITKernel();
  yield(); // Allow ESP8266 to handle WiFi tasks
}