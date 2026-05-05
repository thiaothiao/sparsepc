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
    }
	else if (systemInfo.kernelType === "darwin") 
	{
	    const desktopDir = installer.value("DesktopDir");
        component.addOperation("CreateShortcut",
            installPath + "/" + baseName + ".app", desktopDir + "/" + baseName + ".app");
	}
}
