
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

function exportFmodProjectData(outFilePath)
{
    var allBanks  = studio.project.model.Bank.findInstances({includeDerivedTypes: true});
    var allEvents = studio.project.model.Event.findInstances();
    var allSnapshots = studio.project.model.Snapshot.findInstances();
    var allParams = studio.project.model.ParameterPreset.findInstances();
    var allGroups = studio.project.model.MixerGroup.findInstances();
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

    var banks = allBanks.map( bank => {
        var evts = bank.events.map( evt => {
            return {
                name: evt.name,
                path: evt.getPath(),
                parameters: getParameters(evt.getParameterPresets())
            };
        } );

        return {
            name: bank.name,
            isLocalized: bank.audioTable !== undefined && bank.audioTable !== null,
            events: evts
        };
    });

    var events = allEvents.map( evt => {
        return {
            name: evt.name,
            path: evt.getPath(),
            parameters: getParameters(evt.getParameterPresets())
        };
    });

    var parameters = allParams.filter( pp => pp.parameter.isGlobal == true).map(pp => {
        return {
            name: pp.name,
            path: pp.getPath(),
            initialValue: pp.parameter.initialValue,
            labels: pp.parameter.enumerationLabels,
            minimum: pp.parameter.minimum,
            maximum: pp.parameter.maximum
        };
    });

    var snapshots = allSnapshots.map( ss => {
        return {
            name: ss.name,
            path: ss.getPath(),
            params: getParameters(ss.getParameterPresets())
        }
    });

    var groups = allGroups.map( mg => ({
        name: mg.name,
        path: mg.getPath(),
        output: mg.output?.getPath() //TODO: Should be name instead?
    }));

    var returns = allReturns.map( r => ({ name: r.name, path: r.getPath(), output: r.output?.getPath() }));

    var exportData = {
        banks,
        events,
        parameters,
        snapshots,
        groups,
        returns
    };

    var jsonFile = studio.system.getFile(outFilePath);
    if (!jsonFile.open(studio.system.openMode.WriteOnly)) {
        console.log("Failed to open file {0}.".format(outFilePath));
        return false;
    }

    jsonFile.writeText(JSON.stringify(exportData, null, 2));
    jsonFile.close();

    console.log("JSON file successfully created at: " + outFilePath);
    return true;

}
