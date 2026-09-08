# Editor Light DataBase
Universal database management client written in C++ with a Qt graphical user interface.

## Technologies

    C++
    Qt 6 (Widgets, Sql)
    CMake (build system)
    Git (version control)

## Current Features
	Connection Window:
	- Connection to SQLite DB via local or network path.
	- DBMS type selection (PostgreSQL, MySQL, SQLite, Access, Oracle).
    - Input fields for address, port, login, password, or DB path.
    - Connection testing before connecting.
	- Configuration saving.
	
	Main Window:
	- View the list of database tables.
	- Case-insensitive search across all values.
	
	Database Operations:
	- Data merging from multiple tables with filtering by creating a new physical table.
	- Data editing inside tables.
    - Insertion, modification, and row-by-row deletion of data.
	- Creation of new tables.
    - Renaming of tables.
    - Deletion of tables.
	
	Additional Features:
	- Local notes (stored independently from the DB).

	
## Planned Features

	- User administration capabilities (creation, privilege assignment, blocking/deletion).
	- Data merging from multiple tables with filtering by creating a new virtual table.
	- SQL injection protection.
	- Input dialog for new row values.
	- Pagination for loading large tables.
	- Backup and recovery.
	- Data export/import (CSV, JSON, SQL INSERT script).
	- SQL console.
	- Transactions for batch operations.
	- Index management (listing, creation, deletion).

Build:
	
	x64 Native Tools Command Prompt for VS
	set QTDIR=C:\QT\*.*.*\msvc2019_64
	echo %QTDIR%
	git clone https://github.com
	cmake --preset=Qt-Release
	cmake --build out/build/release
	cd out/build/release
	"%QTDIR%\bin\windeployqt.exe" Editor-Light-DataBase.exe	// ELDB.exe
	
