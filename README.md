# Editor Light DataBase
Universal database management client written in C++ with a Qt graphical user interface.

## Technologies

    C++23 | Qt 6.12 | QML | SQL

## Current Features
	Connection Window:
	- Connection to SQLite DB via local or network path.
	- DBMS type selection (PostgreSQL, MySQL, SQLite, Access, Oracle).
    - Input fields for address, port, login, password, or DB path.
    - Connection testing before connecting.
	- Configuration saving.
	
	Main Window:
	- Pagination for loading large tables.
	- View the list of database tables.
	- Case-insensitive search across all values.
	
	Database Operations:
	- Data merging from multiple tables with filtering by creating a new physical table.
	- Data editing inside tables.
    - Insertion, modification, and row-by-row deletion of data.
	- Creation of new tables.
    - Renaming and deletion of tables.
	- Input dialog for new row values.
	
	Additional Features:
	- Local notes (stored independently from the DB).
	- SQL injection protection.
	- Added database driver support(SQLite, MySQL, MariaDB, PostgreSQL, Oracle, Access/ODBC, InterBase, Mimer).
	- Port UI from C++ to QML.

## Planned Features

	- User administration capabilities (creation, privilege assignment, blocking/deletion).
	- Data merging from multiple tables with filtering by creating a new virtual table.
	- Backup and recovery.
	- Data export/import (CSV, JSON, SQL INSERT script).
	- SQL console.
	- Transactions for batch operations.
	- Index management (listing, creation, deletion).
	- Stored routines & triggers management (dedicated windows with built-in validation and a quick-access toolbar).

## Downloads
Prebuilt Windows x64 installer: see [Releases](https://github.com/VSRom/Editor-Light-DataBase/releases)
