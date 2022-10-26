#!/usr/bin/env bash
BASE_ADDR=""   # FPGA base address
SEEK_LOCKING=""  # Seek locking flag
EXIT_LOCKING=""  # Stop event writing once locking flag is found. Used only with SEEK_LOCKING.
RESTART=""  # Restart event recording process on FPGA
READ_TO_CURRENT=""  # Read all events till the current index.
TEST_SCRIPT="" # test the functions of this script. Hidden argument.

bits_to_check=( )

#region Read Commandline arguments
SHOW_HELP=YES
error_msg=""

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
        -s|--seek)
            SEEK_LOCKING=YES
            SHOW_HELP=""
            shift # past argument
            ;;
        -x|--exit)
            EXIT_LOCKING=YES
            SHOW_HELP=""
            shift # past argument
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
            bits_to_check+=("$1") # save positional arg for check bits
            shift # past argument
            ;;
    esac
done

function show_help_msg { # pring usage message to STDOUT
    cat << EOF
    Usage:

    read_event.sh -b|--base <BASE_ADDR> [-s|--seek] [-x|--exit] [-r|--read] [-h|--help]

    -b, --base: Mandatory, FPGA base address, e.g. 0x03000000.
    -s, --seek: Optinal. Poll FPGA for HFN locking flag and print the event index.
                Event writing is NOT stopped.
    -x, --exit: Optinal. Stop event writing once HFN locking flag is found so event index will not change.
                Used with -s or --seek.
    -r, --read: Optinal. Read all events till the current event index to STDOUT.
    -h, --help: Optinal. Print this help message.
EOF
}

if [[ -n $error_msg ]]; then
    show_help_msg
    echo -e "\nErrors:\n${error_msg}\n"
    exit 1
fi

if [[ -n $SHOW_HELP ]]; then
    show_help_msg
    exit 0
fi

function debug { # Print debug message to STDERR
    if [[ -n "$TEST_SCRIPT" ]]; then
         echo "DEBUG: $@" 1>&2
    fi
}

function debug_hex { # Print the hex value of the provided varible
    local a=$1
    debug $a is $( to_hex ${!a} )
}
#endregion



#region Number conversion functions
function dec2hex { # Decimal to hexadecimal conversion
    local dec=$1
    if [[ $dec == "0d"* ]]; then
        dec=${dec:2}
    fi
    local result=$(printf "%08x" "$dec")
    result="0x${result^^}"
    echo $result
}

