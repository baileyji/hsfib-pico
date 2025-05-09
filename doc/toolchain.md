openocd needs to be built from raspberry pi source as rp2350 not yet mainlined
use edited openocd file

brew install --build-from-source ./openocd.rb --head 

expect `Error: /opt/homebrew/Cellar/open-ocd/HEAD-cf9c0b4_1 is not a directory`, its seems fine
