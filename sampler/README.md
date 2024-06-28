# sampler module - README

## Description
The module performs sampling of unirec interface data

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
- `-r --rate <int>`  Specify the sampling rate 1:r. Every -rth sample will be forwarded to the output.


## Usage Examples
```
# Data from the input unix socket interface "trap_in" is processed and sampled accroding to defined rate"

$ sampler -r 8 -i u:trap_in,u:trap_out
```
