
studio.menu.addMenuItem({
    name: "O3DE FMOD AudioSystem\\Generate FMOD Project Info",
    execute: () => {
        var outputPath = studio.project.filePath;
        var projectName = outputPath.substr(outputPath.lastIndexOf("/") + 1, outputPath.length);
        projectName =  projectName.substring(0, projectName.lastIndexOf('.'));
        outputPath  = outputPath.substr(0, outputPath.lastIndexOf("/") + 1) + 'FMODProjectInfo.json';
        if(exportFmodProjectData(outputPath))
        {
            alert("JSON file successfully created at, please copy it to your O3DE project's Assets/Audio/FMOD/ directory:\n\n{0}".format(outputPath));
        }
        else
        {
            alert("Failed to open file {0}\n\nCheck the file is not read-only.".format(outFilePath));
        }
    }
});

studio.project.buildEndedConnect( success => {
    if(!success) { //Don't generate O3DE project info if the build failed.
        return;
    }

    //Go a directory up!
    var path = studio.project.workspace.builtBanksOutputDirectory;
    if (path.endsWith("/")) path = path.substring(0,path.length-1)
        const lastSlashPosition = path.lastIndexOf("/");

    var outputInfoPath = path.substring(0,lastSlashPosition);
    outputInfoPath += "/" + "FMODProjectInfo.json";

    if(!exportFmodProjectData(outputInfoPath))
    {
        console.log("Failure creating O3DE Project Info in the postbuild event");
    }
});

function isStringEmpty(value) {
    return value == null || (typeof value === "string" && value.trim().length === 0);
}

function exportFmodProjectData(outFilePath) {

    var allBanks  = studio.project.model.Bank.findInstances({includeDerivedTypes: true});
    var allReturns = studio.project.model.MixerReturn.findInstances();

    const getParameters = (paramPresets) => {
        return paramPresets.map( pp => {
            return {
                name: pp.presetOwner.name,
                path: pp.presetOwner.getPath(),
                initialValue: pp.initialValue,
                labels: pp.enumerationLabels,
                minimum: pp.minimum,
                maximum: pp.maximum
            };
        });
    };

    const getEvtData = (evt) => ({
        type: "Event",
        name: evt.name,
        path: evt.getPath(),
        parameters: getParameters(evt.getParameterPresets())
    });

    const transverseEventFolder = (evtFolder) => {
        var items = [];
        evtFolder.items.forEach( item => {
            if(item.isOfType("EventFolder") || item.isOfType("MasterEventFolder"))
            {
                if(item.items.length > 0) {
                    items.push({
                        type: "Folder",
                        name: item.name,
                        items: transverseEventFolder(item)
                    });
                }
            }
            else if(item.isOfType("Event"))
            {
                items.push(getEvtData(item));
            }
        });

        return items;
    };
    var events = transverseEventFolder(studio.project.workspace.masterEventFolder);

    const transverseSnapshotGroup = (snapshotGrp) => {
        var items = [];

        snapshotGrp.items.forEach( child => {
            if(child.isOfType("SnapshotList") || child.isOfType("SnapshotGroup")) {
                if(child.items.length > 0)
                    items.push({
                        type: "Folder",
                        name: child.name,
                        items: transverseSnapshotGroup(child)
                    });
            }
            else if(child.isOfType("Snapshot")) {
                items.push({
                    type: "Snapshot",
                    name: child.name,
                    path: child.getPath(),
                    params: getParameters(child.getParameterPresets())
                });
            }
        });

        return items;
    };
    var snapshots = transverseSnapshotGroup(studio.project.workspace.mixer.snapshotList, "Snapshots");

    var banks = allBanks.map( bank => {
        return {
            name: "{0}.bank".format(bank.name),
            isLocalized: bank.audioTable !== undefined && bank.audioTable !== null,
        };
    });

    const transverseParameterPresetsFolder = (PPFolder) => {
        var paramData = [];
        PPFolder.items.forEach( pp => {
            if(pp.isOfType("ParameterPresetFolder")) {
                var entry = {
                    type: "Folder",
                    name: pp.name,
                    items: transverseParameterPresetsFolder(pp)
                };
                if(entry.items.length > 0)
                    paramData.push(entry);
            }
            else if(pp.isOfType("ParameterPreset")) {
                if(pp.parameter.isGlobal) {
                    paramData.push({
                        type: "Parameter",
                        name: pp.name,
                        path: pp.getPath(),
                        initialValue: pp.parameter.initialValue,
                        labels: pp.parameter.enumerationLabels,
                        minimum: pp.parameter.minimum,
                        maximum: pp.parameter.maximum
                    });
                }
            }
        });

        return paramData;
    };
    var parameters = transverseParameterPresetsFolder(studio.project.workspace.masterParameterPresetFolder);
    var returns = allReturns.map( r => ({ name: r.name, path: r.getPath() })); //Should have the nested object structures like events, snapshots, and parameters have?

    var exportData = {
        banks,
        events,
        parameters,
        snapshots,
        returns
    };

    var jsonFile = studio.system.getFile(outFilePath);
    if (!jsonFile.open(studio.system.openMode.WriteOnly)) {
        console.log("Failed to open file {0}.".format(outFilePath));
        return false;
    }

    jsonFile.writeText(JSON.stringify(exportData));
    jsonFile.close();

    console.log("JSON file successfully created at: " + outFilePath);
    return true;

}
