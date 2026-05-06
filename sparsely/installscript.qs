function Component() {}

Component.prototype.createOperations = function() 
{
    component.createOperations(); 
    
    const installPath = installer.value("TargetDir");
    const baseName = "sparsely";
    
    if (systemInfo.kernelType === "linux") 
    { 
        const homeDirLocal = installer.value("HomeDir") + "/.local";
        const installedDesktopPath = installPath + "/share/applications/" + baseName + ".desktop";
        const userDesktopPath = homeDirLocal + "/share/applications/" + baseName + ".desktop";
        component.addOperation("Replace", installedDesktopPath, "tmp", installPath);
        component.addOperation("Copy", installedDesktopPath, userDesktopPath);
        component.addOperation("Execute", "chmod", "+x", userDesktopPath);
        
		const iconsFolderPath = homeDirLocal + "/share/icons";
		if (!installer.fileExists(iconsFolderPath)) 
		{
			component.addOperation("Mkdir", iconsFolderPath);
		}

        const installedIconPath = installPath + "/share/icons/icon.png";
        component.addOperation("Copy",	installedIconPath, iconsFolderPath + "/" + baseName + "Icon.png");
    }
    else if (systemInfo.kernelType === "winnt") 
    {
        const startMenuDir = installer.value("StartMenuDir");
        component.addOperation("CreateShortcut", installPath + "/bin/" + baseName + ".exe",
            	startMenuDir + "/" + baseName + ".lnk", "workingDir=" + installPath + "/bin");
				
        const vcredistKey = "HKEY_LOCAL_MACHINE\\SOFTWARE\\WOW6432Node\\Microsoft\\VisualStudio\\14.0\\VC\\Runtimes\\x64";
        const isInstalled = installer.execute("reg", ["QUERY", vcredistKey, "/v", "Installed"])[0];
        if (!isInstalled) 
		{            
            // Add elevated operation to run the installer silently	
			component.addElevatedOperation("Execute", installPath + "/bin/vc_redist.x64.exe", 
                "/passive", "/norestart");	
        }
    }
	else if (systemInfo.kernelType === "darwin") 
	{
	    const desktopDir = installer.value("DesktopDir");
        component.addOperation("CreateShortcut",
            installPath + "/" + baseName + ".app", desktopDir + "/" + baseName + ".app");
	}
}
