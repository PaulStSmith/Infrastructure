         1         2         3         4         5         6         7
....|....0....|....0....|....0....|....0....|....0....|....0....|....0....|

+=========================================================================+
| Copyright © 2005-2007 Paulo Santos                                      |
|                                                                         |
| Permission is hereby granted, free of charge, to any person obtaining a |
| copy  of  this  software  and  associated  documentation   files   (the |
| "Software"), to deal in the  Software  without  restriction,  including |
| without limitation the rights to use,  copy,  modify,  merge,  publish, |
| distribute, sublicense, and/or sell copies  of  the  Software,  and  to |
| permit persons to whom the Software is furnished to do so,  subject  to |
| the following conditions:                                               |
|                                                                         |
| The above copyright notice and this permission notice shall be included |
| in all copies or substantial portions of the Software.                  |
+=========================================================================+
| D I S C L A I M E R                                                     |
|                                                                         |
| The Software is provided "AS IS", without warranty of any kind, express |
| or  implied,  including  but  not  limited   to   the   warranties   of |
| merchantability, fitness for a particular purpose and non-infringement. |
| In no event shall the authors or copyright holders be  liable  for  any |
| claim, damages or other liability, whether in an  action  of  contract, |
| tort or otherwise, arising from, out  of  or  in  connection  with  the |
| software or the use or other dealings in the software.                  |
+=========================================================================+

