
#!/bin/sh

set -xe

make
time (make) >&1 1>/dev/null
         
