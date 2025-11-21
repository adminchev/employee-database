# Employee Database

A concurrent TCP server for managing an employee database with a custom binary protocol. Implements stateful packet buffering and finite state machine for robust network communication.

## Features

- **Concurrent multi-client support** - Handle up to 10 simultaneous connections using `poll()`
- **Stateful packet buffering** - Handles TCP packet fragmentation with automatic reassembly
- **Protocol handshake** - Version negotiation with client validation
- **Timeout detection** - 5-second timeout for stalled partial receives
- **Custom binary format** - Network byte order for cross-platform compatibility
- **CRUD operations** - Create, list, add, and delete employee records
- **Interactive client mode** - Shell-like interface for database operations

## Architecture

### Server (dbview)
- Finite state machine with 3 states: `S_NEW` → `S_CONNECTED` → `S_DISCONNECTED`
- Partial packet accumulation in per-client buffers
- Automatic cleanup of timed-out connections
- Binary database file with header validation

### Client
- TCP connection with protocol handshake (MSG_HELLO)
- Interactive mode for multiple operations
- Single-command mode for scripting
- Network byte order conversion for all requests

## Getting Started

### Prerequisites

- `gcc` compiler
- `make`
- Linux/Unix environment (uses POSIX sockets)

### Building

```bash
make clean && make
```

This creates two executables:
- `bin/dbview` - The database server
- `bin/client` - The client application

## Usage

### 1. Start the Server

Create a new database and start the server:
```bash
./bin/dbview -f employees.db -n
```

Open an existing database:
```bash
./bin/dbview -f employees.db
```

The server listens on port `5555` by default.

### 2. Connect with Client

**Interactive mode** (recommended):
```bash
./bin/client -c 127.0.0.1 -i
```

This opens an interactive shell:
```
> list
Number of employees: 2
	name: John Doe, address: 123 Main St, hours: 40
	name: Jane Smith, address: 456 Oak Ave, hours: 35
> add Alice Johnson,789 Elm St,40
Added Alice Johnson, address: 789 Elm St
> del Alice Johnson
Deleting employee - Alice Johnson
> list
...
```

**Single command mode** (for scripting):
```bash
./bin/client -c 127.0.0.1 -l  # List employees
```

### Available Commands

| Command | Format | Example |
|---------|--------|---------|
| `list` | `list` | Lists all employees |
| `add` | `add name,address,hours` | `add John Doe,123 Main St,40` |
| `del` | `del name` | `del John Doe` |

## Database File Format

All multi-byte integers stored in network byte order (big-endian).

### Header Structure (12 bytes)
| Field      | Type     | Size | Description                          |
|------------|----------|------|--------------------------------------|
| Magic      | uint32_t | 4    | Magic number for file identification |
| Version    | uint16_t | 2    | Database format version              |
| Count      | uint16_t | 2    | Number of employee records           |
| Filesize   | uint32_t | 4    | Total file size in bytes             |

### Employee Record (516 bytes)
| Field      | Type     | Size | Description              |
|------------|----------|------|--------------------------|
| Name       | char[]   | 256  | Employee name            |
| Address    | char[]   | 256  | Employee address         |
| Hours      | uint32_t | 4    | Hours worked (network order) |

## Network Protocol

### Message Format (520 bytes)
```c
typedef struct {
    cmd_type_e cmd;      // Command type (4 bytes, network order)
    uint32_t len;        // Data length (4 bytes, network order)
    char data[512];      // Command payload
} request_t;
```

### Protocol Flow
1. **Client → Server**: `MSG_HELLO` with protocol version
2. **Server → Client**: `MSG_HELLO` response (handshake accepted)
3. **Client → Server**: Command requests (LIST/ADD/DELETE)
4. **Server → Client**: Response data

Connection rejected if:
- First message is not `MSG_HELLO`
- Protocol version mismatch
- Partial packet times out after 5 seconds

## Technical Highlights

### Partial Packet Handling
The server accumulates bytes using `bytes_received` tracking:
```c
recv(fd, buffer + bytes_received, sizeof(request_t) - bytes_received, 0)
```
Defers byte order conversion until full packet received to avoid operating on incomplete data.

### Finite State Machine
- **S_NEW**: New connection - expects MSG_HELLO handshake
- **S_CONNECTED**: Authenticated client - processes commands
- **S_DISCONNECTED**: Connection closed or failed validation

### Concurrency Model
Uses `poll()` for I/O multiplexing with per-client state tracking. Supports up to 10 concurrent clients (configurable via `MAX_CLIENTS`).

## Project Structure

```
.
├── include/          # Header files
│   ├── common.h      # Shared types and constants
│   ├── server.h      # Server interface
│   ├── client.h      # Client interface
│   ├── parse.h       # Database operations
│   └── file.h        # File I/O
├── src/              # Source files
│   ├── server.c      # TCP server with FSM
│   ├── client.c      # TCP client
│   ├── main.c        # Server entry point
│   ├── parse.c       # CRUD operations
│   └── file.c        # Database file handling
├── bin/              # Compiled executables
├── obj/              # Object files
└── Makefile          # Build configuration
```

## Known Limitations

- Maximum 10 concurrent connections
- Employee name/address limited to 256 characters
- No authentication/encryption (TCP plaintext)
- Single-threaded server (uses poll, not threads)
- No Windows support (POSIX sockets only)

## Development

### Compilation Flags
```bash
gcc -Wall -Wextra -Wunused -g -Iinclude
```

### Testing
Start server in one terminal:
```bash
./bin/dbview -f test.db -n
```

Run multiple clients in separate terminals:
```bash
./bin/client -c 127.0.0.1 -i
```

Test partial packet handling by artificially slowing network or using tools like `tc` (traffic control).

## Acknowledgments

This project was developed as part of my learning path at [lowlevel.academy](https://lowlevel.academy).

Generative AI was used to assist with debugging, documentation generation, and code review. All implementation decisions, architecture choices, and final code were written and validated by me.

## License

This project is licensed under the GPL-3.0 license - see LICENSE for details.
