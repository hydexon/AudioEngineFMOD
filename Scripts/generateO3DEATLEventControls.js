
studio.menu.addMenuItem({
    name: "O3DE FMOD AudioSystem\\Generate ATL Event Controls",
    execute: function() {
        outputAllBanksEvents();
    }
});


function outputAllBanksEvents()
{
    /*
    studio.ui.showModalDialog({
        windowTitle: 'O3DE Project Information',
        widgetType: studio.ui.widgetType.Layout,
        layout: studio.ui.layoutType.VBoxLayout,
        items: [
        {
            widgetType: studio.ui.widgetType.PathLineEdit,
            label: "O3DE Project's Audio Controls directory",
            pathType: studio.ui.pathType.Directory
        },
        {
            widgetType: studio.ui.widgetType.PathLineEdit,
            label: 'Bank Dependencies Output Folder',
            pathType: studio.ui.pathType.Directory
        },
        {
            widgetType: studio.ui.widgetType.CheckBox,
            text: "Create Default Controls XML for all events"
        }
        ]

    });
    */
    var outputPath = studio.project.filePath;
    var projectName = outputPath.substr(outputPath.lastIndexOf("/") + 1, outputPath.length);
    projectName =  projectName.substring(0, projectName.lastIndexOf('.'));

    var allbanks  = studio.project.model.Bank.findInstances({includeDerivedTypes: true});
    var allEvents = studio.project.model.Event.findInstances();
    var allParams = studio.project.model.ParameterPreset.findInstances();


    var xml = "<?xml version=\"1.0\" encoding=\"utf-8\"?>\n";
    xml += "<ATLConfig atl_name=\""+projectName+"\">\n";
    xml += "  <AudioTriggers>\n";

    allEvents.forEach(function(evt) {
        xml += "    <ATLTrigger atl_name=\""+evt.name.split(" ").join("_") +"\" path=\"z_placeholders\">\n";
        xml += "        <FMODStudioEvent path=\""+evt.getPath()+"\"/>\n"
        xml += "    </ATLTrigger>\n";
    });

    /*
    allbanks.forEach(function(bank) {
        console.log("Bank: " + bank.name);

        if(bank.events.length > 0) {
            bank.events.forEach(function(evt) {
                console.log(" - Event: " + evt.getPath() + " ("+ evt.name + ")");
            });
        }
        else {
            console.log(" - (No events assigned)");
        }
    });
    */

    xml += "  </AudioTriggers>\n"
    xml += "  <AudioRtpcs>\n";
    allParams.forEach(function (param) {
        var paramPath = param.getPath();
        paramPath = paramPath.substr(0, paramPath.lastIndexOf("/"));
        paramPath = paramPath.slice("parameter:/".length);
        xml += "    <ATLRtpc atl_name=\""+param.name+"\" path=\""+ paramPath +"\">\n";
        xml += "        <FMODStudioParamPreset path=\""+param.getPath()+"\" />\n";
        xml += "    </ATLRtpc>\n";
    });
    xml += "  </AudioRtpcs>\n";
    xml += "  <AudioSwitches>\n";
    //TODO: Implement Switches and States.
    xml += "  </AudioSwitches>\n";
    xml += "  <AudioEnvironments>\n";
    //TODO: Implement Environments here.
    xml += "  </AudioEnvironments>\n";
    //TODO: Bank Preloads?
    xml += "</ATLConfig>";
    console.log(xml);
}
