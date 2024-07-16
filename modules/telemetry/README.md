# Telemetry module - README

## Description
The module provides unirec telemetry of the input interface over available output plugins.

## Interfaces
- Input: 1
- Output: 1

## Parameters
### Common TRAP parameters
- `-h [trap,1]`      Print help message for this module / for libtrap specific parameters.
- `-i IFC_SPEC`      Specification of interface types and their parameters.
- `-v`               Be verbose.
- `-vv`              Be more verbose.
- `-vvv`             Be even more verbose.

### Module specific parameters
- `-o, --output outputPluginSpec` Specify the output plugin name and its parameters. Format: `pluginName:param=value,...`

## Output plugins

### appfs

The `appfs` output plugin allows telemetry statistics to be exposed via the AppFS interface. It provides a convenient way to access telemetry data as files within a specified directory.

- **Parameters:**
  - `mountPoint`: Specifies the directory path where the telemetry data will be mounted in the AppFS. **[required]**

### stdout

The `stdout` output plugin utilizes the Ncurses library to display telemetry statistics directly on the terminal in a text-based interactive format. It provides real-time monitoring capabilities for telemetry data.

- **Parameters:**
  - `interval`: Specifies the frequency (in seconds) at which telemetry data should be updated and displayed on the terminal. **[default=1]**


## Usage Examples
```
# Telemetry stats from the input unix socket interface "trap_in" are provided through appfs interface

$ telemetry_stats -i u:trap_in,u:trap_out -o appfs:mountPoint=/var/run/myModuleStats
```
