src_dir=$1
target_dir=$2

for f in $src_dir/src/resources/*.ase; do
    name=$(basename $f .ase)
    aseprite -b $f --save-as $target_dir/resources/$name.png
    echo "creating $target_dir/resources/$name.png"
done

for f in $src_dir/src/resources/*.wav; do
    name=$(basename $f .wav)
    echo "copying over $target_dir/resources/$name.wav"
    cp $src_dir/src/resources/$name.wav $target_dir/resources/$name.wav
done
