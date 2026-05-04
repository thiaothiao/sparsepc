// --- controller.qs ---
function Controller() 
{
    // Defines the directory in the UI
    const version = installer.value("ProductVersion");
    installer.setValue("TargetDir", installer.value("HomeDir") + "/Sparsely-"+ version);
}

Controller.prototype.TargetDirectoryPageCallback = function() 
{
    var widget = gui.currentPageWidget();
    // Prevent user from changing the directory
    if (widget) {
        widget.TargetDirectoryLineEdit.setEnabled(false);
        widget.TargetDirectoryChooser.setEnabled(false);
    }
}

