# Employee Database

A lightweight command-line (CLI) application for managing an employee database.

## Features
* Create new database files.
* Add new employees to the database.
* List all employees in the database.

## Getting Started

### Prerequisites

* `gcc`
* `make`

### Building

To build the application, run the following command:

```bash
make
```

This will create the `dbview` executable in the `bin/` directory.

## Usage

### Create a new database

```bash
./bin/dbview -f <database file> -n
```

### Add a new employee

```bash
./bin/dbview -f <database file> -a "name,address,hours"
```

### List employees

```bash
./bin/dbview -f <database file> -l
```

## Database File Format

The application uses a custom binary format for storing data.

### Header

| Field      | Size (bytes) | Description                                  |
|------------|--------------|----------------------------------------------|
| Magic      | 4            | A magic number to identify the file type.    |
| Version    | 2            | The version of the database format.          |
| Count      | 2            | The number of employee records in the file.  |
| Filesize   | 4            | The total size of the database file in bytes.|

### Employee Record

| Field      | Size (bytes) | Description                   |
|------------|--------------|-------------------------------|
| Name       | 256          | The employee's name.          |
| Address    | 256          | The employee's address.       |
| Hours      | 4            | The employee's hours worked.  |

## Acknowledgments

This project was developed as part of my learning path at lowlevel.academy (https://lowlevel.academy).
Generative AI was used as a tool to assist with debugging and to generate portions of this README file. All code and documentation were ultimately written, reviewed, and
validated by me. No code was written by AI.

## License

This project is licensed under the GPL-3.0 license, see LICENSE for details.
