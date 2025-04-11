#!/bin/bash

flavour="$1"
script_path=$(realpath "$(dirname $0)")
vars_script_path="$script_path/../vars.sh"
if [[ ! -f "$vars_script_path" ]]; then
  echo "🚨 vars.sh script not found at $vars_script_path"
  exit 1
fi
source "$vars_script_path"

src_root="${SPEC17_DIR}"
dst_dir="$BASE/results/$flavour"
mkdir -p "$dst_dir"
mkdir -p "$dst_dir/stats"

save_stats() {
    find "$src_root" -type f -path "*/${flavour}_*/stats.txt" | while read -r filepath; do
        # Extract benchmark name like perlbench from path
        benchmark=$(echo "$filepath" | sed -E 's#.*/([0-9]+)\.([a-zA-Z0-9_]+)_s/.*#\2#')

        # Extract variant name from the mirage_* directory
        variant=$(basename "$(dirname "$filepath")" | cut -d'_' -f 2)

        # Construct new filename
        new_filename="${benchmark}_${variant}.stats"

        # Copy with new name
        cp "$filepath" "$dst_dir/stats/$new_filename"
        echo "✅ Copied: $filepath → $dst_dir/stats/$new_filename"
    done
}

save_plots_summary() {
    cp -r "${PLOTS_DIR}/"{data,plots} "$dst_dir"
    echo "✅ Copied: plots and data"
}

save_stats
save_plots_summary