function dec2bin { # Decimal to binary conversion
    # $1 is input positive decimal number
    # $2 is output binary number width. Defaults to 32 === one 32-bit word.
    local dec=$1
    local target_width=${2:-32}
    # debug input to dec2bin is $1
    if [[ $dec == "0d"* ]]; then
        dec=${dec:2}
    fi
    local bits=""
    for (( n=dec; n>0; n>>=1 )); do
        bits=$(( n & 1 ))$bits;
    done
    local result_width=${#bits}
    for (( i=target_width; i>result_width ; --i )); do
        bits="0${bits}"
    done
    echo "0b$bits"
}

function hex2dec {
    hex=$1
    if [[ $hex == "0x"* ]]; then
        hex=${hex:2}
    fi
    if [[ $hex == "0h"* ]]; then
        hex=${hex:2}
    fi
    result=$(( 16#$hex ))
    echo $result
}

function to_dec {
    a=$1
    if [[ $a == "0x"* ]]; then
        echo $( hex2dec $a )
    elif [[ $a == "0h"* ]]; then
        echo $( hex2dec $a )
    elif [[ $a == "0d"* ]]; then
        echo ${a:2}
    else
        echo $a
    fi
}

function to_hex {
    a=$( to_dec $1 )
    dec2hex $a
}

function to_bin {
    local width=${2:-32}
    # debug input to to_dec is $1
    a=$( to_dec $1 )
    dec2bin $a $width
}

function sum_to_hex {
    local result=0
    for number in "$@"; do
        number_dec=$( to_dec "$number" )
        result=$(( result + number_dec ))
    done
    echo $( dec2hex $result )
}
#endregion

#region Tests
if [[ $(dec2hex 33)=="0x00000021" ]]; then
    debug dec2hex passed
fi

if [[ $(dec2hex 33)=="0b00000000000000000000000000100001" ]]; then
    debug dec2bin passed
fi
#endregion

BASE_ADDR_HEX=$( to_hex $BASE_ADDR )
CFG_REG_ADDR=$( sum_to_hex $BASE_ADDR_HEX "0x5000" )
EVENT_RAM_ADDR=$( sum_to_hex $BASE_ADDR_HEX "0x38000" )

ctrl_reg=$CFG_REG_ADDR
mask_lo=$( sum_to_hex $CFG_REG_ADDR 0x08 )
mask_hi=$( sum_to_hex $CFG_REG_ADDR 0x0C )

function restart { # Restart event writing
    fpga read $ctrl_reg -32
    fpga write $ctrl_reg 0x00000000  # Stop recording events
    fpga write $mask_lo 0xFFFFFFFF
    fpga write $mask_hi 0xFFFFFFFF
    fpga write $ctrl_reg 0x00000014  # Reset event index, cycle counter and second counter
    fpga write $ctrl_reg 0x00000009 # Start recording events
    fpga read $ctrl_reg -32 # Read current ctrl register
}

#region Event reading functions
function get_current_idx_addr {  # Read current_idx register
    if [[ -n "$is_test" ]]; then
        current_idx_raw="0300 5010 : 000001AA"
    else
        current_idx_reg_addr=$( sum_to_hex CFG_REG_ADDR 0x10 )
        current_idx_raw=$(fpga read $current_idx_reg_addr "-32")
    fi
    current_idx=${current_idx_raw: -8}
    current_idx_addr="0x${current_idx}0" # << 4
    echo $current_idx_addr
    debug_hex current_idx_addr
}

function get_start_end_addr {  # Get start addr and end addr based on current_idx
    # $1 should be the preivous current_idx

    current_idx_addr=$( get_current_idx_addr )

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

    debug_hex start_addr
    debug_hex end_addr

    echo $start_addr $end_addr
}

function read_hex { # fpga read $1 -32
    if [[ -z "$1" ]]; then
        echo "Address must be provided to read_hex function"
        exit 1
    fi
    local result=$(fpga read $1 -32)
    result="0x${result: -8}"
    debug input of read_hex is $1
    debug output of read_hex is $result
    echo $result
}

function read_bin { # fpga read $1 -32 and converted to binary
    if [[ -z "$1" ]]; then
        echo "Address must be provided to read_bin function"
        exit 1
    fi
    local result=$( read_hex $1 )
    result=$( to_bin $result )
    debug input of read_bin is $1
    debug output of read_bin is $result
    echo $result
}

function read_dec { # fpga read $1 -32 and converted to binary
    if [[ -z "$1" ]]; then
        echo "Address must be provided to read_dec function"
        exit 1
    fi
    local result=$( read_hex $1 )
    result=$( to_dec $result )
    debug input of read_dec is $1
    debug output of read_dec is $result
    echo $result
}

function combine_to_bin { # returns ((high << 32) + low) === (($1 << 32) + $2)
    if [[ "$#" != "2" ]]; then
        echo "Two argumetns should be provided to combine_bin function."
    fi

    local high=$1
    local low=$2

    high=$( to_dec ${high} )
    low=$( to_dec ${low} )

    # debug "high in decimal is $high"
    # debug "low in decimal is $low"

    local result=$(( (high << 32)  + low ))
    result=$( to_bin $result 64)
    # debug "combine_to_bin output is $result"
    echo $result
}

function read_one_event { # read one event based on provided address
    if [[ -z "$1" ]]; then
        echo "Address must be provided to read_one_event function"
        exit 1
    fi
    addr=$1
    debug "Reading $addr"

    current_addr=$( sum_to_hex $addr 0)
    event_lo=$(read_hex $current_addr)

    current_addr=$( sum_to_hex $addr 4)
    event_hi=$(read_hex $current_addr)

    event=$( combine_to_bin $event_hi $event_lo )

    current_addr=$( sum_to_hex $addr 8)
    cycles=$(read_dec $current_addr)

    current_addr=$( sum_to_hex $addr 12)
    seconds=$(read_dec $current_addr)


}

function read_one_block { # read 32 words by using `fpga read $1`
    if [[ -z "$1" ]]; then
        echo "Address must be provided to read_one_block function"
        exit 1
    fi
    local result=$(fpga read $1)
    local old_ifs=$IFS
    IFS=$'\n'
    local addr=""
    local words=( )
    local addr_word=""
    for var in ${result[@]}; do
        var=${var//[[:blank:]]/}
        addr="${var:0:8}" # addr is before " : "
        # debug var is $var
        for (( i=0; i<4; ++i )); do
            start=$(( 9+(i*8) ))
            offset=$(( i*4 ))
            current_addr=$( sum_to_hex 0x$addr 0d$offset )
            addr_word="$addr_word ${current_addr}:0x${var:${start}:8} "
        done
    done
    IFS=$old_ifs
    echo $addr_word
}

function check_one_event { # Check event bin, e.g. check_one_event --event=00000000000... 3=1 4=1

    local bit_indexes=( )
    local expected_values=( )
    local event_bin=""

    debug check_one_event arguments are "$@"

    for i in "$@"; do # Parse arguments in arg=value format
        case $i in
            --event=*)
                event_bin="${i#*=}"
                shift
                ;;
            *=*)
                bit_indexes+=("${i%=*}")
                expected_values+=("${i#*=}")
                shift
                ;;
            *)
                bit_indexes+=("$i")
                expected_values+=("1")
                shift
                ;;
        esac
    done

    event_bin=${event_bin:2}

    # debug "bit_indexes is ${bit_indexes[@]}"
    # debug "expected_values is ${expected_values[@]}"
    # debug "event_bin is $event_bin"

    for i in "${!bit_indexes[@]}"; do  # Check all bit_indexes for expected value
        bit_index="${bit_indexes[$i]}"
        expected="${expected_values[$i]}"

        index=$(( 63 - bit_index ))  # reversed index since bash index 0 is MSB
        value=${event_bin:$index:1}
        # debug "i is $i"
        # debug "bit_index value expected are $bit_index, $value, $expected"

        if [[ "$value" == "$expected" ]]; then
            # value == expected
            # debug value == expected
            echo "${bit_index}=${expected}"
            return 0
        fi
    done
}

# fixme Not working yet
function seek_lock { # seek lock flags provided by index=value or index in $@, e.g. --start=0x03038000 --end=0x03039000 1=1 32=1 15

    start_addr=""
    end_addr="" # inclusive end

    debug seek_lock arguments are "$@"

    local expected_indexes_values=( )

    for i in "$@"; do # Parse arguments in arg=value format
        case $i in
            --start=*)
                start_addr="${i#*=}"
                shift
                ;;
            --end=*)
                end_addr="${i#*=}"
                shift
                ;;
            *)
                expected_indexes_values+=("${i}")
                shift
                ;;
        esac
    done

    # debug expected_indexes_values are ${expected_indexes_values[@]}
    debug "start_addr is $start_addr"
    debug "end_addr is $end_addr"

    end_addr_dec=$( to_dec $end_addr )

    one_block=($(read_one_block $start_addr | tr " " "\n")) # fixme here
    start_addr_dec=$( to_dec $start_addr )
    # debug one_block is "${one_block[@]}"
    local found=""
    while true; do
        if (( ))
        for addr_words in "${one_block[@]}"; do
            debug addr_words is $addr_words
            addr="${addr_words%:*}"
            word="${addr_words#*:}"
            # debug addr is $addr
            # debug word is $word
            # debug addr[-1] is "${addr: -1}"
            if [[ "${addr: -1}" == "0" ]]; then
                event_lo=$word
                # debug "event_lo is $event_lo"
            elif [[ "${addr: -1}" == "4" ]]; then
                event_hi=$word
                # debug "event_hi is $event_hi"
                event_bin=$( combine_to_bin $event_hi $event_lo )
                check_result=$(check_one_event --event=$event_bin "${expected_indexes_values[@]}")
                # debug "check_result is >$check_result<"
                if [[ -n "$check_result" ]]; then  # found expected value
                    echo $check_result
                    found=YES
                    break
                fi
            elif [[ "${addr: -1}" == "8" ]]; then
                cycle_counter=$word
            else # [[ "${addr: -1}" == "C" ]]
                second_counter=$word
            fi
        done
        if [[ -n "$found" ]]; then  # found expected value
            break
        fi
    done
}


if [[ -n $RESTART ]]; then
    restart
    exit 0
fi

if [[ -n $READ_TO_CURRENT ]]; then
    read_till_current
    exit 0
fi

if [[ -n $SEEK_LOCKING ]]; then
    # fixme Not working yet
    echo "seeking locking ..."
    debug bits_to_check is "${bits_to_check[@]}"
    # seek_lock "--start=0x03038000 --end=0x03038010 ${bits_to_check[@]}"
    exit 0
fi
