param(
	[string]$cmd = "",
	[string]$module_path,
	[string]$module_name = "F5XFFHttpModule",
	[string]$app_name = ""
);

$WINDIR = $env:windir;
$APPCMD = "$WINDIR\\system32\\inetsrv\\appcmd.exe";
$CMDTORUN = "";
$MODULE_NAME = "F5XFFHttpModule";


function Write-Usage()
{
	Write-Host "Usage: Install-Xff.ps1";
	Write-Host "         -cmd <list|install|uninstall|add|remove>";
	Write-Host "         -module_path <path_to_module_dll>";
	Write-Host "         -module_name <module_name> (defaults to F5XFFHttpModule)";
	Write-Host "         -app_name <app_name> (used for add/remove for specific app)";
}


switch ($cmd.ToLower())
{
	"list" {
		$CMDARGS = "list modules";
	}
	"install" {
		if ( $module_path.Length -gt 0 )
		{
			$CMDARGS = "install module /name:$module_name /image:$module_path";
		}
	}
	"uninstall" {
		$CMDARGS = "uninstall module $module_name";
	}
	"add" {
		$CMDARGS = "add module /name:$module_name";
		if ( $app_name.Length -gt 0 ) { $CMDARGS += " /app.name:$app_name/"; }
	}
	"remove" {
		$CMDARGS = "delete module $module_name";
		if ( $app_name.Length -gt 0 ) { $CMDARGS += " /app.name:$app_name/"; }
		
	}
}

if ( $CMDARGS.Length -gt 0 )
{
	& $APPCMD $CMDARGS.Split(' ');
}
else
{
	Write-Usage;
}