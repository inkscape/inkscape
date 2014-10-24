#!/usr/bin/env bash
#
#
# Create new icon theme based on GTK+ stock icons
#
# Copyright (C) 2014 ~suv
#


# config
#---------------------------------------------------------

if [ -z $LIBPREFIX ]; then
    LIBPREFIX="/opt/local-x11"
fi
if [ -z $stock_src ]; then
    stock_src="$(pwd)/stock-icons"
fi


# setup
#---------------------------------------------------------

map_legacy_icons="$LIBPREFIX/libexec/icon-name-mapping"
if [ ! -x "$map_legacy_icons" ]; then
    echo "Install icon-naming-utils" 
    exit 1
fi

if [ ! -d "$stock_src" ]; then
    echo "extra icons not found." 
    exit 1
fi

ICONDIR="$1"
icon_theme_dir="$(dirname "$ICONDIR")"
if [ ! -d "$icon_theme_dir" ] ; then
    mkdir -p  "$icon_theme_dir"
fi
icon_theme_name="$(basename "$ICONDIR")"
if [ -z "$icon_theme_name" ]; then
    echo "Not a valid icon theme name." 
    exit 1
fi
theme_color="$2"

contexts="actions animations apps categories devices emblems emotes mimetypes places status"
gtk_stock_sizes="16 20 24 32 48"

orig_dir="$(pwd)"
cd "$icon_theme_dir"
current_dir="$(pwd)"

index_file="${icon_theme_name}/index.theme"


# Remove a previously existing icon theme if necessary
#---------------------------------------------------------

if [ -d "$icon_theme_name" ]; then
    echo "Removing previous $icon_theme_name"
    rm -R "$icon_theme_name"
fi


# create new icon theme structure
#---------------------------------------------------------

mkdir -p "$icon_theme_name"
for size in $gtk_stock_sizes; do
    mkdir "${icon_theme_name}/${size}x${size}"
done


# copy stock icons
#---------------------------------------------------------

for size in $gtk_stock_sizes; do
    cp -RP "${stock_src}/$size" "${icon_theme_name}/${size}x${size}/stock"
done


# workarounds for broken icons (bug #1269698)
#---------------------------------------------------------

for size in $gtk_stock_sizes; do
    cd "${icon_theme_name}/${size}x${size}/stock"
    # directional icons
    di_stock="edit-undo edit-redo document-revert gtk-undelete
    format-indent-less format-indent-more
    go-first go-jump go-last go-next go-previous
    media-playback-start media-seek-backward media-seek-forward media-skip-backward media-skip-forward"
    for di in $di_stock; do
        if [ -f "${di}-ltr.png" ]; then
            if [ ! -e "${di}.png" ]; then
                ln -s "${di}-ltr.png" "${di}.png"
            fi
        fi 
    done
    # misc failed lookups
    for i in "preferences"; do
        if [ -f "gtk-${i}.png" ]; then
            if [ ! -e "${i}-system.png" ]; then
                ln -s "gtk-${i}.png" "${i}-system.png"
            fi
        fi
    done
    cd "$current_dir"
done


# create links (round 1): legacy mapping
#---------------------------------------------------------

for size in $gtk_stock_sizes; do
    cd "${icon_theme_name}/${size}x${size}"
    for ct in $contexts; do
        echo "size: $size  context: $ct"
        mv "stock" "$ct"
        $map_legacy_icons -c "$ct"
        mv $ct "stock"
    done
    cd "$current_dir"
done


# create links (round 2): directional icons (bug #1269698)
#---------------------------------------------------------

for size in $gtk_stock_sizes; do
    cd "${icon_theme_name}/${size}x${size}/stock"
    # legacy directional icons
    di_stock="gtk-undo gtk-redo gtk-revert-to-saved
    gtk-unindent gtk-indent 
    gtk-goto-first gtk-jump-to gtk-goto-last gtk-go-forward gtk-go-back"
    for di in $di_stock; do
        if [ -f "${di}-ltr.png" ]; then
            if [ ! -e "${di}.png" ]; then
                ln -s "${di}-ltr.png" "${di}.png"
            fi
        fi 
    done
    cd "$current_dir"
done


# create links (round 3): fallbacks for symbolic icon lookup
#---------------------------------------------------------

# for size in $gtk_stock_sizes; do
#     cd "${icon_theme_name}/${size}x${size}/stock"
#     for icon_file in *.png; do
#         [ -s $icon_file ] && ln -s "$icon_file" "$(basename $icon_file .png)"-symbolic.png
#     done
#     cd "$current_dir"
# done


# create new index.theme
#---------------------------------------------------------

dir_list=
for size in $gtk_stock_sizes; do
    dir_list="${dir_list}${size}x${size}/stock,"
done

cat > "$index_file" <<End-of-message
[Icon Theme]
Name=$icon_theme_name
Inherits=hicolor
Comment=Gtk Stock Icons for Inkscape.app
Example=folder

# Directory list
Directories=$dir_list

End-of-message

for size in $gtk_stock_sizes; do
    cat >> "$index_file" << End-of-message
[${size}x${size}/stock]
Size=${size}
Context=Stock
Type=fixed

End-of-message
done


#---------------------------------------------------------
# vim: expandtab shiftwidth=4 tabstop=8 softtabstop=4 fileencoding=utf-8 textwidth=99
