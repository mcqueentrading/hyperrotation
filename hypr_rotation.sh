#!/bin/bash

# Transform values for Hyprland
declare -A ROTATION_MAP=(
    ["normal"]=0
    ["right"]=3
    ["down"]=2
    ["left"]=1
    ["lef"]=4
)

rotate_monitors() {
    local rotation="$1"
    
    # Check if rotation is valid
    if [[ ! -v ROTATION_MAP[$rotation] ]]; then
        echo "Invalid mode: $rotation"
        echo "Valid options: normal, right, down, left, lef"
        exit 1
    fi
    
    local transform="${ROTATION_MAP[$rotation]}"
    
    # Get all connected monitors (parse text output)
    local monitors=$(hyprctl monitors | grep -oP '(?<=Monitor ).*(?= \(ID)')
    
    if [[ -z "$monitors" ]]; then
        echo "No monitors detected!"
        exit 1
    fi
    
    # Apply rotation to each monitor
    local count=0
    while IFS= read -r monitor; do
        hyprctl keyword monitor "$monitor,transform,$transform" >/dev/null
        echo "Applied transform $transform ($rotation) to $monitor"
        ((count++))
    done <<< "$monitors"
    
    echo -e "\n✓ Rotated $count monitor(s) to '$rotation'"
}

# Main
if [[ $# -ne 1 ]]; then
    echo "Usage: $(basename "$0") [normal|right|down|left|lef]"
    exit 1
fi

rotate_monitors "$1"
