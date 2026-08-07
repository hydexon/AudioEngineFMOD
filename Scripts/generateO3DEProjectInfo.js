
studio.menu.addMenuItem({
    name: "O3DE FMOD AudioSystem\\Generate FMOD Project Info",
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
    outputPath  = outputPath.substr(0, outputPath.lastIndexOf("/") + 1) + 'FMODProjectInfo.json';
    console.log(outputPath);

    var allbanks  = studio.project.model.Bank.findInstances({includeDerivedTypes: true});
    var allEvents = studio.project.model.Event.findInstances();
    var allParams = studio.project.model.ParameterPreset.findInstances();
    var allBuses = studio.project.model.MixerGroup.findInstances();
    allBuses = allBuses.concat(studio.project.model.MixerReturn.findInstances());
    allBuses = allBuses.concat(studio.project.workspace.mixer.masterBus);

    var evts = [];
    allEvents.forEach(function(evt){
        evts.push(evt.getPath());
    });

    var banks = [];
    allbanks.forEach(function(bnk) {
        var bnkEvts = [];
        bnk.events.forEach(function(evt){
            bnkEvts.push(evt.getPath());
        });
        banks.push({
            name: bnk.name,
            isLocalized: bnk.audioTable !== undefined && bnk.audioTable !== null,
            events: bnkEvts
        });
    });

    var params = [];
    allParams.forEach(function(ps) {
        console.log("Minimum: {0}, Maximum: {2}, initialValue: {2}".format(ps.parameter.minimum, ps.parameter.maximum, ps.parameter.initialValue));
        params.push({
            path: ps.getPath(),
            labels: ps.parameter.enumerationLabels,
            isGlobal: ps.parameter.isGlobal,
            minimum: ps.parameter.minimum,
            maximum: ps.parameter.maximum,
            initialValue: ps.parameter.initialValue
        });
    });

    var buses = [];
    allBuses.forEach(function(bus) {
        buses.push(bus.getPath());
    });
    var allSnapshots = studio.project.model.Snapshot.findInstances();
    var snapshts = [];
    allSnapshots.forEach(function(ss){
        snapshts.push(ss.getPath());
    });

    var FMODExportData = {
        events: evts,
        banks: banks,
        parameters: params,
        buses: buses,
        snapshots: snapshts
    };

    var jsonFile = studio.system.getFile(outputPath);
    if (!jsonFile.open(studio.system.openMode.WriteOnly)) {
        alert("Failed to open file {0}\n\nCheck the file is not read-only.".format(outputPath));
        console.error("Failed to open file {0}.".format(outputPath));
        return;
    }

    jsonFile.writeText(JSON.stringify(FMODExportData, null, 2));
    jsonFile.close();

    alert("JSON file successfully created at, please copy it to your O3DE project's Assets/Audio/FMOD/ directory:\n\n{0}".format(outputPath));
    console.log("JSON file successfully created at: " + outputPath);

}
