#!/usr/bin/env bash

# Restart event recording and read all events to binary

#region global varaibles
BASE_ADDR=""   # FPGA base address
SEEK_LOCKING=""  # Seek locking flag
EXIT_LOCKING=""  # Stop event writing once locking flag is found. Used only with SEEK_LOCKING.
RESTART=""  # Restart event recording process on FPGA
READ_TO_CURRENT=""  # Read all events till the current index.
TEST_SCRIPT="" # test the functions of this script. Hidden argument.
SHOW_HELP=YES

BASE_ADDR_HEX=""
CFG_REG_ADDR=""  # BASE_ADDR_HEX + 0x5000
EVENT_RAM_ADDR=""  # BASE_ADDR_HEX + 0x38000
#endregion

#region CLI functions
function read_args { # Read commande line arguments and update global vairbles
    local error_msg=""
    local i
    for i in "$@"; do # Parse arguments in arg=value format
        case $i in
            -h|--help)
                SHOW_HELP=YES
                shift # past argument
                ;;
            -b=*|--base=*)
                BASE_ADDR="${i#*=}"
                SHOW_HELP=""
                shift # past argument=value
                ;;
            -r|--read)
                READ_TO_CURRENT=YES
                SHOW_HELP=""
                shift # past argument
                ;;
            --restart)
                RESTART=YES
                SHOW_HELP=""
                shift # past argument
                ;;
            -t|--test)
                TEST_SCRIPT=YES
                SHOW_HELP=""
                shift # past argument
                ;;
            -*|--*)
                error_msg="${error_msg}\nUnknown option $1"
                shift # past argument
                ;;
            *)
                error_msg="${error_msg}\nUnknown positional option $1"
                shift # past argument
                ;;
        esac
    done
}

function show_help_msg { # pring usage message to STDOUT
    cat << EOF
    Usage:

    read_event.sh -b|--base=<BASE_ADDR> [-s|--seek] [-x|--exit] [-r|--read] [-h|--help]

    -b, --base: Mandatory, FPGA base address, e.g. -b=0x03000000.
    --restart:  Optional. Restart event recording.
    -r, --read: Optional. Stop event recoring and read all events.

    -h, --help: Optional. Print this help message.
EOF
}
#endregion

#region Utilities
function debug { # Print debug message to STDERR
    if [[ -n "$TEST_SCRIPT" ]]; then
         echo "DEBUG: $@" 1>&2
    fi
}
#endregion

#region Number conversion functions
function dec2hex { # Convert decimal to hexadecimal with 0x prefix. 0d prefix in decimal is allowed
    local dec=$1
    if [[ $dec == "0d"* ]]; then
        dec=${dec:2}
    fi
    local result=$(printf "%08x" "$dec")
    result="0x${result^^}"
    echo $result
}

