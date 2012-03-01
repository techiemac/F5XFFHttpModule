Mac's changes to F5 X-Forwarded-For HTTP Module
===============================================

This is the source of the excellent F5 X-Forwarded-For IIS Module. This module takes the X-Forwarded-For HTTP header and converts it so it's visible as the c-ip in the IIS logs. Credit is due to Joe Pruitt over a F5 and the original source and discussion can be found [here](https://devcentral.f5.com/weblogs/joe/archive/2009/12/23/x-forwarded-for-http-module-for-iis7-source-included.aspx)

Installation
------------
A full readme.txt describing the installation is located in the F5XFFHttpModule-Source directory.

Occasionally a site may be running in Model-T mode (32 bit) and the installation may require modification. If this is the case, use the 32 bit DLL.

In the case of mixed 32 and 64 bit sites, you may have to run appcmd.exe manually (or simply add the module manually). The 32 and 64 bit modules will have to have different names. 

Remember to leave the modules disabled in your top level IIS config so the 64 bit module doesn't accidently get brought in to the 32 bit site (check your sys log if you are getting 500s and remember to restart any crashed app pools after the issue is fixed). Selectivly enable the module for each site after you bring the native module into your top level config.

If you are an automation nut and like to script everything (it's a good thing), then remember to set the correct precondition on appcmd.exe (/preCondition:"bitness32" or preCondition:"bitness64"). It's a common gotcha with mixed modules.

Changelog
---------

* 1.0 Original module
* 1.1 Modified module to pick up XFF earlier in the request pipeline

Version
-------
The current version is 1.1