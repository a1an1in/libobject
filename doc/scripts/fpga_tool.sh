#!/usr/bin/env bash

function parse_args { # Read commande line arguments and update global vairbles
    local error_msg=""
    local i
    for i in "$@"; do # Parse arguments in arg=value format
        case $i in
            read_eci_iq_ingress|help)
                CMD_NAME=$i
                OPTION_HELP=""
                shift # past argument=value
                ;;
            -p=*|--port=*)
                OPTION_PORT="${i#*=}"
                OPTION_HELP=""
                shift # past argument=value
                ;;
            -a=*|--address=*)
                OPTION_ADDRESS="${i#*=}"
                OPTION_HELP=""
                shift # past argument
                ;;
            -s=*|--size=*)
                OPTION_SIZE="${i#*=}"
                OPTION_HELP=""
                shift # past argument
                ;;
            -f=*|--file=*)
                OPTION_FILE="${i#*=}"
                OPTION_HELP=""
                shift # past argument
                ;;
            *)
                error_msg="Unknown option $i"
                OPTION_HELP="true"
                shift # past argument
                ;;
        esac
    done
}

function do_help { # pring usage message to STDOUT
cat << EOF
    Usage:
    fpga_tool.sh read_eci_iq_ingress | help 
            [-p|--port=<port number>] [-a|--address=<address>]
            [-s|--size=<read or write size>]
    
    commands:
    read_eci_iq_ingress    build this repo.
        -p, --port:        port number.
        -a, --address:     read or write address.
        -s, --size:        read or write size

    help                   Print this help message.

    demos:
    ./fpga_tool.sh read_eci_iq_ingress -p=0 -a=0x0303501 -s=10
EOF
}

function process_args {
    if [[ $CMD_NAME == "read_eci_iq_ingress" ]]; then
        do_read_eci_iq_ingress
    else
        OPTION_HELP="true"
    fi

    if [[ -n $error_msg ]]; then
        do_help
        echo -e "\nErrors:\n${error_msg}\n"
        exit 1
    fi

    if [[ $OPTION_HELP == "true" ]]; then
        do_help
        exit 0
    fi
}

# Convert decimal to hexadecimal with 0x prefix. 0d prefix in decimal is allowed
function dec2hex { 
    local dec=$1
    if [[ $dec == "0d"* ]]; then
        dec=${dec:2}
    fi
    local result=$(printf "%08x" "$dec")
    result="0x${result^^}"
    echo $result
}

# Convert decimal to binary with 0b prefix. 0d prefix in decimal is allowed
# $1 is input positive decimal number
# $2 is output binary number width. Defaults to 32 === one 32-bit word.
function dec2bin {
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
    local result_width=${#bits}        # ${#bits} compute bits length
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

function read_fpga { # fpga read $1 -32
    if [[ -z "$1" ]]; then
        echo "Address must be provided to read_hex function"
        exit 1
    fi
    local result=$(fpga read $( to_hex $1 ) -32)
    # local result=123456
    echo "0x${result: -8}"
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

function do_read_eci_iq_ingress {
    local value
    local array=()
    echo "do_read_eci_iq_ingress $OPTION_PORT $OPTION_ADDRESS $OPTION_SIZE"
    if [[ -f $OPTION_FILE ]]; then
        echo "output file name:$OPTION_FILE"
        rm -rf $OPTION_FILE
    fi
    
    for (( i=0; i<$OPTION_SIZE; ++i )); do
        local index=$(($i % 16))
        
        write_fpga $(($(to_dec 0x03035024) + $OPTION_PORT * $(to_dec 0x100))) $i
        # echo write_fpga $(to_hex $(( $(to_dec 0x03035024) + $OPTION_PORT * $(to_dec 0x100)))) $i
        value=$(read_fpga $(to_hex $(($(to_dec 0x03035028) + $OPTION_PORT * $(to_dec 0x100)))))
        # echo read_fpga  $(to_hex $(($(to_dec 0x03035028) + $OPTION_PORT * $(to_dec 0x100))))
        
        array[$index]=$value
        # array[$index]=$i

        if [[ $((($i + 1) % 16)) -eq 0 ]]; then
            if [[ -n $OPTION_FILE ]]; then
                echo ${array[*]} >> $OPTION_FILE
            else
                echo ${array[*]}
            fi
            unset array
        elif [[ $i == $(($OPTION_SIZE - 1)) ]]; then
            if [[ -n $OPTION_FILE ]]; then
                echo ${array[*]} >> $OPTION_FILE
            else
                echo ${array[*]}
            fi
            unset array
        fi
    done
}

parse_args "$@"
process_args