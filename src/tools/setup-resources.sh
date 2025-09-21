src_dir=$1
target_dir=$2

for f in $src_dir/src/resources/*.ase; do
    name=$(basename $f .ase)
    aseprite -b $f --save-as $target_dir/resources/$name.png
    echo "creating $target_dir/resources/$name.png"
done
