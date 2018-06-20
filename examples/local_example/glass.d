# glass.d
# glass configuration file
{
	# this config is for the glass broker app
	"Configuration":"glass-broker-app",

	# Set this logging level
	# trace, debug, info, warning, error, criticalerror
	"LogLevel":"debug",

	# Use this directory for the other glass
	# subcomponent configuration files
	"ConfigDirectory":"C:\\hydra\\glass-broker-app\\hi_params",

	# Association thread configuration
	# The file containing the configuration
	# to initialize glass.
	"InitializeFile":"initialize.d",

	# The file containing the initial station list.
	"StationList":"StationInfoList_Culled.json",

	# List of files containing the configuration
	# to define 1 or more regional/local grids
	"GridFiles":[
		"hi_grid.d"
	],

	# The file containing the configuration
	# for the input thread.
	"InputConfig":"input.d",

	# The file containing the configuration
	# for the output thread.
	"OutputConfig":"output.d"
}
# End of glass.d
