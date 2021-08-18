sudo lfs --block_size=4096 --block_count=15625 --read_size=16 --prog_size=16 --lookahead_size=32 --cache_size=64 -o auto_unmount -o allow_other -o big_writes /dev/sda mp