This infrastructure setup helper is an  evolution  from  the  original  one
posted at "The Code Project" web site by Kevin Moore.
(http://www.codeproject.com/managedcpp/dotnetsetup.asp)

It is an instalation program that verifies if the target computer  has  all
the apropriate infrastructure to run a .NET application. I've changed a few
things, including a better version checking about MDAC and IE, as well I've
added  support  to  select  wich  components  to  install  (once  not   all
aplications need MDAC and MSDE for instance).

This program refers to a "settings.ini"  file  where  all  the  information
regarding what install and where to find are stored.

There are two sections in the INI file:

    [Bootstrap]
    [WhatInstall]

[Bootstrap]
Gathers the information about the location  of  the  setup  files  for  the
infrastrucutre components.

It has the following keys:

+---------------------+------------+--------------------------------------+
| Key                 | Status     | Description                          |
+---------------------+------------+--------------------------------------+
| CaptionText         | optional   | The title of all the message         |
|                     |            | box that will be displayed.          |
|                     |            | defaults to                          |
|                     |            | "<ProductName> Setup"                |
+---------------------+------------+--------------------------------------+
| ErrorCaptionText    | optional   | The title of the ERROR message       |
|                     |            | boxes.                               |
|                     |            | defaults to                          |
|                     |            | "<ProductName> Setup Error"          |
+---------------------+------------+--------------------------------------+
| FxInstallerName     | optional   | Name of the redistributable .NET     |
|                     |            | installer.                           |
|                     |            | defaults to                          |
|                     |            | "dotnetfx.exe"                       |
+---------------------+------------+--------------------------------------+
| FxInstallerPath     | optional   | Relative path to "DOTNETFX.EXE"      |
|                     |            | defaults to                          |
|                     |            | the SETUP.EXE path                   |
+---------------------+------------+--------------------------------------+
| FxInstallerParams   | optional   | Command line parameter passed to     |
|                     |            | the .NET Framework installer.        |
|                     |            | defaults to                          |
|                     |            | /q:a /c:"install /l /q"              |
+---------------------+------------+--------------------------------------+
| FxVersion           | mandatory* | Version of the .NET Framework        |
|                     |            | required by the application.         |
|                     |            | Requires the FULL version like       |
|                     |            |     v1.0.3705                        |
|                     |            |     v1.1.4322                        |
|                     |            |     v2.0.50727                       |
|                     |            |     v3.0                             |
+---------------------+------------+--------------------------------------+
| IEInstallerName     | optional   | The name of the IE installer         |
|                     |            | defaults to                          |
|                     |            | "IE6SETUP.EXE"                       |
+---------------------+------------+--------------------------------------+
| IEInstallerPath     | optional   | Relative path to IE setup            |
|                     |            | defaults to                          |
|                     |            | the SETUP.EXE path                   |
+---------------------+------------+--------------------------------------+
| IEInstallerParams   | optional   | Command line parameters to pass to   |
|                     |            | the IE setup.                        |
|                     |            | defaults to                          |
|                     |            | /passive                             |
+---------------------+------------+--------------------------------------+
| IEVersion           | mandatory* | The minimum version od IE required   |
|                     |            | defaults to                          |
|                     |            | 6.0                                  |
+---------------------+------------+--------------------------------------+
| MDACInstallerPath   | optional   | Relative path to MDAC_TYP.EXE        |
|                     |            | defaults to                          |
|                     |            | the SETUP.EXE path                   |
+---------------------+------------+--------------------------------------+
| MDACInstallerParams | optional   | Command line parameters to pass to   |
|                     |            | the MDAC installer.                  |
|                     |            | defaults to                          |
|                     |            | /q                                   |
+---------------------+------------+--------------------------------------+
| MDACVersion         | mandatory* | Version of the MDAC to be checked in |
|                     |            | the format <MAJOR>.<MINOR>           |
|                     |            | defaults to                          |
|                     |            | 2.7                                  |
+---------------------+------------+--------------------------------------+
| MSDEInstallerPath   | optional   | Relative path of the SETUP.EXE file  |
|                     |            | that will install the MSDE           |
|                     |            | defaults to                          |
|                     |            | the SETUP.EXE path                   |
+---------------------+------------+--------------------------------------+
| MSDEInstallerParams | optional   | Parameters to be passed to the MSDE  |
| (**)                |            | setup (see MSDE documentation for    |
|                     |            | greater details)                     |
|                     |            | defaults to                          |
|                     |            | /setup.ini /SAPWD='password'         |
+---------------------+------------+--------------------------------------+
| MSIInstallerName    | optional   | Name of the MSI redistributable      |
|                     |            | installer.                           |
|                     |            | defaults to                          |
|                     |            |  * INSTMSIA.EXE for Win9x            |
|                     |            |  * INSTMSIW.EXE for Win2000          |
|                     |            |  * INSTMSI.EXE  for WinXP and UP     |
+---------------------+------------+--------------------------------------+
| MSIInstallerPath    | optional   | Relative path to the                 |
|                     |            | Redistributable Windows Installer    |
|                     |            | defaults to                          |
|                     |            | the SETUP.EXE path                   |
+---------------------+------------+--------------------------------------+
| MSIInstallerParams  | optional   | Commandline parameters to pass to    |
|                     |            | the Windows Installer Setup.         |
|                     |            | defaults to                          |
|                     |            | /Q                                   |
+---------------------+------------+--------------------------------------+
| Package             | mandatory  | Name of the MSI  package  that  will |
|                     |            | be  installed   after   the   infra- |
|                     |            | structure                            |
+---------------------+------------+--------------------------------------+
| ProductName         | optional   | The name of your product             |
|                     |            | defaults to                          |
|                     |            | "Application"                        |
+---------------------+------------+--------------------------------------+

(*)  The keys  marked  mandatory*  are  mandatory  only  if  the  specified
     component is required by the application.
     See [WhatInstall] section below.

(**) In the MSDEParams key you may specify the SA password in  the  command
     line, however, it will be stored as plain text in the  "settings.ini".
     This  might  raise  some  security  issues  within  your  application.
     Be aware of this.

[WhatInstall]
Configures what is necessary to the current installation.
It has the following keys:

+------------------+--------+---------------------------------------------+
| Key              | Values | Description                                 |
+------------------+--------+---------------------------------------------+
| .NETFramework    | 1 or 0 | .NET Framework                              |
+------------------+        +---------------------------------------------+
| IE               |        | Internet Explorer                           |
+------------------+        +---------------------------------------------+
| MDAC             |        | Microsoft Data Access Components            |
+------------------+        +---------------------------------------------+
| MSDE             |        | Microsoft SQL Server Desktop Engine         |
+------------------+        +---------------------------------------------+
| WindowsInstaller |        | Windows Installer                           |
+------------------+--------+---------------------------------------------+
