function Component() {}

Component.prototype.createOperations = function() {
    // Standard operations
    component.createOperations();
    var installPath = installer.value("TargetDir");
    if (systemInfo.kernelType === "linux") {  
        component.addOperation("CreateDesktopEntry", 
            "sparsely.desktop", 
            "Type=Application\nName=Sparsely\nExec=" + installPath + "/bin/sparsely\nPath=" + installPath + "/bin\nIcon=" + installPath + "/share/icons/sparselyIcon.png\nTerminal=false\nCategories=Utility;"
        );
    }
	else if (systemInfo.kernelType === "winnt") {
        // Create shortcut in Start Menu
        component.addOperation("CreateShortcut",
            "@TargetDir@/bin/sparsely.exe",
            "@StartMenuDir@/sparsely.lnk",
            "workingDir=@TargetDir@/bin"
        );
    }
}