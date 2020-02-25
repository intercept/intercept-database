class CfgPatches {
	class intercept_database {
		name = "Intercept Database";
		units[] = {};
		weapons[] = {};
		requiredVersion = 1.82;
		requiredAddons[] = {"intercept_core"};
		author = "Dedmen";
		authors[] = {"Dedmen"};
		url = "https://github.com/intercept/intercept-database";
		version = "1.5";
		versionStr = "1.5";
		versionAr[] = {1,5};
	};
};
class Intercept {
    class Dedmen { //Change this. It's either the name of your project if you have more than one plugin. Or just your name.
        class intercept_database { //Change this.
            pluginName = "intercept-database"; //Change this.
        };
    };
};
