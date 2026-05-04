function Controller() 
{
    const version = installer.value("ProductVersion");
    installer.setValue("TargetDir", installer.value("HomeDir") + "/Sparsely-"+ version);
}

Controller.prototype.TargetDirectoryPageCallback = function() 
{
    var widget = gui.currentPageWidget();
    if (widget) 
	{
        widget.TargetDirectoryLineEdit.setEnabled(false);
        widget.TargetDirectoryChooser.setEnabled(false);
    }
}

