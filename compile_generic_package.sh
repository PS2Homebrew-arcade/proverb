
GAMELIST='gamelist.json'
OUTPUT='proverb.zip'


if ! command -v jq >/dev/null 2>&1
then
    echo "jq could not be found"
    exit 1
fi

if ! command -v zip >/dev/null 2>&1
then
    echo "zip could not be found"
    exit 1
fi

echo BUILDING PACKAGE FOR GAME BOOTLOADER REPLACEMENTS

for k in $(jq '.games | keys | .[]' $GAMELIST); do
    value=$(jq -r ".games[$k]" $GAMELIST);
    BOOT=$(echo "$value" | jq -r '.boot');
    ID=$(echo "$value" | jq -r '.id');
    TITLE=$(echo "$value" | jq -r '.title');
    echo - "$TITLE"
    make --silent clean all nkbu BOOT_PATH=\"$BOOT\" BINDIR="bin/$ID"
    printf "GAME:%s\nEXECUTABLE:%s" "$TITLE" "$BOOT" >"bin/$ID/game.txt"
done
rm -f $OUTPUT
zip -r $OUTPUT bin/* README.md LICENSE