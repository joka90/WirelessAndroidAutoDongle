#!/bin/bash

set -e

#remove some stuff
rm -rf "${TARGET_DIR}"/lib/modules/*/kernel/drivers/{w1,hid,accessibility,ata,char/tpm,iio,input,md,media,net/can,net/hamradio,net/ieee802154,net/ppp,rtc,scsi,staging/fbtft,staging/media}/

exit 0
