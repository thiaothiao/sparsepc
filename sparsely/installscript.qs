function Component() {}

Component.prototype.createOperations = function() 
{
    component.createOperations(); 
    
    const installPath = installer.value("TargetDir");
    const isSystemWide = installer.value("AllUsers") === "true";
    const baseName = "sparsely";
    
    if (systemInfo.kernelType === "linux") 
    { 
        const homeDirLocal = isSystemWide ? "/usr/local" : (installer.value("HomeDir") + "/.local");
        const installedDesktopPath = installPath + "/share/applications/" + baseName + ".desktop";
        const userDesktopPath = homeDirLocal + "/share/applications/" + baseName + ".desktop";
        component.addOperation("Replace", installedDesktopPath, "tmp", installPath);
        component.addOperation("Copy", installedDesktopPath, userDesktopPath);
        component.addOperation("Execute", "chmod", "+x", userDesktopPath);
        
	const scalableFolderPath = homeDirLocal + "/share/icons/hicolor/scalable/apps";
	if (!installer.fileExists(scalableFolderPath)) 
	{
            component.addOperation("Mkdir", scalableFolderPath);
	}

        const installedIconPath = installPath + "/share/icons/icon.svg";
        component.addOperation("Copy",	installedIconPath, scalableFolderPath + "/" + baseName + "Icon.svg");
    }
    else if (systemInfo.kernelType === "winnt") 
    {
        // Create shortcut in Start Menu
        const startMenuDir = installer.value("StartMenuDir");
        component.addOperation("CreateShortcut", installPath + "/bin/" + baseName + ".exe",
            	startMenuDir + "/" + baseName + ".lnk", "workingDir=" + installPath + "/bin");
    }
}
