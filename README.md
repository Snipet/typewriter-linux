# Building
`gcc -o keyclick keyclick.c -lsndfile -lpulse-simple -lpulse -lpthread`

# Running
- Ensure you can read `/dev/input`: `sudo usermod -aG input <linuxuser>`. Linux audio does not like programs ran by sudo.
- Read event2: `./keyclick /dev/input/event2`
