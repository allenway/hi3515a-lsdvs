mv rootfs_uclibc_64k.jffs2 rootfs_uclibc_64k.jffs2$(date +%y%m%d%H%M%S)
echo "start mk jffs2 for spi flash 64k block"
#strip bhdvs
cp rhadvs /install_sdk/Hi3520D_SDK_V1.0.1.0/package/rootfs_uclibc/tmp/dvs/
./mkfs.jffs2 -d /install_sdk/Hi3520D_SDK_V1.0.1.0/package/rootfs_uclibc -l -e 0x10000 --pad=0xb00000 -n -o ./rootfs_uclibc_64k.jffs2 
cp rootfs_uclibc_64k.jffs2 /mnt/hgfs/F/tftp
echo "mk end"
