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

for f in $src_dir/src/resources/*.mp3; do
    name=$(basename $f .mp3)
    echo "copying over $target_dir/resources/$name.mp3"
    cp $src_dir/src/resources/$name.mp3 $target_dir/resources/$name.mp3
done

for f in $src_dir/src/resources/*.glsl; do
    name=$(basename $f .glsl)
    echo "copying over $target_dir/resources/$name.glsl"
    cp $src_dir/src/resources/$name.glsl $target_dir/resources/$name.glsl
done

for f in $src_dir/src/resources/*.ttf; do
    name=$(basename $f .ttf)
    echo "copying over $target_dir/resources/$name.ttf"
    cp $src_dir/src/resources/$name.ttf $target_dir/resources/$name.ttf
done
