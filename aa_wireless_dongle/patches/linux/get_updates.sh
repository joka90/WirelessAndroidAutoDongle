#!/bin/bash -e

get_p() {
  local i="$1"
  local pname="00$i-6.1.${i}-$((i+1)).patch.xz"
  if [ ! -e "$pname" ] ; then
    wget "https://cdn.kernel.org/pub/linux/kernel/v6.x/incr/patch-6.1.${i}-$((i+1)).xz" -O "$pname"
  fi
}

start=73
end=90
for (( i=start; i<end; i++ )) ; do
   get_p "$i"
done
#rm -f patches-*-rt*.tar.xz
#wget "https://cdn.kernel.org/pub/linux/kernel/projects/rt/6.1/older/patches-6.1.${end}-rt30.tar.xz" -O "9990-rt-patch6.1.${end}-rt30.patch.xz"