function dec2bin { # Convert decimal to binary with 0b prefix. 0d prefix in decimal is allowed
    # $1 is input positive decimal number
    # $2 is output binary number width. Defaults to 32 === one 32-bit word.
    local dec=$1
    local target_width=${2:-32}
    if [[ $dec == "0d"* ]]; then
        dec=${dec:2}
    fi
    local bits=""
    local n
    for (( n=dec; n>0; n>>=1 )); do
        bits=$(( n & 1 ))$bits;
    done
    local result_width=${#bits}
    for (( i=target_width; i>result_width ; --i )); do
        bits="0${bits}"
    done
    echo "0b$bits"
}

function hex2dec { # Convert hexadecimal to decimal without prefix.
    local hex=$1
    if [[ $hex == "0x"* ]]; then
        hex=${hex:2}
    fi
    if [[ $hex == "0h"* ]]; then
        hex=${hex:2}
    fi
    local result=$(( 16#$hex ))
    echo $result
}

function to_dec { # Convert any positive number to decimal without prefix.
    local a=$1
    if [[ $a == "0x"* ]]; then
        hex2dec $a
    elif [[ $a == "0h"* ]]; then
        hex2dec $a
    elif [[ $a == "0d"* ]]; then
        echo ${a:2}
    else
        echo $a
    fi
}

function to_hex { # Convert any positive number to hexadecimal with 0x prefix.
    local a=$( to_dec $1 )
    dec2hex $a
}

function to_bin { # Convert any positive number to binary with 0b prefix.
    local width=${2:-32}
    local a=$( to_dec $1 )
    dec2bin $a $width
}

function sum_to_hex { # Sum up all arguments and output to hexadecimal with 0x prefix.
    local result=0
    local number
    local number_dec
    for number in "$@"; do
        number_dec=$( to_dec "$number" )
        result=$(( result + number_dec ))
    done
    dec2hex $result
}
#endregion

#region FPGA operations
function read_hex { # fpga read $1 -32
    if [[ -z "$1" ]]; then
        echo "Address must be provided to read_hex function"
        exit 1
    fi
    local result=$(fpga read $( to_hex $1 ) -32)
    echo "0x${result: -8}"
}

function read_bin { # fpga read $1 -32 and converted to binary
    if [[ -z "$1" ]]; then
        echo "Address must be provided to read_bin function"
        exit 1
    fi
    to_bin $( read_hex $1 )
}

function read_dec { # fpga read $1 -32 and converted to binary
    if [[ -z "$1" ]]; then
        echo "Address must be provided to read_dec function"
        exit 1
    fi
    to_dec $( read_hex $1 )
}

function write_fpga { # fpga write $1 $2
    if [[ -z "$1" ]]; then
        echo "Address must be provided to write_fpga function"
        exit 1
    fi
    if [[ -z "$2" ]]; then
        echo "Data must be provided to write_fpga function"
        exit 1
    fi
    fpga write $( to_hex $1 ) $( to_hex $2 )
}

function restart { # Restart event recording
    fpga read $ctrl_reg -32
    fpga write $ctrl_reg 0x00000000  # Stop recording events
    fpga write $mask_lo 0xFFFFFFFF
    fpga write $mask_hi 0xFFFFFFFF
    fpga write $ctrl_reg 0x00000014  # Reset event index, cycle counter and second counter
    fpga write $ctrl_reg 0x00000009 # Start recording events
    fpga read $ctrl_reg -32 # Read current ctrl register
}

function stop { # Stop event recording
    fpga write $ctrl_reg 0 > /dev/null
}
#endregion

#region Event reading functions
function get_current_idx_addr {  # Read current_idx register
    local current_idx_reg_addr=$( sum_to_hex $CFG_REG_ADDR 0x10 )
    local current_idx=$( read_hex $current_idx_reg_addr )
    local current_idx_addr="${current_idx}0" # << 4
    echo $current_idx_addr
}

function combine_to_bin { # returns ((high << 32) + low) === (($1 << 32) + $2)
    if [[ "$#" != "2" ]]; then
        echo "Two argumetns should be provided to combine_to_bin function."
    fi

    local high=$1
    local low=$2

    high=$( to_dec ${high} )
    low=$( to_dec ${low} )

    local result=$(( (high << 32)  + low ))
    to_bin $result 64
}

# Example output
# 0x03038B34: 0b0000000000000000000000000010000000000000000000000000000000000000: 28437538: 23
function read_one_event { # read one event based on provided address
    if [[ -z "$1" ]]; then
        echo "Address must be provided to read_one_event function"
        exit 1
    fi

    local addr=$( to_hex $1 )
    debug "Reading $addr"

    local current_addr
    current_addr=$( sum_to_hex $addr 0 )
    local event_addr=$current_addr
    local event_lo=$( read_hex $current_addr )

    current_addr=$( sum_to_hex $addr 4 )
    local event_hi=$( read_hex $current_addr )

    local event=$( combine_to_bin $event_hi $event_lo )

    current_addr=$( sum_to_hex $addr 8 )
    local cycles=$( read_dec $current_addr )

    current_addr=$( sum_to_hex $addr 12 )
    local seconds=$( read_dec $current_addr )

    echo "$event_addr: $event: $cycles: $seconds"
}

function read_one_block { # read 32 words by using `fpga read $1`
    if [[ -z "$1" ]]; then
        echo "Address must be provided to read_one_block function"
        exit 1
    fi
    local result=$(fpga read $(to_hex $1))
    local old_ifs=$IFS
    IFS=$'\n'
    local addr=""
    local words=( )
    local addr_word=""
    local var
    local i
    local start
    local offset
    local current_addr
    local word
    local event_lo
    local event_hi
    local event_addr
    local event
    local cycles
    local seconds
    for var in ${result[@]}; do
        var=${var//[[:blank:]]/}
        debug "var is $var"
        addr="${var:0:8}" # addr is before " : "
        for (( i=0; i<4; ++i )); do
            start=$(( 9+(i*8) ))
            offset=$(( i*4 ))
            current_addr=$( sum_to_hex 0x$addr 0d$offset )
            word="${var:${start}:8}"
            if [[ "${current_addr: -1}" == "0" ]]; then
                event_lo=$word
                event_addr=$current_addr
            elif [[ "${current_addr: -1}" == "4" ]]; then
                event_hi=$word
                event=$( combine_to_bin 0x$event_hi 0x$event_lo )
            elif [[ "${current_addr: -1}" == "8" ]]; then
                cycles=$( to_dec 0x$word )
            else # [[ "${current_addr: -1}" == "C" ]]
                seconds=$( to_dec 0x$word )
                echo "$event_addr: $event: $cycles: $seconds"
            fi
        done
    done
    IFS=$old_ifs
}

# Example output
# 0x03038B34: 0b0000000000000000000000000010000000000000000000000000000000000000: 28437538: 23
function read_till_current { # read all events one by one till current index
    local current_idx_addr=$( get_current_idx_addr )
    local start_addr
    local end_addr

    if [[ -n "$1" ]]; then
        start_addr=$1
    else
        start_addr=$( to_dec $EVENT_RAM_ADDR )
    fi

    end_addr=$( to_dec $( sum_to_hex $EVENT_RAM_ADDR $current_idx_addr ) )

    if (( start_addr > end_addr )); then
        echo "Start addr $( to_hex $start_addr) is larger than the end address $( to_hex $end_addr)"
        exit 1
    fi

    local i
    for (( i=start_addr; i<=end_addr; i+=16 )); do
        read_one_event $i
    done
}

# Example output
# 0x03038B34: 0b0000000000000000000000000010000000000000000000000000000000000000: 28437538: 23
function read_till_current_block { # read all events block by block till current index
    local current_idx_addr=$( get_current_idx_addr )
    local start_addr
    local end_addr

    if [[ -n "$1" ]]; then
        start_addr=$1
    else
        start_addr=$( to_dec $EVENT_RAM_ADDR )
    fi

    end_addr=$( to_dec $( sum_to_hex $EVENT_RAM_ADDR $current_idx_addr ) )

    if (( start_addr > end_addr )); then
        echo "Start addr $( to_hex $start_addr) is larger than the end address $( to_hex $end_addr)"
        exit 1
    fi

    local i
    for (( i=start_addr; i<=end_addr; i+=16*8 )); do
        read_one_block $i
    done
}
#endregion

#region main
read_args "$@"

BASE_ADDR_HEX=$( to_hex $BASE_ADDR )
CFG_REG_ADDR=$( sum_to_hex $BASE_ADDR_HEX "0x5000" )
EVENT_RAM_ADDR=$( sum_to_hex $BASE_ADDR_HEX "0x38000" )

ctrl_reg=$CFG_REG_ADDR
mask_lo=$( sum_to_hex $CFG_REG_ADDR 0x08 )
mask_hi=$( sum_to_hex $CFG_REG_ADDR 0x0C )

if [[ -n $error_msg ]]; then
    show_help_msg
    echo -e "\nErrors:\n${error_msg}\n"
    exit 1
fi

if [[ -n $SHOW_HELP ]]; then
    show_help_msg
    exit 0
fi


if [[ -n $RESTART ]]; then
    restart
    exit 0
fi

if [[ -n $READ_TO_CURRENT ]]; then
    stop
    read_till_current_block
    exit 0
fi
#endregion
