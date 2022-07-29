set -e

docker build -t pokefirered_builder - < Dockerfile > /dev/null
docker run -u 1000:1000 --rm -it -v $PWD:/app/pokefirered pokefirered_builder
echo "default 16777216 41cb23d8dccc8ebd7c649cd8fbb58eeace6e2fdc"
checksum=`sha1sum pokefirered.gba | cut -d " " -f1`
size=`stat --format "%s" pokefirered.gba`
echo "current $size $checksum"
