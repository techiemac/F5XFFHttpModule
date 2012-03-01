-----------------------------------------------------------------------------
Files:
-----------------------------------------------------------------------------

readme.txt      - this document
Install-XFF.ps1 - Install script to register the module with IIS.
x86\
  Debug\        - 32 bit Debug build
  Release\      - 32 bit Release build
x64\
  Debug\        - 64 bit Debug build
  Release\      - 64 bit Release build
  
-----------------------------------------------------------------------------
Installation
-----------------------------------------------------------------------------

1) Create a directory for the HTTP Module

ie. C:\> md c:\HttpModules

2) Copy the files from the Release build of your target platform (x86 for 32-bit or x64 for 64-bit).

ie. C:\> xcopy <zip_dir>\x64\Release\* c:\HttpModules

3) Copy the Install-XFF-ps1 PowerShell script to the target directory

ie. C:\> xcopy <zip_dir>\Install-XFF.ps1 c:\HttpModules

4) Change directory to the install path

C:\> cd c:\HttpModules

5) Register the Module with the install script (or via IIS admin).

C:\HttpModules\> .\Install-XFF.ps1 -cmd install -module_path c:\HttpModules\F5XFFHttpModule.dll

The module should now be installed in your top level IIS server Module settings as well as each existing application.  You can selectively add/remove them from the IIS admin at this point.

-----------------------------------------------------------------------------
Customizing The X-Forwarded-For Header Name
-----------------------------------------------------------------------------

The F5 XFF Http Module supports a configuration file named F5XFFHttpModule.ini.  It looks in the same directory as the configured .DLL for a file of the same name as it self with the .INI extension.  An example file is included but renamed to F5XFFHttpModule.ini.bak.  Once you get rid of the ".bak" extension and restart the application the settings will take effect.  In this configuration file, you can override the default header name of "X-Forwarded-For" to whatever you choose.  At this point, the Module only supports a single header.

-----------------------------------------------------------------------------
Troubleshooting
-----------------------------------------------------------------------------

The debug build will log a bunch of info to the c:\F5XFFModule.log file.  You may have to remove the module from your IIS applications to overwrite the module in the C:\HttpModules directory.  Sometimes an "iisreset" will do the trick.  The debug build produces A LOT of data so DO NOT run the debug build in production.

